// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#include "hphp/hack/src/hackc/ffi_bridge/decl_provider.h"

static bool recurse;

extern "C" {
// Called from hackc/decl_provider/external.rs
__attribute__((noinline)) ExternalDeclProviderResult provide_type_or_alias(
    const void* provider,
    const char* symbol,
    size_t symbol_len,
    uint64_t depth) noexcept {
  if (recurse)
    provide_type_or_alias(provider, symbol, symbol_len, depth);
  return ((DeclProvider*)provider)
      ->getType(std::string_view(symbol, symbol_len), depth);
}
}
