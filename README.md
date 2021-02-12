### RIOT firmwares for [Pyaiot](https://github.com/pyaiot/pyaiot)

![GitHub Workflow Status](https://img.shields.io/github/workflow/status/fjmolinas/riot-firmwares/compile-test)

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

Some of the firmware applications can perform OTA (over the air) updates. More
on this below.

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

### Over-the-air (OTA) support

If you haven't already take a look at [pyaiot](https://github.com/pyaiot/pyaiot)
to get an overview of the project and its building blocks.

Multiple applications are able to perform over the air updates using riot `suit`
module.

For a detailed suit walkthrough refer to [examples/suit_update](https://github.com/RIOT-OS/RIOT/tree/master//examples/suit_update/README.md). These applications will follow the
same approach so you can think of `examples/suit_update` as any of these `apps`:

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

For better interaction with `pyaiot` we use [otaserver](https://github.com/aabadie/ota-server),
instead of `aiocoap-fileserver` as is done in `examples/suit_update`, but this
only changes the firmware updates backed.

#### Over-the-air (OTA) setup overview

The following guide will detail how to setup over the air updates between a Node
running one of the above listed applications and the `ota-server` backend. The
server will store new firmware updates.

##### Gateway

Instead of a Linux Gateway we will use a RIOT node as a border router (more
details in [examples/gnrc_border_router](https://github.com/RIOT-OS/RIOT/tree/master/examples/gnrc_border_router)).

This will be the bridge between the node and the rest of the `pyaiot` applications
as well as the `ota-server`.

Pick any RIOT supported BOARD with 802.15.4 support and flash `gnrc_border_router`
aplications:

    $ BOARD=iotlab-m3 make -C RIOT/examples/gnrc_border_router/ flash

Then start the border router by running:

    $ sudo RIOT/dist/tools/ethos/start_network.sh /dev/ttyUSB0 riot0 2001:db8::/64

Make sure the port (/dev/ttyUSB0) matches the serial port of your border router
node.

##### Node

We need to first enable a node to perform OTA updates by flashing an OTA capable
firmware. We will also need to know the ipv6 address of the node we want to update.

So first flash one of the above listed applications

    $ make -C apps/node_air_monitor flash term

If the Border Router is already set up your device should get a global ipv6 address.
On the opened terminal use `ifconfig` to find this out.

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

I this case the address is `SUIT_CLIENT=[2001:6d6f:6c69:6e61:7b7e:3255:1313:8d96]`

Your device should now be reachable from your host.

    $ ping6 2001:db8::7b7e:3255:1313:8d96

##### ota-server

The new firmware we publish needs to be stored in a backed server. For this
scenario we also want to be able to notify a device of a new update and trigger
it.

We will use [otaserver](https://github.com/aabadie/ota-server). Please follow the
setup instructions in [otaserver](https://github.com/aabadie/ota-server) before
proceeding.

To run locally:

    $ python3 otaserver/main.py --http-port=8888 --coap-port=5683 --coap-host=[fd00:dead:beef::1] --debug

We use `[fd00:dead:beef::1]` since this address that will be automatically configure
for the  `lo` interface when using `start_network.sh`. If the `ota-server` is
running on a publicly reachable address you ca use that one instead of your host
`lo` address, or configure routing yourself.

You can use any value for `--http-port` as long as you don't have conflicts with
other applications e.g.: `pyaiot`

There is also a web interface now running at `http://localhost:8888` where you
can visualize the published firmware as well as trigger updates for specific
version.

##### Updating the device

Everything is now in place to publish new updates and trigger the device to fetch
the new firmware.

`ota-server` does not use the same `make` targets for `suit/publish` or `suit/notify`
than the default ones in RIOT.

To override the default targets we add `suit.v4.http.mk` as a
[RIOT_MAKEFILES_GLOBAL_POST](http://riot-os.org/api/advanced-build-system-tricks.html)

This is done by default when any `suit/%` target is called, to not use this then
set `OTASERVER=0`

Lets publish firmware! For this you will also need to specify the url where the
ota-server is running, assuming it is running locally, it would be:
`SUIT_OTA_SERVER_URL="http://127.0.0.1:8888`, so:

    $ SUIT_OTA_SERVER_URL="http://127.0.0.1:8888" make -C apps/node_air_monitor/ suit/publish

Now that the new firmware is published we can tell the device about this new
update. For this we will need to specify the device ipv6 address (which we
found out in previous steps):

To notify a device you need to specify the address of the client as well as the
server url. Optionally you can also specify the manifest version (SUIT_NOTIFY_VERSION).

    $ SUIT_CLIENT=[2001:6d6f:6c69:6e61:7b7e:3255:1313:8d96] SUIT_OTA_SERVER_URL="http://127.0.0.1:8888" make -C apps/node_air_monitor/ suit/notify

This will trigger an update of the new firmware on the device. The application
will keep running while the device is updating and when the update completes the
device will reboot and run the new firmware.

##### Making it easier

To avoid setting all the command line variables you can save them to `demo_config.sh`
and source this file `flash`, and `suit/publish`, `suit/notify`. This the commands
will as simple as:

    $ make -C apps/node_air_monitor flash
    $ make -C apps/node_air_monitor suit/publish
    $ make -C apps/node_air_monitor suit/notify

##### Troubleshooting

OTA updates implement [suit](https://datatracker.ietf.org/wg/suit/about/) IETF
working standard. In short, this insures end-to-end security for the update
process.

Every update will be signed with keys generated under `apps/keys` if these keys
change the device might reject an update and will need to be flashed again with
the correct keys.

The OTA mechanism will reject older updates, so a Node will reject a firmware
older than the one it is running.

A `suit` device will check some parameters validity before updating, among these
the application and device it was built for. These means a `samr21-xpro` will
reject a firmware built for `nrf52dk`. This also means that different applications
will also be rejected. In RIOT we specify the application with the `APPLICATION`
variable in the applications `Makefile` (e.g.:
[apps/node_air_monitor/Makefile](apps/node_air_monitor/Makefile))

To be able to perform OTA updates of different applications on the same device
this variable must be set you can do this by prefixing your commands with
the `APPLICATION` name or exporting it, so:

    $ APPLICATION=foo make suit/publish

    $ export APPLICATION=foo
    $ make suit/publish

If using the configuration file you can set your desired `APPLICATION` name there
as well.

#### riot-firmware + pyaiot + ota-server

The following steps will specify commands to setup the whole `pyaiot` ecosystem
as well as the `ota-server` and with this be able to perform over the air updates
on a Node, while at the same time visualizing the process and the updates on
the `pyaiot` dashboard.

This assumes that setup for [Pyaiot](https://github.com/pyaiot/pyaiot) and
[ota-server](https://github.com/aabadie/ota-server) has already been completed.
As good practice install the in separate [virtualenv](https://virtualenv.pypa.io/en/latest/),
to avoid conflictts (checkout out [virtualenvwrapper](https://virtualenvwrapper.readthedocs.io/en/latest/install.html) as well)

1. Launch the broker:

    ```
    $ aiot-broker --debug
    ```

2. Launch the coap gateway

    ```
    $  aiot-coap-gateway --coap-port=5688 --debug
    ```

3. Launch the dashboard, 'static-path' must be replaced with your path

    ```
    $ aiot-dashboard --static-path=/home/pyaiot/dashboard/static --map-api-key=<APP-KEY> debug
    ```

For the `APP-KEY` ask pyaiot maintainers.

The dashboard web page will be in: http://localhost:8080

4. Launch the ota-server (important to not use default 8080, used by dashboard)

    ```
    $ python3 otaserver/main.py --http-port=8888 --coap-port=5683 --coap-host=[fd00:dead:beef::1] --debug
    ```

The ota-server web page will be in: http://localhost:8888

5. Flash the device with an coap application and recover EUI64, first 64 bytes of
   its link local IpV6 address:

    ```
    $ make -C apps/node_air_monitor flash term
    ...
    > ifconfig
    ...
        inet6 addr: fe80::7b7e:3255:1313:8d96  scope: link  VAL
    ...
    ```

    Here it would be `7b7e:3255:1313:8d96`

6. Modify `demo_config.sh` to set `SUIT_CLIENT` and `LOCAL_IPV6_PREFIX` for your
   network, e.g.:

   ```
   export LOCAL_IP6_PREFIX="2001:6d6f:6c69:6e61"
   export SUIT_CLIENT="[${LOCAL_IP6_PREFIX}:7b7e:3255:1313:8d96]"
   ```

7. Source your configuration file (you can of course skip this and pass required
   parameters via the command line as detailed in previous steps)

    ```
    $ source demo_config.sh
    ```

8. Flash and start the border router

    ```
    $ make BOARD=iotlab-m3 -C examples/gnrc_border_router/ flash
    $ sudo RIOT/dist/tools/ethos/start_network.sh /dev/riot/tty-iotlab-m3 riot0 ${LOCAL_IP6_PREFIX}::1/64
    ```

    You might need to wait a bit for address configuration, you can check this
    with the `ifconfig` command in your nodes terminal.

9. Publish new firmware:

    ```
    $ make -C apps/node_air_monitor suit/publish
    ```

10. Trigger the update process on the device

    ```
    $ make -C apps/node_air_monitor suit/notify
    ```

Repeat 9 and 10 as you want.
