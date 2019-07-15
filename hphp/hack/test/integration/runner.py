from __future__ import absolute_import, division, print_function, unicode_literals

import argparse
import sys
import unittest

import hh_paths


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("hh_server")
    parser.add_argument("hh_client")
    parser.add_argument("hh_single_type_check")
    args = parser.parse_args()
    hh_paths.hh_server = args.hh_server
    hh_paths.hh_client = args.hh_client
    hh_paths.hh_single_type_check = args.hh_single_type_check
    import test_save_mini
    import test_fresh_init

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(
        test_save_mini.TestSaveMiniState
    )
    test_fresh_suite = unittest.defaultTestLoader.loadTestsFromTestCase(
        test_fresh_init.TestFreshInit
    )
    suite.addTests(test_fresh_suite)

    result = unittest.TextTestRunner(verbosity=2).run(suite)
    if not result.wasSuccessful():
        sys.exit(1)
