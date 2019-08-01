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

SUIT_DIR = os.path.dirname(sys.argv[0])
RIOT_DIR = os.path.abspath(os.path.join(SUIT_DIR, '../../RIOT'))
BASE_DIR = os.path.abspath(os.path.join(SUIT_DIR, '..'))
COAPROOT = os.path.join(BASE_DIR, 'firmwares/ota')
SETUP_DIR = os.path.join(BASE_DIR, 'firmwares/setup')

os.environ['SUIT_MAKEFILE'] = os.path.join(BASE_DIR, 'Makefiles/suit.v4.http.mk')

OTASERVER = os.path.join(BASE_DIR, '../ota-server')


def list_from_string(list_str=None):
    value = (list_str or '').split(' ')
    return [v for v in value if v]


def setup_aiocoap():
    logger.info('Setting up aiocoap-fileserver')
    cmd = ['aiocoap-fileserver', COAPROOT]
    process = subprocess.Popen(cmd)
    return process


def setup_otaserver():
    logger.info('Setting up ota-server')
    cmd = ['python3', 'otaserver/main.py', '--coap-host=[fd00:dead:beef::1]']
    process = subprocess.Popen(cmd, cwd=os.path.expanduser(OTASERVER))
    return process


def setup_ethos(port, prefix, cwd_dir):
    cmd = ['sudo', './dist/tools/ethos/start_network.sh', port, 'suit0', prefix]
    process = subprocess.Popen(cmd,
                               cwd=os.path.expanduser(cwd_dir),
                               preexec_fn=os.setpgrp,
                               stdout=subprocess.DEVNULL)
    return process


def setup_network(prefix, cwd_dir):
    cmd = ['sudo', './dist/tools/ethos/setup_network.sh', 'suit1', prefix]
    process = subprocess.Popen(cmd,
                               cwd=os.path.expanduser(cwd_dir),
                               preexec_fn=os.setpgrp,
                               stdout=subprocess.DEVNULL)
    return process


def start_bin(board, start_app, start_bin):
    cmd = ['mkdir', '-p', '{}/{}/'.format(SETUP_DIR, board)]
    subprocess.call(cmd)
    files = glob.glob('{}/{}/bin/{}/*-slot0-extended.bin'.format(BASE_DIR,
                                                                 start_app,
                                                                 board))
    if files:
        cmd = ['cp', files[0], '{}/{}/'.format(SETUP_DIR, board)]
        subprocess.call(cmd)
    if start_bin:
        cmd = ['mv', os.path.basename(files[0]), '{}.bin'.format(start_bin)]
        subprocess.call(cmd, cwd='{}/{}/'.format(SETUP_DIR, board))


def update_pi(rpi):
    cmd = [
        'scp',
        '-r',
        '{}/'.format(SETUP_DIR),
        'pi@{}:/opt/src/riot-firmwares/firmwares'.format(rpi)
        ]
    subprocess.call(cmd, cwd=BASE_DIR)


def make_all(board, start_app, make_args):
    logger.info('Initial build of {}'.format(board))
    cmd = ['make', 'clean', 'all', 'BOARD={}'.format(board)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.join(BASE_DIR, start_app))


def make_flash(board, start_app, flash_cmd, make_args):
    logger.info('Flashing {}'.format(board))
    cmd = ['make', flash_cmd, 'BOARD={}'.format(board)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.join(BASE_DIR, start_app))


def make_reset(board, cwd_dir, port, make_args):
    logger.info('Reseting board {}'.format(board))
    cmd = ['make', 'reset', 'BOARD={}'.format(board), 'PORT={}'.format(port)]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.expanduser(cwd_dir))


def make_publish(board, server_url, cwd_dir, make_args, mode, manifest):
    logger.info('Publishing  %s Firmware to %s', cwd_dir, server_url)
    if mode is True:
        cmd = [
            'make',
            'suit/publish',
            'BOARD={}'.format(board),
            'APPLICATION={}'.format(manifest),
            'SUIT_OTA_SERVER_URL={}'.format(server_url),
        ]
    else:
        cmd = [
            'make',
            'suit/publish',
            'BOARD={}'.format(board),
            'SUIT_MANIFEST_SIGNED_LATEST={}'.format(manifest),
            'SUIT_COAP_SERVER={}'.format(server_url),
            'SUIT_COAP_FSROOT={}'.format(COAPROOT),
        ]
    cmd.extend(make_args)
    subprocess.call(cmd, cwd=os.path.join(BASE_DIR, cwd_dir))


PARSER = argparse.ArgumentParser(
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
PARSER.add_argument('--applications', default='apps/node_leds',
                    help='List of applications publish', type=list_from_string)
PARSER.add_argument('--board-node', default='nrf52840-mdk',
                    help='Board to test')
PARSER.add_argument('--board-ethos', default='iotlab-m3',
                    help='Board to test')
PARSER.add_argument('--ethos-br', default=False, action='store_true',
                    help='True if test is to be setup locally over ethos.')
PARSER.add_argument('--setup-nwk', default=False, action='store_true',
                    help='Setups uhcp, tap device and propagate prefix.')
PARSER.add_argument('--http', default=False, action='store_true',
                    help='Use http server')
PARSER.add_argument('--coapserver', default=False, action='store_true',
                    help='Start coapserver, Default=True')
PARSER.add_argument('--flash', default=False, action='store_true',
                    help='Flashes target node , Default False')
PARSER.add_argument('--keys', default=None,
                    help='Name for sec and pub keys')
PARSER.add_argument('--key-dir', default=None,
                    help='Directory to save keys to')
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

    coap_server = None
    ethos_br = None
    setup_nwk = None

    try:
        # Setup Ethos
        if args.ethos_br is True:
            make_reset(board_ethos, start_app, port_ethos, make_args)
            ethos_br = setup_ethos(port_ethos, prefix, riot_dir)
            time.sleep(1)
            logger.info("Ethos pid {} and group pid {}".format(ethos_br.pid,
                        os.getpgid(ethos_br.pid)))

        # Setup Nwk
        if args.setup_nwk is True:
            setup_nwk = setup_network(prefix, riot_dir)
            logger.info("setup_nwk.sh pid {} and group pid {}".format(setup_nwk.pid,
                        os.getpgid(setup_nwk.pid)))

        # Setup File Sever
        if args.coapserver is True:
            if args.http is False:
                coap_server = setup_aiocoap()
            else:
                coap_server = setup_otaserver()

        # Set keys and key-dir to use
        if args.keys is not None:
            os.environ['SUIT_KEY'] = args.keys 
        if args.key_dir is not None:
            os.environ['SUIT_KEY_DIR'] = args.key_dir 

        # Make start binaries
        if args.start_bin is True:
            make_all(board_node, start_app, make_args)
            start_bin(board_node, start_app, args.start_bin_nm)

        # Flashes new start binaries to the board
        if args.flash is True:
            flash_cmd = 'flash-only' if args.start_bin is True else 'flash'
            make_flash(board_node, start_app, flash_cmd, make_args)

        # Publish firmware(s)
        if args.publish is True:
            if len(manifests) == len(app_dirs):
                for i in range(0, len(app_dirs)):
                    make_publish(board_node, host, app_dirs[i], make_args, http,
                                 manifests[i])
            else:
                for i in range(0, len(app_dirs)):
                    make_publish(board_node, host, app_dirs[i], make_args, http,
                                 "latest-{}".format(i))

        # Copy firmware(s) to pi
        if args.rpi is not None:
            update_pi(args.rpi)

        # Run tests and keep running if coapserver or ethos were setup
        if args.ethos_br is True or args.coapserver is True or args.setup_nwk is True:
            while True:
                time.sleep(1)

    except SystemExit as e:
        sys.exit(e)

    finally:
        # If ethos cleanup with sudo
        if ethos_br is not None:
            try:
                gpid = os.getpgid(ethos_br.pid)
                logger.info("Killing group {}".format(gpid))
                subprocess.check_call(["sudo", "kill", '-{}'.format(gpid)])
            except:
                logger.info("Failed to stop process {}".format(ethos_br.pid))
            cmd = ['fuser', '-k', port_ethos]
            subprocess.call(cmd)
        # If setup_nwk cleanup with sudo
        if setup_nwk is not None:
            try:
                gpid = os.getpgid(setup_nwk.pid)
                logger.info("Killing group {}".format(gpid))
                subprocess.check_call(["sudo", "kill", '-{}'.format(gpid)])
            except:
                logger.info("Failed to stop process {}".format(setup_nwk.pid))
        # Kill server process
        if coap_server is not None:
            coap_server.kill()
