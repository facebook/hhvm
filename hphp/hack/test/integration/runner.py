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
    import test_fresh_init

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(
            test_save_mini.TestSaveMiniState)
    test_fresh_suite = unittest.defaultTestLoader.loadTestsFromTestCase(
            test_fresh_init.TestFreshInit)
    suite.addTests(test_fresh_suite)

    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if not result.wasSuccessful():
        sys.exit(1)
