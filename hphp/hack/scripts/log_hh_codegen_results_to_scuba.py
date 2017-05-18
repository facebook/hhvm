from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from dsi.logger.py.LoggerConfigHandler import LoggerConfigHandler
from everstore import Everstore
from ServiceRouter import ServiceRouter
import logging
import sys
import time
import traceback

EVERSTORE_HACK_CRASH_LOGS = 10124


def upload_to_everstore(fileName):
    try:
        with open(fileName, 'rb') as inputFile:
            serviceRouter = ServiceRouter()
            client = serviceRouter.getClient2(Everstore.Client, 'dfsrouter.common')
            handle = client.write(
                blob=inputFile.read(),
                fbtype=EVERSTORE_HACK_CRASH_LOGS,
                extension='txt',
            )
        return handle
    except Exception:
        logging.error('Exception while uploading to everstore: {}'.format(
            traceback.format_exc()))
        return 'NoHandle'


def main(args):
    data = {
        'time': int(time.time()),
        'hhvm_num_hhas_lines': int(args[1]),
        'hh_num_hhas_lines': int(args[2]),
        'common_num_hhas_lines': int(args[3]),
        'dir_of_php_files': args[4],
        'diff_everstore_handle': upload_to_everstore(args[5]),
    }

    HandlerInstance = LoggerConfigHandler('MeasureHHCodeGenProgressLoggerConfig')
    HandlerInstance.log(data)


if __name__ == "__main__":
    main(sys.argv)
