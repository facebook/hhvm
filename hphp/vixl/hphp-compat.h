// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#pragma once

// Bridge header: pulls new VIXL aarch64 types into the flat vixl:: namespace
// for backward compatibility with HHVM consumers that use "using namespace vixl".

#include "hphp/vixl/aarch64/macro-assembler-aarch64.h"
#include "hphp/vixl/aarch64/disasm-aarch64.h"
#include "hphp/vixl/aarch64/instructions-aarch64.h"

namespace vixl {
using namespace aarch64;
}  // namespace vixl
