from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import argparse
import sys
import unittest

from test_save_restore import TestSaveRestore

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('hh_server')
    parser.add_argument('hh_client')
    args = parser.parse_args()
    TestSaveRestore.init_class(args.hh_server, args.hh_client)

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(TestSaveRestore)

    result = unittest.TextTestRunner().run(suite)
    if not result.wasSuccessful():
        sys.exit(1)
