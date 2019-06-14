#!/usr/bin/env python3

import argparse
import logging
import os
import pexpect
import signal
import subprocess
import sys
import time

DEMO_RESET     = 300
DEMO_PERIOD    = 1
TIMEOUT        = 10

LOG_HANDLER = logging.StreamHandler()
LOG_HANDLER.setFormatter(logging.Formatter(logging.BASIC_FORMAT))
LOG_LEVELS = ('debug', 'info', 'warning', 'error')

SUIT_DIR = os.path.dirname(sys.argv[0])
BASE_DIR = os.path.abspath(os.path.join(SUIT_DIR, '..'))
COAPROOT = os.path.join(BASE_DIR, 'firmwares/ota')

OTA_SERVER_MAKEFILE = os.path.join(BASE_DIR, 'Makefiles/suit.v4.http.mk')

START_APP = 'node/empty'

def wait_for_update(child):
    try:
        while True:
            child.expect(r"riotboot_flashwrite: processing bytes (\d+)-(\d+)",
                         timeout=TIMEOUT)
    except pexpect.TIMEOUT:
        child.expect_exact(
            "riotboot_flashwrite: riotboot flashing completed successfully",
            timeout=TIMEOUT)

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
    process = pexpect.spawn(' '.join(cmd), cwd=os.path.expanduser(app_dir), encoding='utf-8')
    return process


def make_flash(board, cwd_dir, make_args):
    logger.info('Initial Flash of {}'.format(board))
    cmd = ['make', 'clean', 'riotboot/flash-extended-slot0', 'BOARD={}'.format(board)]
    cmd.extend(make_args)
    assert not subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


def make_flash_only(board, cwd_dir, make_args):
    logger.info('Initial Flash of {}'.format(board))
    cmd = ['make', 'riotboot/flash-only-extended-slot0', 'BOARD={}'.format(board)]
    cmd.extend(make_args)
    assert not subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


def notify(board, server_url, client_url, cwd_dir, mode, tag):

    if mode is True:
        cmd = ['make','suit/notify', 'BOARD={}'.format(board),
            'SUIT_PUBLISH_ID={}'.format(tag),
            'SUIT_OTA_SERVER_URL={}'.format(server_url),
            'SUIT_CLIENT={}'.format(client_url),
            'SUIT_MAKEFILE={}'.format(OTA_SERVER_MAKEFILE)]
    else:
        cmd = ['make', 'suit/notify', 'BOARD={}'.format(board),
            'SUIT_MANIFEST_SIGNED_LATEST={}'.format(tag),
            'SUIT_COAP_SERVER={}'.format(server_url),
            'SUIT_COAP_FSROOT={}'.format(COAPROOT),
            'SUIT_CLIENT={}'.format(client_url)]
    assert not subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


PARSER = argparse.ArgumentParser(
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
PARSER.add_argument('tags', type=list_from_string,
                    help='List of firmwares to loop over')
PARSER.add_argument('--app-base', default='apps/node_empty',
                    help='List of applications publish')
PARSER.add_argument('--board', default='samr21-xpro',
                    help='Board to test')
PARSER.add_argument('--http', default=False, action='store_true',
                    help='Use http server')
PARSER.add_argument('--loglevel', choices=LOG_LEVELS, default='info',
                    help='Python logger log level')
PARSER.add_argument('--make', type=list_from_string, default=None,
                    help='Additional make arguments')
PARSER.add_argument('--flash', default=False, action='store_true',
                    help='Flashes target node with new firmware, Default False')
PARSER.add_argument('--flash-only', default=False, action='store_true',
                    help='Flashes target node , Default False')
PARSER.add_argument('--port', default='/dev/ttyACM0',
                    help='Node serial port.')
PARSER.add_argument('--serial',
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
    board       = args.board
    host        = args.server
    port        = args.port
    make_args   = args.make
    tags        = args.tags
    http        = args.http

    if args.serial is not None:
        make_args.append("DEBUG_ADAPTER_ID={}".format(args.serial))

    term = None

    try:
        while True:
            # Reset Device to demo start up and open terminal
            make_flash_only(board, app_base, make_args)
            term = make_term(board, app_base, port)
            make_reset(board, app_base, port, make_args)

            # Get device global address
            term.expect(r'inet6 addr: (?P<gladdr>[0-9a-fA-F:]+:[A-Fa-f:0-9]+)'
                        '  scope: global', timeout=TIMEOUT)
            client = '[{}]'.format(term.match.group("gladdr").lower())
            # Leave some time for discovery discovery
            time.sleep(3)

            for tag in tags:
                time.sleep(DEMO_PERIOD)
                term.expect_exact('suit_coap: started.', timeout=TIMEOUT)
                notify(board, host, client, app_base, http, tag)
                wait_for_update(term)
            
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
