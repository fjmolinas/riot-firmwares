#!/usr/bin/env python3

# TODO: add tox.ini tests

import argparse
import logging
import os
import pexpect
import signal
import subprocess
import sys
import yaml
import time

UPDATING_TIMEOUT = 10
MANIFEST_TIMEOUT = 15

LOG_HANDLER = logging.StreamHandler()
LOG_HANDLER.setFormatter(logging.Formatter(logging.BASIC_FORMAT))
LOG_LEVELS = ('debug', 'info', 'warning', 'error')

SUIT_DIR = os.path.dirname(sys.argv[0])
BASE_DIR = os.path.abspath(os.path.join(SUIT_DIR, '..'))
CONFIG_DIR = os.path.join(SUIT_DIR, 'config')
MANIFEST_DIR = os.path.join(BASE_DIR, 'firmwares/ota')

os.environ['SUIT_MAKEFILE'] = os.path.join(BASE_DIR, 'Makefiles/suit.v4.http.mk')

# Default don't use ethos, doesn't fit in samr21-xpro.
TAP = os.getenv('TAP', 'tap0')

DEFAULT_CONFIG =  os.path.join(CONFIG_DIR, 'demo_config.yml')
MAKE_DIR = os.path.join(SUIT_DIR, 'reset')


def wait_for_update(child):
        return child.expect([r"riotboot_flashwrite: processing bytes (\d+)-(\d+)",
                             "riotboot_flashwrite: riotboot flashing "
                             "completed successfully"],
                             timeout=UPDATING_TIMEOUT)


# TODO: create exceptions to handle failures
def make_reset(board, cwd_dir, port, make_args):
    logger.info('Reseting board {}'.format(board))
    cmd = ['make', 'reset', 'BOARD={}'.format(board), 'PORT={}'.format(port)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


def make_term(board, app_dir, port, make_args):
    logger.info('Setting up Terminal {}'.format(board))
    cmd = ['make', 'term', 'BOARD={}'.format(board), 'PORT={}'.format(port)]
    cmd.extend(make_args)
    process = pexpect.spawn(' '.join(cmd), cwd=os.path.expanduser(app_dir),
                            encoding='utf-8')
    return process


def make_flash_bin(board, cwd_dir, binfile, make_args):
    logger.info('Initial Flash of {}'.format(board))
    cmd = [
        'make',
        'FLASHFILE={}'.format(binfile),
        'flash-only',
        'BOARD={}'.format(board)
        ]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


def notify(board, server_url, client_url, cwd_dir, mode, manifest):
    logger.info('Notifying {}'.format(client_url))
    if mode == 'http':
        cmd = [
            'make',
            'suit/notify',
            'BOARD={}'.format(board),
            'APPLICATION={}'.format(manifest),
            'SUIT_OTA_SERVER_URL={}'.format(server_url),
            'SUIT_CLIENT={}'.format(client_url),
        ]
    else:
        cmd = [
            'make',
            'suit/notify',
            'BOARD={}'.format(board),
            'SUIT_MANIFEST_SIGNED_LATEST={}'.format(manifest),
            'SUIT_COAP_SERVER={}'.format(server_url),
            'SUIT_COAP_FSROOT={}'.format(MANIFEST_DIR),
            'SUIT_CLIENT={}'.format(client_url)]
    subprocess.Popen(cmd, cwd=os.path.expanduser(cwd_dir))


def parse_command_line():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--loglevel', choices=LOG_LEVELS, default='info',
                        help='Python logger log level')
    parser.add_argument('--config', default=DEFAULT_CONFIG,
                        help='Configuration file.')
    parser.add_argument('--device', default='air-monitor-0',
                        help='Device to run demo on.')
    return parser.parse_args()


def read_config(filename):
    with open(filename, 'r') as ymlfile:
        config = yaml.load(ymlfile, Loader=yaml.Loader)
    return config


def get_device_config(config, tag):
    devices = config["devices"]
    for device in devices:
        if device['name'] == tag:
            return device
        if device['uid'] == tag:
            return device
        if device['debug_id'] == tag:
            return device
    return None


def clean_up(term):
    if term is not None:
        try:
            term_pid = os.getpgid(term)
            logger.info("Killing process pid: {}".format(term_pid))
            os.kill(term_pid, signal.SIGKILL)
        except:
            logger.info("Failed to stop process {}".format(term))
        cmd = ['fuser', '-k', port]
        subprocess.call(cmd)

if __name__ == "__main__":
    """For one board test if specified application is updatable"""

    args = parse_command_line()
    config = read_config(args.config)

    logger = logging.getLogger()
    if args.loglevel:
        loglevel = logging.getLevelName(args.loglevel.upper())
        logger.setLevel(loglevel)
    logger.addHandler(LOG_HANDLER)

    device = get_device_config(config, args.device)

    binfile = device['demo']['binfile']
    board = device['board']
    http = device['demo']['coap']['server']
    host = device['demo']['coap']['address']
    port = device['port']
    make_args = device['demo']['make']
    manifests = device['demo']['manifests']
    demo_reset = device['demo']['demo_reset']
    demo_period = device['demo']['demo_period']
    ethos = device['demo']['ethos']
    tap = device['demo']['tap']

    make_args.append("DEBUG_ADAPTER_ID={}".format(device['debug_id']))
    
    term = None

    try:
        while True:

            # Reset Device to demo start state
            make_flash_bin(board, MAKE_DIR, binfile, make_args)

            # Open terminal
            logger.info('Opening terminal on {}'.format(port))
            term = make_term(board, MAKE_DIR, port, make_args)
            make_reset(board, MAKE_DIR, port, make_args)

            if ethos is False:
                # Get device global address
                term.expect(r'inet6 addr: (?P<gladdr>[0-9a-fA-F:]+:[A-Fa-f:0-9]+)'
                            '  scope: global')
                client = "[{}]".format(term.match.group("gladdr").lower())
            else:
                # Get device local address
                client = "[fe80::2%{}]".format(tap)

            term.expect(r'running from slot (\d+)')
            logger.debug('Running from slot {}'.format(term.match.group(1)))
            logger.info('Node address {}'.format(client))

            for manifest in manifests:
                logger.info('Updating every {} s'.format(demo_period))
                time.sleep(demo_period)

                term.expect_exact('suit_coap: started.')
                notify(board, host, client, MAKE_DIR, http, manifest)
                term.expect_exact('suit: verifying manifest signature...')
                term.expect(
                    r'riotboot_flashwrite: initializing update to target slot (\d+)',
                    timeout=MANIFEST_TIMEOUT)
                # Wait for update to complete
                while wait_for_update(term) == 0:
                    print(term.after)

            logger.info('Demo ended, delaying reboot by {} s'.format(demo_reset))
            time.sleep(demo_reset)

    except SystemExit as e:
        sys.exit(e)

    finally:
        clean_up(term)
