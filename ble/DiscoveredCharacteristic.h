/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __DISCOVERED_CHARACTERISTIC_H__
#define __DISCOVERED_CHARACTERISTIC_H__

#include "UUID.h"
#include "Gap.h"
#include "GattAttribute.h"
#include "GattClient.h"
#include "CharacteristicDescriptorDiscovery.h"
#include "ble/DiscoveredCharacteristicDescriptor.h"


/**
 * Structure for holding information about the service and the characteristics
 * found during the discovery process.
 */
class DiscoveredCharacteristic {
public:
    struct Properties_t {
        uint8_t _broadcast       :1; /**< Broadcasting the value permitted. */
        uint8_t _read            :1; /**< Reading the value permitted. */
        uint8_t _writeWoResp     :1; /**< Writing the value with Write Command permitted. */
        uint8_t _write           :1; /**< Writing the value with Write Request permitted. */
        uint8_t _notify          :1; /**< Notications of the value permitted. */
        uint8_t _indicate        :1; /**< Indications of the value permitted. */
        uint8_t _authSignedWrite :1; /**< Writing the value with Signed Write Command permitted. */

    public:
        bool broadcast(void)       const {return _broadcast;      }
        bool read(void)            const {return _read;           }
        bool writeWoResp(void)     const {return _writeWoResp;    }
        bool write(void)           const {return _write;          }
        bool notify(void)          const {return _notify;         }
        bool indicate(void)        const {return _indicate;       }
        bool authSignedWrite(void) const {return _authSignedWrite;}

        friend bool operator==(Properties_t rhs, Properties_t lhs) {
            return rhs._broadcast == lhs._broadcast &&
                   rhs._read == lhs._read &&
                   rhs._writeWoResp == lhs._writeWoResp &&
                   rhs._write == lhs._write &&
                   rhs._notify == lhs._notify &&
                   rhs._indicate == lhs._indicate &&
                   rhs._authSignedWrite == lhs._authSignedWrite;
        }

        friend bool operator!=(Properties_t rhs, Properties_t lhs) { 
            return !(rhs == lhs);
        }

    private:
        operator uint8_t()  const; /* Disallow implicit conversion into an integer. */
        operator unsigned() const; /* Disallow implicit conversion into an integer. */
    };

    struct SubscriptionFlags_t { 
        SubscriptionFlags_t(bool notify, bool indicate) :
            _notify(notify), _indicate(indicate) { 
        }

        bool notify() const { return _notify; }               /**< subscription to notification */
        bool indicate() const { return _indicate; }           /**< subscription to indication */

        uint8_t serialize() { 
            return (indicate() << 1) | (notify() << 0);
        }

        friend bool operator==(SubscriptionFlags_t rhs, SubscriptionFlags_t lhs) {
            return rhs._notify == lhs._notify &&
                   rhs._indicate == lhs._indicate;
        }

        friend bool operator!=(SubscriptionFlags_t rhs, SubscriptionFlags_t lhs) { 
            return !(rhs == lhs);
        }

    private:
        uint8_t _notify:1;
        uint8_t _indicate:1;
    };

    struct SubscriptionCallbackParams_t {
        const DiscoveredCharacteristic& characteristic;
        ble_error_t status;
        SubscriptionFlags_t operation;
    };

    typedef FunctionPointerWithContext<const SubscriptionCallbackParams_t*> SubscriptionCallback_t;

    /**
     * Initiate (or continue) a read for the value attribute, optionally at a
     * given offset. If the characteristic or descriptor to be read is longer
     * than ATT_MTU - 1, this function must be called multiple times with
     * appropriate offset to read the complete value.
     *
     * @return BLE_ERROR_NONE if a read has been initiated, or
     *         BLE_ERROR_INVALID_STATE if some internal state about the connection is invalid, or
     *         BLE_STACK_BUSY if some client procedure is already in progress, or
     *         BLE_ERROR_OPERATION_NOT_PERMITTED due to the characteristic's properties.
     */
    ble_error_t read(uint16_t offset = 0) const;

    /**
     * Perform a write without response procedure.
     *
     * @param  length
     *           The amount of data being written.
     * @param  value
     *           The bytes being written.
     *
     * @note   It is important to note that a write without response will generate
     *         an onDataSent() callback when the packet has been transmitted. There
     *         will be a BLE-stack specific limit to the number of pending
     *         writeWoResponse operations; the user may want to use the onDataSent()
     *         callback for flow-control.
     *
     * @retval BLE_ERROR_NONE Successfully started the Write procedure, or
     *         BLE_ERROR_INVALID_STATE if some internal state about the connection is invalid, or
     *         BLE_STACK_BUSY if some client procedure is already in progress, or
     *         BLE_ERROR_NO_MEM if there are no available buffers left to process the request, or
     *         BLE_ERROR_OPERATION_NOT_PERMITTED due to the characteristic's properties.
     */
    ble_error_t writeWoResponse(uint16_t length, const uint8_t *value) const;

    /**
     * Initiate a GATT Characteristic Descriptor Discovery procedure for descriptors within this characteristic.
     *
     * @param onCharacteristicDiscovered This callback will be called every time a descriptor is discovered
     * @param onTermination This callback will be called when the discovery process is over.
     *
     * @return  BLE_ERROR_NONE if descriptor discovery is launched successfully; else an appropriate error.
     */
    ble_error_t discoverDescriptors(const CharacteristicDescriptorDiscovery::DiscoveryCallback_t& onCharacteristicDiscovered, 
                                    const CharacteristicDescriptorDiscovery::TerminationCallback_t& onTermination) const;

    /**
     * @brief Set the subscription flags for this characteristic
     * @details This is an asynchronous operation which can take some time.
     * The operations involved are:
     *    - Discover the CCCD (Client Characteristic Configuration Descriptor) 
     *      of this characteristic
     *    - Write the CCCD with the new values provided
     * 
     * @param flags Two type of subscriptions are available: 
     *   - notification: A server can notify a client that a characteristic has 
     *   changed and provide the new value. If the notification flag is enabled
     *   and the server send a notification, the notification will be available 
     *   through GattClient::onHVX registered callback. Notifications are reliable.
     *   - indication: same as above except that an indication is **not** reliable.
     *   
     * @param callback The callback called when the operation end.
     * 
     * @return BLE_ERROR_NONE if the operation has been launched successfully; 
     * else an appropriate error.
     */
    ble_error_t setSubscriptionFlags(SubscriptionFlags_t flags, const SubscriptionCallback_t& callback);

    /**
     * Perform a write procedure.
     *
     * @param  length
     *           The amount of data being written.
     * @param  value
     *           The bytes being written.
     *
     * @note   It is important to note that a write will generate
     *         an onDataWritten() callback when the peer acknowledges the request.
     *
     * @retval BLE_ERROR_NONE Successfully started the Write procedure, or
     *         BLE_ERROR_INVALID_STATE if some internal state about the connection is invalid, or
     *         BLE_STACK_BUSY if some client procedure is already in progress, or
     *         BLE_ERROR_NO_MEM if there are no available buffers left to process the request, or
     *         BLE_ERROR_OPERATION_NOT_PERMITTED due to the characteristic's properties.
     */
    ble_error_t write(uint16_t length, const uint8_t *value) const;

    void setupLongUUID(UUID::LongUUIDBytes_t longUUID) {
        uuid.setupLong(longUUID);
    }

public:
    const UUID& getUUID(void) const {
        return uuid;
    }

    const Properties_t& getProperties(void) const {
        return props;
    }

    GattAttribute::Handle_t getDeclHandle(void) const {
        return declHandle;
    }

    GattAttribute::Handle_t getValueHandle(void) const {
        return valueHandle;
    }

    GattAttribute::Handle_t getLastHandle(void) const {
        return lastHandle;
    }

    void setLastHandle(GattAttribute::Handle_t last) { 
        lastHandle = last;
    }

    GattClient* getGattClient() { 
        return gattc;
    }

    const GattClient* getGattClient() const { 
        return gattc;
    }

    Gap::Handle_t getConnectionHandle() const {
        return connHandle;
    }

    friend bool operator==(const DiscoveredCharacteristic& rhs, const DiscoveredCharacteristic& lhs) {
        return rhs.gattc == lhs.gattc && 
               rhs.uuid == lhs.uuid &&
               rhs.props == lhs.props &&
               rhs.declHandle == lhs.declHandle &&
               rhs.valueHandle == lhs.valueHandle &&
               rhs.lastHandle == lhs.lastHandle &&
               rhs.connHandle == lhs.connHandle;
    }

    friend bool operator !=(const DiscoveredCharacteristic& rhs, const DiscoveredCharacteristic& lhs) {
        return !(rhs == lhs);
    }

public:
    DiscoveredCharacteristic() : gattc(NULL),
                                 uuid(UUID::ShortUUIDBytes_t(0)),
                                 props(),
                                 declHandle(GattAttribute::INVALID_HANDLE),
                                 valueHandle(GattAttribute::INVALID_HANDLE),
                                 lastHandle(GattAttribute::INVALID_HANDLE),
                                 connHandle() {
        /* empty */
    }

protected:
    GattClient              *gattc;

protected:
    UUID                     uuid;
    Properties_t             props;
    GattAttribute::Handle_t  declHandle;
    GattAttribute::Handle_t  valueHandle;
    GattAttribute::Handle_t  lastHandle;

    Gap::Handle_t            connHandle;
};

#endif /*__DISCOVERED_CHARACTERISTIC_H__*/
