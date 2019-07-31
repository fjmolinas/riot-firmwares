#!/usr/bin/env python3

# Copyright (C) 2019 Inria
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import os
import subprocess
import sys

from testrunner import run

# Custom Timeouts
UPDATING_TIMEOUT = 10
MANIFEST_TIMEOUT = 15

# If available use user defined tags for manifests, Default: manifests-1/2
MANIFESTS = os.getenv('MANIFESTS', 'manifest-1 manifest-2').split(' ')

# Get external makefile
SUIT_MAKEFILE = os.getenv('SUIT_MAKEFILE', 'local')

# Default don't use ethos, doesn't fit in samr21-xpro.
USE_ETHOS = int(os.getenv('USE_ETHOS', '0'))
TAP = os.getenv('TAP', 'tap0')

# Default test over localhost
SUIT_COAP_SERVER = os.getenv('SUIT_COAP_SERVER', 'localhost')


def wait_for_update(child):
    return child.expect([r"riotboot_flashwrite: processing bytes (\d+)-(\d+)",
                            "riotboot_flashwrite: riotboot flashing "
                            "completed successfully"],
                            timeout=UPDATING_TIMEOUT)


def notify(server_url, client_url, manifest):
    if SUIT_MAKEFILE == 'local':
        cmd = [
            'make',
            'suit/notify',
            'SUIT_MANIFEST_SIGNED_LATEST={}'.format(manifest),
            'SUIT_COAP_SERVER={}'.format(server_url),
            'SUIT_CLIENT={}'.format(client_url)
        ]
    else:
        cmd = [
            'make',
            'suit/notify',
            'APPLICATION={}'.format(manifest),
            'SUIT_COAP_SERVER={}'.format(server_url),
            'SUIT_CLIENT={}'.format(client_url)
        ]

    assert not subprocess.call(cmd)


def testfunc(child):
    """For one board test if specified application is updatable"""

    # Initial Setup and wait for address configuration
    child.expect_exact('main(): This is RIOT!')

    if USE_ETHOS is 0:
        # Get device global address
        child.expect(r'inet6 addr: (?P<gladdr>[0-9a-fA-F:]+:[A-Fa-f:0-9]+)'
                    '  scope: global')
        client = "[{}]".format(child.match.group("gladdr").lower())
    else:
        # Get device local address
        client = "[fe80::2%{}]".format(TAP)

    for manifest in MANIFESTS:
        # Wait for suit_coap thread to start
        child.expect_exact('suit_coap: started.')
        # Trigger update process, verify it validates manifest correctly
        notify(SUIT_COAP_SERVER, client, manifest)
        child.expect_exact('suit_coap: trigger received')
        child.expect_exact('suit: verifying manifest signature...')
        child.expect(
            r'riotboot_flashwrite: initializing update to target slot (\d+)',
            timeout=MANIFEST_TIMEOUT)
        target_slot = int(child.match.group(1))
        # Wait for update to complete
        # Wait for update to complete
        while wait_for_update(child) == 0:
            pass
        # Verify running slot
        child.expect(r'running from slot (\d+)')
        assert target_slot == int(child.match.group(1)), "BOOTED FROM SAME SLOT"

    print("TEST PASSED")


if __name__ == "__main__":
    sys.exit(run(testfunc, echo=True))
