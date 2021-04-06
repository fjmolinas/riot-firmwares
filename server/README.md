### Docker Server Deployment

We need some certificates to allow CI to connect to the daemon. If you’re
a TLS certificate wizard, you’ll whip out your openssl wand and make secure
dreams come true. If you’re a mere mortal like me, you can use:
https://github.com/XIThing/generate-docker-client-certs/. Clone that repo
and run:

    ./generate-client-certs.sh ~/.docker/machine/certs/ca.pem ~/.docker/machine/certs/ca-key.pem

Setup the secrets used in the deploy action, and voila.
