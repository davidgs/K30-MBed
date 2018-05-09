#ifndef __K30_SERVICE_H__
#define __K30_SERVICE_H__
 
class K30Service {
public:
    const static uint16_t K30_SERVICE_UUID              = 0xA000;
    const static uint16_t K30_VALUE_CHARACTERISTIC_UUID = 0xA001;
 
    K30Service(BLEDevice &_ble, float k30Initial) :
        ble(_ble), k30Value(K30_VALUE_CHARACTERISTIC_UUID, &k30Initial, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
    {
        GattCharacteristic *charTable[] = {&k30Value};
        GattService         k30Service(K30Service::K30_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.addService(k30Service);
    }
 
    void updateK30Value(float newValue) {
        ble.updateCharacteristicValue(k30Value.getValueHandle(), (uint8_t *)&newValue, sizeof(float));
    }
 
private:
    BLEDevice                        &ble;
    ReadOnlyGattCharacteristic<float>  k30Value;
};
 
#endif /* #ifndef __K30_SERVICE_H__ */