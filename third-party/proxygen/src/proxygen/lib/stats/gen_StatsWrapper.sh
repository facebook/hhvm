#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

set -e

if [ "x$1" != "x" ];then
  OUTPUT_DIR="$1"
fi

statsWrapper=$(cat <<EOF
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/stats/BaseStats.h>

#pragma once

namespace proxygen {

using StatsWrapper = proxygen::BaseStats;

} // proxygen
EOF
)

touch "${OUTPUT_DIR?}/proxygen/lib/stats/StatsWrapper.h"
echo "$statsWrapper" >  "${OUTPUT_DIR?}/proxygen/lib/stats/StatsWrapper.h"

echo "Generated StatsWrapper.h"
