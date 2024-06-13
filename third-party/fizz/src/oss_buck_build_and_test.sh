#!/bin/bash
# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.


if [ "$GITHUB_ACTIONS" == "true" ]; then
    TARGETS_FILE=$(mktemp)
    ./buck2 targets //fizz/... | grep -F -v -f ./bad_targets | grep -v test>"$TARGETS_FILE"
    ./buck2 build @"$TARGETS_FILE"
else
    dotslash-oss "$BUCK2" build //... && dotslash-oss "$BUCK2" test //...
fi
