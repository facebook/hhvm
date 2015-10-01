from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import argparse
import sys
import unittest

import test_save_restore

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('hh_server')
    parser.add_argument('hh_client')
    args = parser.parse_args()
    test_save_restore.hh_server = args.hh_server
    test_save_restore.hh_client = args.hh_client

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(
            test_save_restore.TestSaveRestore)

    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if not result.wasSuccessful():
        sys.exit(1)
