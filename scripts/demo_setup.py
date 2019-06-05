#!/usr/bin/env python3

import sys
import os
import logging
import subprocess
import argparse
import time


LOG_HANDLER = logging.StreamHandler()
LOG_HANDLER.setFormatter(logging.Formatter(logging.BASIC_FORMAT))
LOG_LEVELS = ('debug', 'info', 'warning', 'error')

SUIT_DIR  = os.path.dirname(sys.argv[0])
RIOT_DIR  = os.path.abspath(os.path.join(SUIT_DIR, '../../RIOT'))
BASE_DIR  = os.path.abspath(os.path.join(SUIT_DIR, '..'))
COAPROOT  = os.path.join(BASE_DIR, 'firmwares/ota')
OTASERVER = os.path.join(BASE_DIR, '../ota-server')
OTA_SERVER_MAKEFILE = os.path.join(BASE_DIR, 'Makefiles/suit.v4.http.mk')

def get_make_args(jobs, args):
    if jobs is not None:
        make_args = ['-j' + str(jobs)]
    else:
        make_args = ['-j1']
    if args is not None:
        make_args.extend(args)
    return make_args


def list_from_string(list_str=None):
    value = (list_str or '').split(' ')
    return [v for v in value if v]


def setup_aiocoap(cwd_dir):
    logger.info('Setting up aiocoap-fileserver')
    cmd = ['./aiocoap/aiocoap-fileserver', COAPROOT]
    process = subprocess.Popen(cmd, cwd=os.path.expanduser(cwd_dir),
                               stdout=subprocess.DEVNULL)
    return process


def setup_otaserver(cwd_dir):
    logger.info('Setting up ota-server')
    cmd = ['python3', 'otaserver/main.py', '--http-port=8080', '--coap-port=5683',
           '--coap-host=[fd00:dead:beef::1]']
    process = subprocess.Popen(cmd, cwd=os.path.expanduser(cwd_dir),
                               stdout=subprocess.DEVNULL)
    return process


def setup_ethos(port, prefix, cwd_dir):
    cmd = ['sudo', './dist/tools/ethos/start_network.sh', port, 'suit0', prefix]
    process = subprocess.Popen(cmd, cwd=os.path.expanduser(cwd_dir),
                               preexec_fn=os.setpgrp,
                               stdout=subprocess.DEVNULL)
    return process


def make_delkeys(cwd_dir):
    logger.info('Removing old Keys in {}'.format(cwd_dir))
    cmd = ['rm', '-f', 'public.key', 'public_key.h', 'secret.key']
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir),
                    stdout=subprocess.DEVNULL)


def make_genkey(cwd_dir):
    logger.info('Generating keys at {}'.format(cwd_dir))
    cmd = ['make', 'suit/genkey']
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir),
                    stdout=subprocess.DEVNULL)


def make_reset(board, cwd_dir, port):
    logger.info('Reseting board {}'.format(board))
    cmd = ['make', 'reset', 'BOARD={}'.format(board), 'PORT={}'.format(port)]
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir),
                    stdout=subprocess.DEVNULL)


def make_flash(board, cwd_dir, make_args):
    logger.info('Initial Flash of {}'.format(board))
    cmd = ['make', 'clean', 'riotboot/flash-extended-slot0',
           'BOARD={}'.format(board)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


def make_flash_only(board, cwd_dir, make_args):
    logger.info('Initial Flash of {}'.format(board))
    cmd = ['make', 'riotboot/flash-only-extended-slot0',
           'BOARD={}'.format(board)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


def make_publish(board, server_url, cwd_dir, make_args, mode, tag):
    logger.info('Publishing  %s Firmware to %s', cwd_dir, server_url)
    if mode is True:
        cmd = ['make','suit/publish', 'BOARD={}'.format(board),
            'SUIT_PUBLISH_ID={}'.format(tag),
            'SUIT_OTA_SERVER_URL={}'.format(server_url),
            'SUIT_MAKEFILE={}'.format(OTA_SERVER_MAKEFILE)]
    else:
        cmd = ['make','suit/publish', 'BOARD={}'.format(board),
            'SUIT_MANIFEST_SIGNED_LATEST={}'.format(tag),
            'SUIT_COAP_SERVER={}'.format(server_url),
            'SUIT_COAP_FSROOT={}'.format(COAPROOT)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


PARSER = argparse.ArgumentParser(
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
PARSER.add_argument('--applications', default='apps/node_leds',
                    help='List of applications publish', type=list_from_string)
PARSER.add_argument('--app-base', default='apps/node_empty',
                    help='List of applications publish')
PARSER.add_argument('--board-node', default='samr21-xpro',
                    help='Board to test')
PARSER.add_argument('--board-ethos', default='iotlab-m3',
                    help='Board to test')
PARSER.add_argument('--ethos', default=False, action='store_true',
                    help='True if test is to be setup locally over ethos.')
PARSER.add_argument('--http', default=False, action='store_true',
                    help='Use http server')
PARSER.add_argument('--fileserver', default=False, action='store_true',
                    help='Start fileserver, Default=True')
PARSER.add_argument('--jobs', '-j', type=int, default=None,
                    help="Parallel building (0 means no limit, like '--jobs')")
PARSER.add_argument('--keys', default=False, action='store_true',
                    help='Remove old keys and generate new ones, Default False')
PARSER.add_argument('--loglevel', choices=LOG_LEVELS, default='info',
                    help='Python logger log level')
PARSER.add_argument('--make', type=list_from_string, default=None,
                    help='Additional make arguments')
PARSER.add_argument('--flash', default=False, action='store_true',
                    help='Flashes target node with new firmware, Default False')
PARSER.add_argument('--flash-only', default=False, action='store_true',
                    help='Flashes target node , Default False')
PARSER.add_argument('--port-ethos', default='/dev/ttyUSB1',
                    help='Ethos serial port.')
PARSER.add_argument('--port-node', default='/dev/ttyACM0',
                    help='Node serial port.')
PARSER.add_argument('--prefix', default='2001:db8::1/64',
                    help='Prefix to propagate over ethos.')
PARSER.add_argument('--publish', default=False, action='store_true',
                    help='Published new Firmware , Default=False')
PARSER.add_argument('--riot_dir', default=RIOT_DIR,
                    help='Base Directory for RIOT')
PARSER.add_argument('--server', default='[fd00:dead:beef::1]',
                    help='Server url.')
PARSER.add_argument('--tags', type=list_from_string, default='latest',
                    help='List of manifest tags to publish')


if __name__ == "__main__":
    """For one board test if specified application is updatable"""
    args = PARSER.parse_args()

    logger = logging.getLogger()
    if args.loglevel:
        loglevel = logging.getLevelName(args.loglevel.upper())
        logger.setLevel(loglevel)
    logger.addHandler(LOG_HANDLER)

    app_dirs    = args.applications
    app_base    = args.app_base
    riot_dir    = args.riot_dir
    board_node  = args.board_node
    board_ethos = args.board_ethos
    host        = args.server
    http        = args.http
    port_ethos  = args.port_ethos
    port_node   = args.port_node
    prefix      = args.prefix
    make_args   = get_make_args(args.jobs, args.make)
    tags        = args.tags

    childs = []

    try:
        # Setup Ethos
        if args.ethos is True:
            make_reset(board_ethos, app_base, port_ethos)
            ethos = setup_ethos(port_ethos, prefix, riot_dir)
            childs.append(ethos)
            time.sleep(1)
            logger.info("Ethos pid {} and group pid {}".format(ethos.pid,
                        os.getpgid(ethos.pid)))

        # Setup File Sever
        if args.fileserver is True:
            if args.http is False:
                childs.append(setup_aiocoap(BASE_DIR))
            else:
                childs.append(setup_otaserver(OTASERVER))

        # Delete old key and generate new ones
        if args.keys is True:
            for app_dir in app_dirs:
                make_delkeys(app_dir)
                make_genkey(app_dir)

        # Provide node, initial flash
        if args.flash is True:
            make_flash(board_node, app_base, make_args)
            make_reset(board_node, app_base, port_node)

        # Flash only
        if args.flash_only is True:
            make_flash_only(board_node, app_base, make_args)
            make_reset(board_node, app_base, port_node)

        # Publish firmware(s)
        if args.publish is True:
            if len(tags) == len(app_dirs):
                for i in range(0, len(app_dirs)):
                    make_publish(board_node, host, app_dirs[i], make_args, http,
                                 tags[i].format(i))
            else:
                for i in range(0, len(app_dirs)):
                    make_publish(board_node, host, app_dirs[i], make_args, http,
                                 "latest-{}".format(i))

        # Run tests and keep running if fileserver or ethos were setup
        if args.ethos is True or args.fileserver is True:
            while True:
                time.sleep(1)

    except SystemExit as e:
        sys.exit(e)

    finally:
        if childs:
            for process in childs:
                try:
                    gpid = os.getpgid(process.pid)
                    logger.info("Killing group {}".format(gpid))
                    subprocess.check_call(["sudo", "kill", '-{}'.format(gpid)])
                except:
                    logger.info("Failed to stop process {}".format(process.pid))