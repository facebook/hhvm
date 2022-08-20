#!/usr/bin/env python
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import sys
import time


log_file_name = sys.argv[1]

args = sys.argv[2:]

with open(log_file_name, "a") as f:
    for arg in args:
        f.write("%s " % time.time())
        f.write(arg)
        f.write("\n")

print("WOOT from trig.sh")
