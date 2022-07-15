// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#include "hphp/hack/src/hackc/ffi_bridge/decl_provider.h"

extern "C" {
// Called from hackc/decl_provider/external.rs
ExternalDeclProviderResult provide_type_or_alias(
    const void* provider,
    const char* symbol,
    size_t symbol_len,
    uint64_t depth) noexcept {
  return ((DeclProvider*)provider)
      ->getType(std::string_view(symbol, symbol_len), depth);
}
}
