// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#include "hphp/hack/src/hackc/ffi_bridge/decl_provider.h"

using namespace HPHP::hackc;

// These stubs perform C++ virtual method dispatch on behalf of
// hackc/decl_provider/external.rs
extern "C" {

ExternalDeclProviderResult provide_type(
    const void* provider,
    const char* symbol,
    size_t symbol_len,
    uint64_t depth) noexcept {
  return ((DeclProvider*)provider)
      ->getType(std::string_view(symbol, symbol_len), depth);
}

ExternalDeclProviderResult provide_func(
    const void* provider,
    const char* symbol,
    size_t symbol_len) noexcept {
  return ((DeclProvider*)provider)
      ->getFunc(std::string_view(symbol, symbol_len));
}

ExternalDeclProviderResult provide_const(
    const void* provider,
    const char* symbol,
    size_t symbol_len) noexcept {
  return ((DeclProvider*)provider)
      ->getConst(std::string_view(symbol, symbol_len));
}

ExternalDeclProviderResult provide_module(
    const void* provider,
    const char* symbol,
    size_t symbol_len) noexcept {
  return ((DeclProvider*)provider)
      ->getModule(std::string_view(symbol, symbol_len));
}
}
