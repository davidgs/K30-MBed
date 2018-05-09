/* mbed Microcontroller Library
 * Copyright (c) 2018 David G. Simmons
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

#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "k30.h"
#include "nrf_nvic.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
//I2C i2c(p24 , p25);
// Standard I2C pins on the nRF52. But you can use any pins you want really. 
I2C i2c(p26, p27);
 /** If you want to debug, or see output, uncomment this **/
//Serial pc(USBTX, USBRX); // tx, rx

/* 7-bit address of the K30 CO2 Sensor */ 
const int addr = 0xD0;

/* keep track of the number of sensor failures */
static int failures = 0;

/** Device name, and the Serice UUID **/
const static char     DEVICE_NAME[] = "CO2Sensor";
static const uint16_t uuid16_list[] = {K30Service::K30_SERVICE_UUID};

/** random initial level and a Service pointer **/
static float co2Level = 50.0;
static K30Service* k30ServicePtr;

/** Event Queue **/
static EventQueue eventQueue(/* event count */ 16 * EVENTS_EVENT_SIZE);

/** light pattern in a circle **/
void lightsFwd(){
    led1 = !led1;
    wait(.15);
    led2 = !led2;
    wait(.15);
    led4 = !led4;
    wait(.15);
    led3 = !led3;
    wait(.15);
}
/** reverser light pattern **/
void lightsRev(){
    led1 = !led1;
    wait(.15);
    led3 = !led3;
    wait(.15);
    led4 = !led4;
    wait(.15);
    led2 = !led2;
    wait(.15);
}

/** here we read the sensor **/
void readSensor(){
 
    // register values
    char cmd[4] =  {0x22, 0x00, 0x08, 0x2A};
    int ack = i2c.write(addr, cmd, 4);
    wait(0.5);
    char readBuff[4];
    i2c.read(addr, readBuff, 4, false);
    int high = readBuff[1];                        //high byte for value is 4th byte in packet in the packet
    int low = readBuff[2];                         //low byte for value is 5th byte in the packet
    float CO2 = high*256 + low;                //Combine high byte and low byte with this formula to get value        
    char sum = readBuff[0] + readBuff[1] + readBuff[2]; //Byte addition utilizes overflow   
    if (sum == readBuff[3] & ack == 0){
        //pc.printf("CO2 value = %f\n", CO2);
        k30ServicePtr->updateK30Value(CO2);
        if(failures > 0){
            failures--;
        }           
        } else {
            //pc.printf("** Sensor Failure **\n");
            failures++;
            CO2 = -1;
            k30ServicePtr->updateK30Value(CO2);
            if(failures > 5){ // Keep track of the number of failures. If more than 5, reboot the board. 
                i2c.stop();
                for(int x = 0; x < 10; x++){
                    lightsRev();
                }
                NVIC_SystemReset();
            }
            
        }
}
void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    //pc.printf("Disconnected!\n");
    BLE::Instance().gap().startAdvertising();
}



void updateSensorValue() {
    lightsFwd();
    readSensor();
    wait(1.5);
    lightsFwd();
    wait(1.5

   // k30ServicePtr->updateK30Value(co2Level);
}
void connectionCallback(const Gap::ConnectionCallbackParams_t *params)
{
    // pc.printf("Connected!\n");
    BLE::Instance().gap().stopAdvertising();
    eventQueue.call(updateSensorValue);
}
void blinkCallback(void)
{
    BLE &ble = BLE::Instance();
    if (ble.gap().getState().connected) {
        eventQueue.call(updateSensorValue);
    } else {
        lightsFwd();
    }
}

/**
 * This function is called when the ble initialization process has failled
 */
void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

void printMacAddress()
{
    /* Print out device MAC address to the console*/
    Gap::AddressType_t addr_type;
    Gap::Address_t address;
    BLE::Instance().gap().getAddress(&addr_type, address);
    //pc.printf("DEVICE MAC ADDRESS: ");
    for (int i = 5; i >= 1; i--){
       // printf("%02x:", address[i]);
    }
    //pc.printf("%02x\r\n", address[0]);
}

/**
 * Callback triggered when the ble initialization process has finished
 */
void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        /* In case of error, forward the error handling to onBleInitError */
        onBleInitError(ble, error);
        return;
    }

    /* Ensure that it is the default instance of BLE */
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);
    ble.gap().onConnection(connectionCallback);

    /* Setup primary service */
    k30ServicePtr = new K30Service(ble, co2Level);

    /* Setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *) uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *) DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms */
    ble.gap().startAdvertising();

   //printMacAddress();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main()
{
    eventQueue.call_every(1000, blinkCallback);
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);
    eventQueue.dispatch_forever();
    return 0;
}
