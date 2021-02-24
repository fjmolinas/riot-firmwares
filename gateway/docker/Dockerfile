FROM ubuntu:bionic as RIOT

RUN \
    echo 'Update the package index files to latest available versions' >&2 && \
    apt-get update && \
    echo 'Install required packages' >&2 && \
    apt-get -y --no-install-recommends install \
        git \
        gcc \
        make \
        ca-certificates \
        libc-dev \
        && \
    echo 'Clean up installation files' >&2 && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN git clone https://github.com/RIOT-OS/RIOT.git /tmp/RIOT
RUN make -C /tmp/RIOT/dist/tools/ethos
RUN make -C /tmp/RIOT/dist/tools/uhcpd

FROM ubuntu:bionic

RUN \
    echo 'Update the package index files to latest available versions' >&2 && \
    apt-get update && \
    echo 'Install required packages' >&2 && \
    apt-get -y --no-install-recommends install \
        net-tools \
        && \
    echo 'Clean up installation files' >&2 && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


RUN mkdir /ethos
RUN mkdir -p /uhcpd/bin
COPY --from=RIOT /tmp/RIOT/dist/tools/ethos/ethos /ethos/ethos
COPY --from=RIOT /tmp/RIOT/dist/tools/uhcpd/bin/uhcpd /uhcpd/bin/uhcpd
COPY --from=RIOT /tmp/RIOT/dist/tools/ethos/start_network.sh /ethos/start-network.sh

COPY run.sh /run.sh

RUN chmod +x /run.sh
CMD ["/run.sh"]

