from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from dsi.logger.py.LoggerConfigHandler import LoggerConfigHandler
import sys
import time


def main(args):
    data = {
        'time': int(time.time()),
        'hhvm_num_hhas_lines': int(args[1]),
        'hh_num_hhas_lines': int(args[2]),
        'common_num_hhas_lines': int(args[3]),
        'dir_of_php_files': args[4],
    }

    HandlerInstance = LoggerConfigHandler('MeasureHHCodeGenProgressLoggerConfig')
    HandlerInstance.log(data)


if __name__ == "__main__":
    main(sys.argv)
