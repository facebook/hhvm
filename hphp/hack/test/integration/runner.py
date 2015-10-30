from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import argparse
import hh_paths
import sys
import unittest

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('hh_server')
    parser.add_argument('hh_client')
    args = parser.parse_args()
    hh_paths.hh_server = args.hh_server
    hh_paths.hh_client = args.hh_client
    import test_save_mini
    import test_save_restore

    save_suite = unittest.defaultTestLoader.loadTestsFromTestCase(
            test_save_restore.TestSaveRestore)
    save_mini_suite = unittest.defaultTestLoader.loadTestsFromTestCase(
            test_save_mini.TestSaveMiniState)
    save_suite.addTests(save_mini_suite)

    result = unittest.TextTestRunner(verbosity=2).run(save_suite)
    if not result.wasSuccessful():
        sys.exit(1)
