# Automatic test

## Prerequisites
[prerequisites]: #Prerequisites

- Install python dependencies (only Python3.6 and later is supported):

      $ pip3 install --user ed25519 pyasn1 cbor click

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

- Clone this repository:

      $ git clone https://github.com/RIOT-OS/RIOT
      $ cd RIOT

- In all setup below, `ethos` (EThernet Over Serial) is used to provide an IP
  link between the host computer and a board.

  Just build `ethos` and `uhcpd` with the following commands:

      $ make -C dist/tools/ethos clean all
      $ make -C dist/tools/uhcpd clean all

  It is possible to interact with the device over it's serial terminal as usual
  using `make term`, but that requires an already set up tap interface.
  See [update] for more information.


## Usage

This test can be performed on any of the applications that

- make sure aiocoap-fileserver is in $PATH