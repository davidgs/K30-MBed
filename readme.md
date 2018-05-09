This example creates and updates a CO2 Monitoring service containing a single
GATT characteristic.

The service updates a CO2 reading in parts per million (PPM).

The sample application runs on a BLE device. This is intended to run on a Nordic nRF52DK developer board using mBed OS.

The CO2 level should not be read more than every 2 seconds as the sensor itself doesn't update the value more frequently. Also, the nRF52DK board for which this is written can power the sensors -- 5v at 500ma -- it sometimes struggles to keep the power rail steady. When this happens, the sensor begins to fail. If there are more than 5 sensor failures in a row, the device automatically reboots which usually fixes the problem.


# Running the application

## Requirements

The sample application can be seen on any BLE scanner on a smartphone. If you don't have a scanner on your phone, please install :

- [nRF Master Control Panel](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) for Android.

- [LightBlue](https://itunes.apple.com/gb/app/lightblue-bluetooth-low-energy/id557428110?mt=8) for iPhone.

Hardware requirements are in the [main readme](https://github.com/ARMmbed/mbed-os-example-ble/blob/master/README.md).

## Building instructions

Building instructions for all samples are in the [main readme](https://github.com/ARMmbed/mbed-os-example-ble/blob/master/README.md).

## Checking for success

**Note:** Screens captures depicted below show what is expected from this example if the scanner used is *nRF Master Control Panel* version 4.0.5. If you encounter any difficulties consider trying another scanner or another version of nRF Master Control Panel. Alternative scanners may require reference to their manuals.

1. Build the application and install it on your board as explained in the building instructions.
1. Open the BLE scanner on your phone.
1. Start a scan.

    ![](img/start_scan.png)

    **figure 1** How to start scan using nRF Master Control Panel 4.0.5

1. Find your device; it should be named `BATTERY`.

    ![](img/scan_result.png)

    **figure 2** Scan results using nRF Master Control Panel 4.0.5

1. Establish a connection with your device.

    ![](img/connection.png)

    **figure 3**  How to establish a connection using Master Control Panel 4.0.5

1. Discover the services and the characteristics on the device. The *undefined service* has the UUID 0xA000 and includes the *undefined* characteristic which has the UUID 0xA0001.


1. Register for the notifications sent by the *undefined* characteristic.


1. You should see the CO2 reading value change.

If you can see the characteristic, and if its value is incrementing correctly, the application is working properly.

