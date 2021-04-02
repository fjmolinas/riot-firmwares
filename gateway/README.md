# Gateway

This directory should hold what is required to:

- OpenWSN border router
- GNRC ethos border router
- GNRC cdc-ecm border router

It also holds `docker-compose.yml` to deploy a [pyaiot](https://github.com/pyaiot/pyaiot)
`aiot-coap-gateway` as well as an [otaserver](https://github.com/aabadie/ota-server).

## Deploy coap-gw and ota-server

Add a docker-machine:

```
$ docker-machine create --driver generic --generic-ssh-user <rpi-user> --generic-ip-address=<ip-address> some-name
```

Load the machine:

```
$ eval "$(docker-machine env some-name)"
```

Deploy the coap-gw and the ota-server

```
$ cd docker
$ docker-compose up -d
```

The default docker-compose will run the `coap-gw` on port 5685 and expects
to connect to the broker on port 8000.

The ota server will service the web page on port 8080 and will expose the
COAP endpoint on port 5684.

## Troubleshooting

### GNRC cdc-ecm Border router

When using cdc-ecm UHCPD is used, make sure that DHCP does not run
on the cdc-ecm interface.
