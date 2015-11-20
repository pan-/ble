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

#include "ble/DiscoveredCharacteristic.h"
#include "ble/GattClient.h"

ble_error_t
DiscoveredCharacteristic::read(uint16_t offset) const
{
    if (!props.read()) {
        return BLE_ERROR_OPERATION_NOT_PERMITTED;
    }

    if (!gattc) {
        return BLE_ERROR_INVALID_STATE;
    }

    return gattc->read(connHandle, valueHandle, offset);
}

ble_error_t
DiscoveredCharacteristic::write(uint16_t length, const uint8_t *value) const
{
    if (!props.write()) {
        return BLE_ERROR_OPERATION_NOT_PERMITTED;
    }

    if (!gattc) {
        return BLE_ERROR_INVALID_STATE;
    }

    return gattc->write(GattClient::GATT_OP_WRITE_REQ, connHandle, valueHandle, length, value);
}

ble_error_t
DiscoveredCharacteristic::writeWoResponse(uint16_t length, const uint8_t *value) const
{
    if (!props.writeWoResp()) {
        return BLE_ERROR_OPERATION_NOT_PERMITTED;
    }

    if (!gattc) {
        return BLE_ERROR_INVALID_STATE;
    }

    return gattc->write(GattClient::GATT_OP_WRITE_CMD, connHandle, valueHandle, length, value);
}

ble_error_t DiscoveredCharacteristic::discoverDescriptors(
    const CharacteristicDescriptorDiscovery::DiscoveryCallback_t& onCharacteristicDiscovered, 
    const CharacteristicDescriptorDiscovery::TerminationCallback_t& onTermination) const {

    if(!gattc) {
        return BLE_ERROR_INVALID_STATE;
    }

    ble_error_t err = gattc->discoverCharacteristicDescriptors(
        *this, onCharacteristicDiscovered, onTermination
    );

    return err;
}


class WriteOperation



class CCCDDiscoveryOperation { 

    static ble_error_t launch(const Characteristic& characteristic, 
                              SubscriptionFlags_t flags, 
                              const SubscriptionCallback_t& callback) { 

        CCCDDiscoveryOperation* op = new CCCDDiscoveryOperation(flags, callback);
        ble_error_t err = characteristic.discoverDescriptors(
            makeFunctionPointer(this, &CCCDDiscoveryOperation::whenDiscoveredCharacteristic),
            makeFunctionPointer(this, &CCCDDiscoveryOperation::whenDiscoveryEnd)
        );

        if(err) { 
            delete op;
        }

        return err;
    }

private:

    CCCDDiscoveryOperation(SubscriptionFlags_t flags, 
                   const SubscriptionCallback_t& callback) 
        : _flags(flags), _callback(callback) { 
    }

    void whenDiscoveredCharacteristic(const DiscoveryCallbackParams_t* params) { 
        const DiscoveredCharacteristic& characteristic = params->characteristic;

        if(params->descriptor.uuid() == DiscoveredCharacteristicDescriptor::CLIENT_CHARACTERISTIC_CONFIGURATION_UUID) {
            // descriptors has been found, launching the write operation




            // terminate the discovery
            //TODO: remove this hack !!!
            ((GattClient*) characteristic.getGattClient())->terminateCharacteristicDescriptorsDiscovery(characteristic);
        }
    }

    void whenDiscoveryEnd(const TerminationCallbackParams_t* params) {
        if(params->status) { 
            SubscriptionCallbackParams_t result = {
                params->characteristic,
                params->status,
                _flags
            };
            _callback(result);
        }

        delete *this;
    }

    SubscriptionFlags_t _flags;
    SubscriptionCallback_t _callback;
};


ble_error_t DiscoveredCharacteristic::setSubscriptionFlags(
    SubscriptionFlags_t flags, const SubscriptionCallback_t& callback) { 

    // check if this characteristic contains a CCCD
    if (!props.notify() && !props.indicate()) {
        return BLE_ERROR_INVALID_PARAM;        
    }

    // check if the subscription flags can be applied to this characteristic
    if ((flags.notify() && !props.notify()) || 
        (flags.indicate() && !props.indicate())) { 
        return BLE_ERROR_INVALID_PARAM;
    }

    // start the discovery of descriptors of this characteristic 
    return CCCDDiscoveryOperation::launch(*this, flags, callback);
}
