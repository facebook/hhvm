# Copyright (c) 2014, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the "hack" directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

# coverage_compare.py -- compares coverages of two json coverage outputs from
#                        hack
# Run `hh_client --coverage . --json &> myfile.json` to create the json
# coverage

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import json
import sys


def gen_data(base, diff):
    for type_name, values in base['result'].items():
        print("== {0} ==".format(type_name))
        for degree in ['unchecked', 'partial', 'checked']:
            base_result = values[degree]
            diff_result = diff['result'][type_name][degree]
            percentage = 0 if base_result == 0 \
                            else 100 * (diff_result - base_result) / base_result
            print(
                "Change in {0:>10}: {1:+d} --> "
                "{2:+0.3f}% --> (from {3} to {4})"
                .format(
                    degree,
                    diff_result - base_result,
                    percentage,
                    base_result,
                    diff_result))
        print("")


def gen_json(filename):
    with open(filename, 'r') as f:
        raw_data = f.read()

    return json.loads(raw_data)


def main(args):
    if len(args) != 3:
        print("Usage: coverage_compare.py "
              "hh_coverage_base.json hh_coverage_diff")
        return 1

    base_filename = args[1]
    diff_filename = args[2]

    print("Comparing Base: {0} and Diff: {1}"
            .format(base_filename, diff_filename))

    gen_data(gen_json(base_filename), gen_json(diff_filename))

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
