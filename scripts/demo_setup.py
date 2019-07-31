#!/usr/bin/env python3

import sys
import os
import logging
import subprocess
import argparse
import glob
import time


LOG_HANDLER = logging.StreamHandler()
LOG_HANDLER.setFormatter(logging.Formatter(logging.BASIC_FORMAT))
LOG_LEVELS = ('debug', 'info', 'warning', 'error')

SUIT_DIR  = os.path.dirname(sys.argv[0])
RIOT_DIR  = os.path.abspath(os.path.join(SUIT_DIR, '../../RIOT'))
BASE_DIR  = os.path.abspath(os.path.join(SUIT_DIR, '..'))
COAPROOT  = os.path.join(BASE_DIR, 'firmwares/ota')
BIN_DIR   = os.path.join(BASE_DIR, 'firmwares/setup')
OTASERVER = os.path.join(BASE_DIR, '../ota-server')
OTA_SERVER_MAKEFILE = os.path.join(BASE_DIR, 'Makefiles/suit.v4.http.mk')


def list_from_string(list_str=None):
    value = (list_str or '').split(' ')
    return [v for v in value if v]


def setup_aiocoap(cwd_dir):
    logger.info('Setting up aiocoap-fileserver')
    cmd = ['./aiocoap/aiocoap-fileserver', COAPROOT]
    process = subprocess.Popen(cmd, cwd=os.path.expanduser(cwd_dir),
                               stdout=subprocess.DEVNULL)
    return process


def setup_otaserver(cwd_dir, server_url):
    logger.info('Setting up ota-server')
    cmd = ['python3', 'otaserver/main.py', '--http-port=8080', '--coap-port=5683',
           '--coap-host={}'.format(server_url)]
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
    subprocess.call(cmd, cwd=os.path.join(BASE_DIR, cwd_dir),
                    stdout=subprocess.DEVNULL)


def make_genkey(cwd_dir):
    logger.info('Generating keys at {}'.format(cwd_dir))
    cmd = ['make', 'suit/genkey']
    subprocess.call(cmd, cwd=os.path.join(BASE_DIR, cwd_dir),
                    stdout=subprocess.DEVNULL)

def make_flash(board, start_app, make_args):
    logger.info('Initial build of {}'.format(board))
    cmd = ['make', 'FLASHFILE=$(RIOTBOOT_EXTENDED_BIN)', 'clean', 'flash',
           'BOARD={}'.format(board)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.join(BASE_DIR, start_app))


def start_bin(board, start_app, start_bin):
    cmd = ['mkdir', '-p', '{}/{}/'.format(BIN_DIR, board)]
    subprocess.call(cmd)
    files = glob.glob('{}/{}/bin/{}/*-slot0-extended.bin'.format(BASE_DIR,
                                                                 start_app,
                                                                 board))
    if files:
        cmd = ['cp', files[0], '{}/{}/'.format(BIN_DIR, board)]
        subprocess.call(cmd)
    if start_bin:
        cmd = ['mv', os.path.basename(files[0]), '{}.bin'.format(start_bin)]
        subprocess.call(cmd, cwd='{}/{}/'.format(BIN_DIR, board))


def update_pi(rpi):
    cmd = ['scp', '-r', '{}/'.format(BIN_DIR),
           'pi@{}:/opt/src/riot-firmwares/firmwares'.format(rpi)]
    subprocess.call(cmd, cwd=BASE_DIR)


def make_reset(board, cwd_dir, port, make_args):
    logger.info('Reseting board {}'.format(board))
    cmd = ['make', 'reset', 'BOARD={}'.format(board), 'PORT={}'.format(port)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir),
                    stdout=subprocess.DEVNULL)


def make_publish(board, server_url, cwd_dir, make_args, mode, manifest):
    logger.info('Publishing  %s Firmware to %s', cwd_dir, server_url)
    if mode is True:
        cmd = ['make','suit/publish', 'BOARD={}'.format(board),
            'APPLICATION={}'.format(manifest),
            'SUIT_OTA_SERVER_URL={}'.format(server_url),
            'SUIT_MAKEFILE={}'.format(OTA_SERVER_MAKEFILE)]
    else:
        cmd = ['make','suit/publish', 'BOARD={}'.format(board),
            'SUIT_MANIFEST_SIGNED_LATEST={}'.format(manifest),
            'SUIT_COAP_SERVER={}'.format(server_url),
            'SUIT_COAP_FSROOT={}'.format(COAPROOT)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.join(BASE_DIR, cwd_dir))


PARSER = argparse.ArgumentParser(
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
PARSER.add_argument('--applications', default='apps/node_leds',
                    help='List of applications publish', type=list_from_string)
PARSER.add_argument('--board-node', default='samr21-xpro',
                    help='Board to test')
PARSER.add_argument('--board-ethos', default='iotlab-m3',
                    help='Board to test')
PARSER.add_argument('--ethos', default=False, action='store_true',
                    help='True if test is to be setup locally over ethos.')
PARSER.add_argument('--http', default=False, action='store_true',
                    help='Use http server')
PARSER.add_argument('--coapserver', default=False, action='store_true',
                    help='Start coapserver, Default=True')
PARSER.add_argument('--keys', default=False, action='store_true',
                    help='Remove old keys and generate new ones, Default False')
PARSER.add_argument('--loglevel', choices=LOG_LEVELS, default='info',
                    help='Python logger log level')
PARSER.add_argument('--make', type=list_from_string, default='-j1',
                    help='Additional make arguments')
PARSER.add_argument('--port-ethos', default='/dev/ttyUSB1',
                    help='Ethos serial port.')
PARSER.add_argument('--prefix', default='2001:db8::1/64',
                    help='Prefix to propagate over ethos.')
PARSER.add_argument('--publish', default=False, action='store_true',
                    help='Published new Firmware , Default=False')
PARSER.add_argument('--riot_dir', default=RIOT_DIR,
                    help='Base Directory for RIOT')
PARSER.add_argument('--rpi', default=None,
                    help='update Rpi files')
PARSER.add_argument('--server', default='[fd00:dead:beef::1]',
                    help='Server url.')
PARSER.add_argument('--start-app', default='apps/node_empty',
                    help='Initial application')
PARSER.add_argument('--start-bin', default=False, action='store_true',
                    help='Save new start bin, Default=False')
PARSER.add_argument('--start-bin-nm', default=None,
                    help='Makes binary file \"<start_bin>.bin\" and copies it to'
                         ' firmwares/setup/<board-node>')
PARSER.add_argument('--manifests', type=list_from_string, default='latest',
                    help='List of manifest manifests to publish')


if __name__ == "__main__":
    """For one board test if specified application is updatable"""
    args = PARSER.parse_args()

    logger = logging.getLogger()
    if args.loglevel:
        loglevel = logging.getLevelName(args.loglevel.upper())
        logger.setLevel(loglevel)
    logger.addHandler(LOG_HANDLER)

    app_dirs    = args.applications
    riot_dir    = args.riot_dir
    board_node  = args.board_node
    board_ethos = args.board_ethos
    host        = args.server
    http        = args.http
    port_ethos  = args.port_ethos
    prefix      = args.prefix
    make_args   = args.make
    manifests   = args.manifests
    start_app   = args.start_app

    childs = []

    try:
        # Setup Ethos
        if args.ethos is True:
            make_reset(board_ethos, start_app, port_ethos, make_args)
            ethos = setup_ethos(port_ethos, prefix, riot_dir)
            childs.append(ethos)
            time.sleep(1)
            logger.info("Ethos pid {} and group pid {}".format(ethos.pid,
                        os.getpgid(ethos.pid)))

        # Setup File Sever
        if args.coapserver is True:
            if args.http is False:
                childs.append(setup_aiocoap(BASE_DIR))
            else:
                childs.append(setup_otaserver(OTASERVER, host ))

        # Delete old key and generate new ones
        if args.keys is True:
            for app_dir in app_dirs:
                make_delkeys(app_dir)
                make_genkey(app_dir)

        # Make new base
        if args.start_bin is True:
            make_flash(board_node, start_app, make_args)
            start_bin(board_node, start_app, args.start_bin_nm)

        # Publish firmware(s)
        if args.publish is True:
            if len(manifests) == len(app_dirs):
                for i in range(0, len(app_dirs)):
                    make_publish(board_node, host, app_dirs[i], make_args, http,
                                 manifests[i].format(i))
            else:
                for i in range(0, len(app_dirs)):
                    make_publish(board_node, host, app_dirs[i], make_args, http,
                                 "latest-{}".format(i))

        # Copy firmware(s) to pi
        if args.rpi is not None:
            update_pi(args.rpi)

        # Run tests and keep running if coapserver or ethos were setup
        if args.ethos is True or args.coapserver is True:
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
            cmd = ['fuser', '-k', port_ethos]
            subprocess.call(cmd)
