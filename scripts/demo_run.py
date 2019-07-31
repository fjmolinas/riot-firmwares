#!/usr/bin/env python3

import argparse
import logging
import os
import pexpect
import signal
import subprocess
import sys
import time

DEMO_RESET     = 5*60
DEMO_PERIOD    = 1
TIMEOUT        = 15

LOG_HANDLER = logging.StreamHandler()
LOG_HANDLER.setFormatter(logging.Formatter(logging.BASIC_FORMAT))
LOG_LEVELS = ('debug', 'info', 'warning', 'error')

SUIT_DIR = os.path.dirname(sys.argv[0])
BASE_DIR = os.path.abspath(os.path.join(SUIT_DIR, '..'))
BIN_DIR   = os.path.join(BASE_DIR, 'firmwares/setup')
COAPROOT = os.path.join(BASE_DIR, 'firmwares/ota')

OTA_SERVER_MAKEFILE = os.path.join(BASE_DIR, 'Makefiles/suit.v4.http.mk')

BIN_FILE = 'samr21-xpro/node_empty-slot0-extended.bin'

def wait_for_update(child):
    try:
        while True:
            child.expect(r"riotboot_flashwrite: processing bytes (\d+)-(\d+)",
                         timeout=TIMEOUT)
            logger.debug(child.after)
    except pexpect.TIMEOUT:
        child.expect_exact(
            "riotboot_flashwrite: riotboot flashing completed successfully",
            timeout=TIMEOUT)
        logger.debug(child.after)


def list_from_string(list_str=None):
    value = (list_str or '').split(' ')
    return [v for v in value if v]


def make_reset(board, cwd_dir, port, make_args):
    logger.info('Reseting board {}'.format(board))
    cmd = ['make', 'reset', 'BOARD={}'.format(board), 'PORT={}'.format(port)]
    cmd.extend(make_args)
    assert not subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir),
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL)


def make_term(board, app_dir, port):
    logger.info('Setting up Terminal {}'.format(board))
    cmd = ['make', 'term', 'BOARD={}'.format(board), 'PORT={}'.format(port)]
    process = pexpect.spawn(' '.join(cmd), cwd=os.path.expanduser(app_dir),
                            encoding='utf-8')
    return process


def make_flash(board, cwd_dir, make_args):
    logger.info('Initial Flash of {}'.format(board))
    cmd = ['make', 'FLASHFILE=$(RIOTBOOT_COMBINED_BIN)' 'clean', 'flash',
           'BOARD={}'.format(board)]
    cmd.extend(make_args)
    assert not subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


def make_flash_only(board, cwd_dir, make_args):
    logger.info('Initial Flash of {}'.format(board))
    cmd = ['make', 'FLASHFILE=$(RIOTBOOT_COMBINED_BIN)', 'flash-only',
           'BOARD={}'.format(board)]
    cmd.extend(make_args)
    assert not subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


def make_flash_bin(board, cwd_dir, binfile, make_args):
    logger.info('Initial Flash of {}'.format(board))
    cmd = ['make', 'BINFILE={}'.format(binfile), 'flash-only',
           'BOARD={}'.format(board)]
    cmd.extend(make_args)
    assert not subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))

def notify(board, server_url, client_url, cwd_dir, mode, manifest):
    logger.info('Notifying {}'.format(client_url))
    if mode is True:
        cmd = ['make','suit/notify', 'BOARD={}'.format(board),
            'APPLICATION={}'.format(manifest),
            'SUIT_OTA_SERVER_URL={}'.format(server_url),
            'SUIT_CLIENT={}'.format(client_url),
            'SUIT_MAKEFILE={}'.format(OTA_SERVER_MAKEFILE)]
    else:
        cmd = ['make', 'suit/notify', 'BOARD={}'.format(board),
            'SUIT_MANIFEST_SIGNED_LATEST={}'.format(manifest),
            'SUIT_COAP_SERVER={}'.format(server_url),
            'SUIT_COAP_FSROOT={}'.format(COAPROOT),
            'SUIT_CLIENT={}'.format(client_url)]
    subprocess.Popen(cmd, cwd=os.path.expanduser(cwd_dir))


PARSER = argparse.ArgumentParser(
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
PARSER.add_argument('manifests', type=list_from_string,
                    help='List of firmwares to loop over')
PARSER.add_argument('--app-base', default='apps/node_empty',
                    help='List of applications publish')
PARSER.add_argument('--binfile', default=os.path.join(BIN_DIR, BIN_FILE),
                    help='Start binfile absolute location')
PARSER.add_argument('--board', default='samr21-xpro',
                    help='Board to test')
PARSER.add_argument('--http', default=False, action='store_true',
                    help='Use http server')
PARSER.add_argument('--loglevel', choices=LOG_LEVELS, default='info',
                    help='Python logger log level')
PARSER.add_argument('--make', type=list_from_string, default='-j1',
                    help='Additional make arguments')
PARSER.add_argument('--flash-only', default=False, action='store_true',
                    help='Flashes target node , Default False')
PARSER.add_argument('--flash-bin', default=False, action='store_true',
                    help='Flashes specific bin file, Default False')
PARSER.add_argument('--port', default='/dev/ttyACM0',
                    help='Node serial port.')
PARSER.add_argument('--debug-id',
                    help='device DEBUG_ADAPTER_ID.')
PARSER.add_argument('--server', default='[fd00:dead:beef::1]',
                    help='Server url.')


if __name__ == "__main__":
    """For one board test if specified application is updatable"""
    args = PARSER.parse_args()

    logger = logging.getLogger()
    if args.loglevel:
        loglevel = logging.getLevelName(args.loglevel.upper())
        logger.setLevel(loglevel)
    logger.addHandler(LOG_HANDLER)

    app_base    = args.app_base
    binfile     = args.binfile
    board       = args.board
    host        = args.server
    port        = args.port
    make_args   = args.make
    manifests   = args.manifests
    http        = args.http

    if args.debug_id is not None:
        make_args.append("DEBUG_ADAPTER_ID={}".format(args.debug_id))

    term = None

    try:
        while True:

            # Reset Device to demo start state
            if args.flash_bin is True:
                make_flash_bin(board, app_base, binfile, make_args)
            if args.flash_only is True:
                make_flash_only(board, app_base, make_args)

            # Open terminal
            logger.info('Opening terminal on {}'.format(port))
            term = make_term(board, app_base, port)
            make_reset(board, app_base, port, make_args)

            # Get device global address
            term.expect(r'inet6 addr: (?P<gladdr>[0-9a-fA-F:]+:[A-Fa-f:0-9]+)'
                        '  scope: global', timeout=TIMEOUT)
            client = '[{}]'.format(term.match.group("gladdr").lower())
            term.expect(r'running from slot (\d+)')
            logger.debug('Running from slot {}'.format(term.match.group(1)))
            logger.info('Node address {}'.format(client))
            # Leave some time for discovery discovery
            time.sleep(3)

            for manifest in manifests:
                logger.info('Updating every {} s'.format(DEMO_PERIOD))
                time.sleep(DEMO_PERIOD)
                term.expect_exact('suit_coap: started.', timeout=TIMEOUT)
                notify(board, host, client, app_base, http, manifest)
                wait_for_update(term)

            logger.info('Demo ended, delaying reboot by {} s'.format(DEMO_RESET))
            time.sleep(DEMO_RESET)

    except SystemExit as e:
        sys.exit(e)

    finally:
        if term:
            try:
                term_pid = os.getpgid(term)
                logger.info("Killing process pid: {}".format(term_pid))
                os.kill(term_pid, signal.SIGKILL)
            except:
                logger.info("Failed to stop process {}".format(term))
            cmd = ['fuser', '-k', port]
            subprocess.call(cmd)

