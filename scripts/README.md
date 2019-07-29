# Suit updates Demo

In the following folder there are to script that help running the riotfp,
update demo.

## Prerequisites

- Install python dependencies (only Python3.6 and later is supported):

      $ pip3 install -r requirements.txt --user

- Install aiocoap from the source

      $ pip3 install --user --upgrade "git+https://github.com/chrysn/aiocoap#egg=aiocoap[all]"

  See the [aiocoap installation instructions](https://aiocoap.readthedocs.io/en/latest/installation.html)
  for more details.

- Install aiocoap-fileserver

      $ wget https://github.com/chrysn/aiocoap/raw/master/contrib/aiocoap-fileserver -O ~/.local/bin/aiocoap-fileserver
      $ chmod a+x ~/.local/bin/aiocoap-fileserver

- add `~/.local/bin` to PATH

  The aiocoap tools are installed to `~/.local/bin`. Either add
  "export `PATH=$PATH:~/.local/bin"` to your `~/.profile` and re-login, or execute
  that command *in every shell you use for this tutorial*.

- Install [ota-server](https://github.com/aabadie/ota-server) and checkout `signed_v4_manifest` branch.

- In all setup below, `ethos` (EThernet Over Serial) is used to provide an IP
  link between the host computer and a board.

  Just build `ethos` and `uhcpd` with the following commands:

      $ make -C dist/tools/ethos clean all
      $ make -C dist/tools/uhcpd clean all

- For now the supported board is samr21-xpro and since ethos doesn't fit on this board you will need
  a second RIOT device or other 6LowPAN enabled device to act as a border router. If setting up a
  6LowPAN on a rpi, follow this [README](https://github.com/RIOT-OS/RIOT/wiki/How-to-install-6LoWPAN-Linux-Kernel-on-Raspberry-Pi).

  Otherwise a border router can be setup by means of a RIOT node a connected to your computer, to do this
  follow the instructions for [examples/gnrc_border_router](https://github.com/RIOT-OS/RIOT/tree/master/examples/gnrc_border_router)

## demo_setup.py

This script is mainly a utility script to ease up setting up the demo.

### Usage 

To use the script:

    $ python demo_setup.py [flags]

- For all actions:
  - use `--riot-dir` to specify the location for the RIOT folder, 

- To setup the coap server:

  - use `--coapserver` to launch the coap server, either aiocoap or otaserver according
         to `--http` flag. This will also keep the script running until manually terminated.
  - use `--http` to use otaserver instead of the default aiocoap-fileserver. This
    will also determine how publishing happens (a post to the sever url or simply
    copying the files to the aiocoap-host root directory)
  - use `--server=<address>` to specify the server host, default is `[fd00:dead:beef::1]`.
    this is also the address to publish firmware updates.

- Setup the ethos BR:

  - use `--ethos` flag to setup the ethos border router, if this flag is set the script will
    keep running until manually terminated.
  - use `--board-ethos<board>` to specify the riot board used as the ethos BR, default is `iotlab-m3`
  - use `--port-ethos=<serial>` to specify the serial port for the ethos BR, defualt is `/tty/USB1`
  - use `--prefix=<prefix>` to specify the ipv6 prefix to be disseminated by the BR, defualt
    `2001:db8::1/64`

- To publish new manifests:

  - use `--publish` to generate and publish new firmwares and manifests to the coapserver
  - use `--applications=<list>` to pass a string as a list of applications to generate new manifests.
    e.g: `--applications="apps/node_combo apps/node_ccs811"`
  - use `--manifest` to pass a string list of specific tags for the published manifests. If none is
    passed or if it doesn't match the amount of applications they will be tagged as `latest-n` according
    to te applications index.
  - use `--board-node=<board>` to specify the device to generate the new fimwares for
  - use `--keys` to generate new keys to sign the manifest. WARN: this keys must match
    the ones already on the device or the update will fail.
  - use `--make=<args>` to pass additional make arguments for publishing
  - use `--rpi=<address>` this will copy the manifests/firmwares `firmwares` directory onto the `rpi`.
    Included here would be all the start binaries and files published to the aiocoap fileserver. The
    address of the rpi must be specified.

- To flash and create the demo start application:
  - use `--start-bin` to create a new start binary according to `--start-app` it will also flash it on
    the device if connected. It will by default flash the bootloader + application. The binary file
    will be also copied to `firmwares/setup`
  - use `--start-app=<directory>` to specify the starting application, default is `apps/node_empty`
  - use `--start-bin-nm=<name>` to use a custom tag to name th start application binary.
  - use `--make=<args>` to pass additional make arguments for compilation

## demo_run.py

This script will keep the demo running by periodically notifying about a new update to the device. Once
all updates have been applied it will re-flash the start-binary and re-start the cycle.

For the script tu run the required firmware must already have been published and compiled.

### Usage 

Running the demo is as simple as:

    $ python demo_run.py <mainfests> [flags]

A list a of manifests/applications must be passed as an argument to the script. This will specify 
the application to update the device. They mist be in ascending order according to their sequence number.

- use `--app-base` to specify the start application for the demo. Default is `apps/node_empty`
- use `--binfile` to specify the name of the start binary to be flashed to the device. It must be stored
  at `firmwares/setup`
- use `--board` to specify the target device for the update, default is `samr21-xpro`
- use `--debug-id=<id>` to specify the device `DEBUG_ADAPTER_ID`
- use `--flash-only` to start every cycle flasing the compiled binary in `--app-base` BINDIR.
- use `--flash-bin` to start every cycle reflashing `--binfile`.
- use `--http` to specify if the coapserver is otaserver, aiocoap-fileserver is used by default.
- use `--make=<args>` to pass additional make arguments for flashing
- use `--port=<port>` to specify the device serial port for the device, `/dev/ttyACM0` by default.
- use `--server=<address>` to specify the address of the coap host.