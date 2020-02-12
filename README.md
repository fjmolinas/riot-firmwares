### RIOT firmwares for [Pyaiot](https://github.com/pyaiot/pyaiot)

[![Build Status](https://travis-ci.org/pyaiot/riot-firmwares.svg?branch=master)](https://travis-ci.org/pyaiot/riot-firmwares)

This repository contains simple firmwares for reading or interacting with
sensor nodes using CoAP and MQTT-SN procotols.

The firmwares are designed to be used with the
[Pyaiot](https://github.com/pyaiot/pyaiot) transport and display backend.

The available firmwares are:
* [Air Monitor (CoAP)](apps/node_air_monitor): read environmental values from a
  [BME280](https://www.bosch-sensortec.com/bst/products/all_products/bme280)
  a gas sensor values from a [CCS811](https://ams.com/ccs811) and a particle
  sensor [SMPWM01C](https://www.amphenol.com/node/4647).
* [BMP180 sensor (CoAP)](apps/node_bmp180): read environmental values from a
  [BMP180](https://www.bosch-sensortec.com/bst/products/all_products/bmp180)
  sensor.
  The sensor has to be plugged on a SAMR21 Xplained Pro board
* [BMx280 CCs811 (CoAP)](apps/node_combo): read environmental values from a
  [BME280](https://www.bosch-sensortec.com/bst/products/all_products/bme280)
  a gas sensor values from a [CCS811](https://ams.com/ccs811)
* [Weather sensor (CoAP)](apps/node_bme180): read weather values (temperature,
  pressure, humidity) from a
  [BME280](https://www.bosch-sensortec.com/bst/products/all_products/bme280)
  sensor
* [LED control (CoAP)](apps/node_leds): interact with the on-board LED using CoAP.
  By default, the firmware is built for an Atmel SAMR21 Xplained Pro board
  (inverted LED)
* [IMU sensor (CoAP)](aps/node_imu): read the inertial measurement unit of an
  IoTLAB-M3 board
* [IoT-Lab A8-M3 node (CoAP)](apps/node_iotlab_a8_m3): interact with M3 LED of an
  A8 node in the IoTLAB testbed
* [Atmel IO1 Xplained sensor (CoAP)](apps/node_io1_xplained): read the temperature
  sensor of an IO1 Xplained extension board. The firmware is built for a SAMR21
  Xplained Pro board
* [Light sensor (CoAP)](apps/node_tsl2561): read the illuminance value (lx) from a
  [TSL2561](http://ams.com/eng/Products/Light-Sensors/Ambient-Light-Sensors/TSL2561/TSL2560-TSL2561-Datasheet)
  sensor.
* [BME280 sensor (MQTT-SN)](apps/node_mqtt_bme280): read environmental values
  from a
  [BME280](https://www.bosch-sensortec.com/bst/products/all_products/bme280)
  sensor. Values are raised using the MQTT-SN protocol.
* [CCS811 sensor (CoAP)](apps/node_ccs811): read gas sensor values from a
  [CCS811](https://ams.com/ccs811) sensor.

All firmwares source codes are based on [RIOT](https://github.com/RIOT-OS/RIOT).

#### Initializing the repository:

RIOT is included as a submodule of this repository. We provide a `make` helper
target to initialize it.
From the root of this repository, issue the following command:

    $ make init_submodules


#### Building the firmwares:

From the root directory of this repository, simply issue the following command:

    $ make


#### Flashing the firmwares

For each firmwares use the RIOT way of flashing them. For example, in
`apps/node_bmp180`, use:

    $ make -C apps/node_bmp180 flash

to flash the firmware on a SAMR21 XPlained Pro board.

#### Global cleanup of the generated firmwares

From the root directory of this repository, issue the following command:

    $ make clean

#### Running suit capable applications

Applications supporting suit:

    - node_air_monitor
    - node_bmp180
    - node_bmx280
    - node_css811
    - node_combo
    - node_empty
    - node_io1_xplained
    - node_leds
    - node_sm_pwm_01c
    - node_tsl2561

One can as follow the setup in [examples/suit_update](https://github.com/RIOT-OS/RIOT/tree/master//examples/suit_update/README.md) and replace all `make -C examples/suit_update`
by the application of your choice.

For better interaction with `pyaiot` we use [otaserver](https://github.com/aabadie/ota-server),
the following setup will require ota-server to work.

##### Application boot - strapping

You will require a border router, to setup a RIOT border router you can follow
instructions in:
[examples/gnrc_border_router](https://github.com/RIOT-OS/RIOT/tree/master/examples/gnrc_border_router)

You can as well run the following command on a BOARD with a 802.15.4 radio:

    $ BOARD=iotlab-m3 make -C RIOT/examples/gnrc_border_router/ flash

Then start the border router running:

    $ sudo RIOT/dist/tools/ethos/start_network.sh /dev/riot/tty-iotlab-m3 riot0 ${LOCAL_IP6_PREFIX}::1/64

Bootstrap your device of choice with one of the enabled applications:

    $ make -C apps/node_air_monitor flash term

If the Border Router is already set up when opening the terminal you should get

    ...

    Iface  6  HWaddr: 0D:96  Channel: 26  Page: 0  NID: 0x23
            Long HWaddr: 79:7E:32:55:13:13:8D:96
             TX-Power: 0dBm  State: IDLE  max. Retrans.: 3  CSMA Retries: 4
            AUTOACK  ACK_REQ  CSMA  L2-PDU:102 MTU:1280  HL:64  RTR
            RTR_ADV  6LO  IPHC
            Source address length: 8
            Link type: wireless
            inet6 addr: fe80::7b7e:3255:1313:8d96  scope: link  VAL
            inet6 addr:  2001:6d6f:6c69:6e61:7b7e:3255:1313:8d96  scope: global  VAL
            inet6 group: ff02::2
            inet6 group: ff02::1
            inet6 group: ff02::1:ff17:dd59
            inet6 group: ff02::1:ff00:2

    suit_coap: started.

Here the global IPv6 is `2001:6d6f:6c69:6e61:7b7e:3255:1313:8d96`.

**The address will be different according to your device and the chosen prefix**.
In this case the RIOT node can be reached from the host using its global address:

    $ ping6 2001:db8::7b7e:3255:1313:8d96

##### Launch the ota-server

For documentation ans installation details refer to https://github.com/aabadie/ota-server.

To run locally:

    $ python3 otaserver/main.py --http-port=8888 --coap-port=5683 --coap-host=[fd00:dead:beef::1] --debug

We use `[fd00:dead:beef::1]` since this address is configure for the `lo` interface
when using `start_network.sh`. You can use any value for `--http-port` as long
as you don't have conflicts with other applications e.g.: `pyaiot`

##### Publish new FW

`ota-server` does not use the same targets for `suit/publish` or `suit/notify`.
To override the default targets we add `suit.v4.http.mk` as a `RIOT_MAKEFILES_GLOBAL_POST`,
eg:

    $ export RIOT_MAKEFILES_GLOBAL_POST=/home/riot-firmwares/Makefiles/suit.v4.http.mk

You can either export this or prefix every `suit/publish` or `suit/notify` call.

You can now publish new firmware:

    $ SUIT_OTA_SERVER_URL="http://127.0.0.1:8888" RIOT_MAKEFILES_GLOBAL_POST=/home/riot-firmwares/Makefiles/suit.v4.http.mk make -C apps/node_air_monitor/ suit/publish

##### Notify the device

To notify a device you need to specify the address of the client as well as the
server url. Optionally you can also specify the manifest version (SUIT_NOTIFY_VERSION).

    $ SUIT_CLIENT=[2001:6d6f:6c69:6e61:7b7e:3255:1313:8d96] SUIT_OTA_SERVER_URL="http://127.0.0.1:8888" RIOT_MAKEFILES_GLOBAL_POST=/home/riot-firmwares/Makefiles/suit.v4.http.mk

##### Making it easier

You can save `SUIT_OTA_SERVER_URL`, `SUIT_CLIENT`, `RIOT_MAKEFILES_GLOBAL_POST` in
`demo_config.sh` and source this file so bootstrap, publishing and notifying new
firmware is reduced to:

    $ make -C apps/node_air_monitor flash
    $ make -C apps/node_air_monitor suit/publish
    $ make -C apps/node_air_monitor suit/notify