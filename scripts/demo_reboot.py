#!/usr/bin/env python3

"""Demo Reset Request Handler."""

import os
import subprocess
import sys
import logging
import json
import yaml

import tornado
from tornado.options import define, options
from tornado import web


logger = logging.getLogger("demoreset")

BASE_DIR = os.path.dirname(sys.argv[0])
MAKE_DIR = os.path.join(BASE_DIR, 'reset')
CONFIG_DIR = os.path.join(BASE_DIR, 'config')

DEFAULT_CONFIG =  os.path.join(CONFIG_DIR, 'demo_config.yml')

class BaseHandler(tornado.web.RequestHandler):

    def set_default_headers(self, *args, **kwargs):
        origin = self.request.headers.get("Origin", None)
        if origin in config['cors']['allowed']:
            logger.debug("Allowed Origin %s" % origin)
            self.set_header("Access-Control-Allow-Origin", origin)
        self.set_header("Access-Control-Allow-Headers", "content-type, x-requested-with")
        self.set_header('Access-Control-Allow-Methods', 'POST')


class DemoResetHandler(BaseHandler):
    """Reset Specified Device."""

    # TODO: this should be in a separate module
    @staticmethod
    def make_flash_only(board, debug_id, binfile):
        logger.info('Initial Flash of {}'.format(board))
        cmd = [
            'make',
            'flash-only',
            'FLASHFILE={}'.format(binfile),
            'BOARD={}'.format(board),
            'DEBUG_ADAPTER_ID={}'.format(debug_id),
            ]
        subprocess.call(cmd, cwd=os.path.expanduser(MAKE_DIR))

    def match_uid(self, uid):
        devices = config["devices"]
        for device in devices:
            if device['uid'] == uid:
                logger.debug("Found matching device {}".format(device))
                return device
        else:
            logger.debug("No matching device found for id {}".format(uid))
            return None

    async def options(self):
        self.set_status(204)
        self.finish()

    async def post(self):
        """Handle request for removing an existing version."""
        request = json.loads(self.request.body.decode())
        logger.debug("Resetting Device with uid  %s", request['uid'])
        device = self.match_uid(request['uid'])
        if device is not None:
            self.make_flash_only(device['board'], device['debug_id'], device['demo']['binfile'])
        else:
            self.set_status(404)


class DemoResetApplication(web.Application):
    """Tornado based web application to listen for incoming reset requests."""

    def __init__(self):
        if options.debug:
            logger.setLevel(logging.DEBUG)

        handlers = [
            (r"/reset", DemoResetHandler),
        ]

        # TODO: security
        settings = {'debug': True,
                    }

        super().__init__(handlers, **settings)
        logger.info('Application started, listening on port {}'
                    .format(options.http_port))


def parse_command_line():
    """Parse command line arguments."""
    define("http_host", default="localhost", help="Web application HTTP host.")
    define("http_port", default=8080, help="Web application HTTP port.")
    define("debug", default=logging.INFO, help="Enable debug mode.")
    define("config", default=DEFAULT_CONFIG, help="Config File.")
    options.parse_command_line()


def read_config(filename):
    global config
    with open(filename, 'r') as ymlfile:
        config = yaml.load(ymlfile, Loader=yaml.Loader)


def run(arguments=[]):
    """Start a reset listener instance."""

    parse_command_line()
    read_config(options.config)

    logger.setLevel(options.debug)

    try:
        app = DemoResetApplication()
        app.listen(options.http_port)
        tornado.ioloop.IOLoop.instance().start()
    except KeyboardInterrupt:
        logger.debug("Stopping application")
        tornado.ioloop.IOLoop.instance().stop()

if __name__ == '__main__':
    run()