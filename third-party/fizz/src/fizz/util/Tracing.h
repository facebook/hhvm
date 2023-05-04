/*
 *  Copyright (c) 2023-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fizz/util/KeyLogTypes.h>

namespace fizz {

extern "C" {
void fizz_probe_secret_available(
    long unsigned int secretSize,
    unsigned char* secretData,
    fizz::NSSLabel nssLabel,
    unsigned char* clientRandom);
}

} // namespace fizz
