## CI test

## Prerequisites

- Install python dependencies (only Python3.6 and later is supported):

      $ pip3 install --user ed25519 pyasn1 cbor click tornado

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

- In all setup below, `ethos` (EThernet Over Serial) is used to provide an IP
  link between the host computer and a board.

  Just build `ethos` and `uhcpd` with the following commands:

      $ make -C dist/tools/ethos clean all
      $ make -C dist/tools/uhcpd clean all

  It is possible to interact with the device over it's serial terminal as usual
  using `make term`, but that requires an already set up tap interface.
  See [update] for more information.

## Run the test

This test can run on any of the provided application supporting suit.
It will require that firmware has already been publish to a running
firmware server.

- setup: using the `demo_setup.py` script:
   - publish two versions of the start app
   - start the fileserver and keep it running

```
    $ python scripts/demo_setup.py --publish --http --server="http://localhost:8080" --fileserver --start-bin --start-app="apps/node_combo" --manifests="manifest_1 manifest_2" --applications="apps/node_combo apps/node_combo --keys"
```

See the `demo_setup.py` script `README.md` for more details.

- run the test:

```
    $ MANIFESTS="manifest_1 manifest_2" make -C ci test
```

MANIFESTS matches the list of pre-compiled manifests that where already
published to the appropriate firmware server.

If an external makefile is to be provided for notifying or publishing updates
add SUIT_MAKEFILE=$SUIT_MAKEFILE_PATH to your command. 