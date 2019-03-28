# Overview

This example application adds suit update capability to the basis node_suit
application. This is the fallback implementation and uses ethos instead of
over the air communication.

## Prerequisites

Dependencies:
    
    Python3 : - ed25519 > 1.4
              - pyasn1  > 0.4.5
              - cbor    > 1.0.0 
              - aiocoap > 0.4
              - click   > 7.0

When this was implemented aiocoap > 0.4 must be built from source you can follow 
installation instructions here https://aiocoap.readthedocs.io/en/latest/installation.html.
If you don't choose to clone the repo locally you still need to download "aiocoap-filesever"
 from https://github.com/chrysn/aiocoap/blob/master/contrib/aiocoap-fileserver.

- RIOT repository checked out into $RIOTBASE

(*) cbor is installed as a dependency of aiocoap but is needed on its own if another
server is used.

# Setup

### Initial flash

In order to get a SUIT capable firmware onto the node. In apps/node_suit:

    $ BOARD=samr21-xpro make  clean riotboot/flash -j4

### Setup network

First, you need to compile `ethos`.
Go to `/dist/tools/ethos` and type:

    $ make clean all

Then, you need to compile UHCP.
This tool is found in `/dist/tools/uhcpd`. So, as for `ethos`:

    $ make clean all

In one shell and with the board already flashed and connected to /dev/ttyACM0:

    $ cd $RIOTBASE/dist/tools/ethos
    $ sudo ./start_network.sh /dev/ttyACM0 riot0 fd00::1/64

Once everyhting is configured you will get:

    ...

    Iface  7  HWaddr: 00:22:09:17:DD:59
            L2-PDU:1500 MTU:1500  HL:64  RTR
            Source address length: 6
            Link type: wired
            inet6 addr: fe80::222:9ff:fe17:dd59  scope: local  TNT[1]
            inet6 addr: fe80::2  scope: local  VAL
            inet6 group: ff02::2
            inet6 group: ff02::1
            inet6 group: ff02::1:ff17:dd59
            inet6 group: ff02::1:ff00:2

    suit_coap: started.

Keep this running (don't close the shell).

Add a routable address to host:

    $ sudo ip address add fd01::1/128 dev riot0

Start aiocoap-fileserver:

    $ mkdir ${RIOTBASE}/coaproot
    $ <PATH>/aiocoap-fileserver ${RIOTBASE}/coaproot

If aiocoap was cloned and built from source aiocoap-filserver will be located
at <AIOCOAP_BASE_DIR>/aiocoap/contrib.

### Key Generation

To sign the manifest a key must be generated.

In apps/node_suit:

    $ BOARD=samr21-xpro make suit/genkey

You will get this message in the terminal:

    the public key is b'a0fc7fe714d0c81edccc50c9e3d9e6f9c72cc68c28990f235ede38e4553b4724'

### Publish

Currently, the build system assumes that it can publish files by simply copying
them to a configurable folder. For this example, aiocoap-fileserver will then
serve the files via CoAP.

Manifests and image files will be copied to
$(SUIT_COAP_FSROOT)/$(SUIT_COAP_BASEPATH).

In apps/node_suit:

    $ BOARD=samr21-xpro SUIT_COAP_SERVER=[fd01::1] make suit/publish

This will publish into the server new firmware for a samr21-xpro board. You should
see 6 pairs of messages indicating where (filepath) the file was published and
the coap resource URI

    ...
    published "/home/francisco/workspace/RIOT/apps/node_suit/bin/samr21-xpro/suit_update-slot0.riot.suit.1553182001.bin"
        as "coap://[fd01::1]/fw/samr21-xpro/suit_update-slot0.riot.suit.1553182001.bin"
    ...


### Notify node

If the network has been started as described above, the RIOT node should be
reachable via link-local "fe80::2" on the ethos interface.

    $ SUIT_COAP_SERVER='[fd01::1]' SUIT_CLIENT=[fe80::2%riot0] BOARD=samr21-xpro make suit/notify

This will notify the node of new available manifest and it will fetch it. It will
take some time to fetch and write to flash, you will a series of messages like:

    ....
    riotboot_flashwrite: processing bytes 1344-1407
    riotboot_flashwrite: processing bytes 1408-1471
    riotboot_flashwrite: processing bytes 1472-1535
    ...

Once the new image is written, the device will reboot and display:

    main(): This is RIOT! (Version: 2019.04-devel-606-gaa7b-ota_suit_v2)
    RIOT SUIT update example application
    running from slot 1
    Waiting for address autoconfiguration...

The slot number should have changed form when you started the application.
You can do the publish-notify sequence again to verify this.
