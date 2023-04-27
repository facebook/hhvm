#!/usr/bin/env python
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import os


os.environ["PWD"] = os.getcwd()

for k, v in os.environ.items():
    print("%s=%s" % (k, v))
