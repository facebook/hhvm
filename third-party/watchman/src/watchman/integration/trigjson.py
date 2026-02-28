#!/usr/bin/env python
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import sys


log_file_name = sys.argv[1]

# Copy json from stdin to the log file
with open(log_file_name, "a") as f:
    print("trigjson.py: Copying STDIN to %s" % log_file_name)
    json_in = sys.stdin.read()
    print("stdin: %s" % json_in)
    f.write(json_in)
