#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.


if [ "$GITHUB_ACTIONS" == "true" ]; then
    TARGETS_FILE=$(mktemp)
    ./buck2 targets //fizz/... | grep -F -v -f ./bad_targets | grep -v test>"$TARGETS_FILE"
    ./buck2 build @"$TARGETS_FILE"
else
    dotslash-oss "$BUCK2" build //... && dotslash-oss "$BUCK2" test //...
fi
