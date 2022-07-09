// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#pragma once
#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs"

// This must be kept in sync with `enum ExternalDeclProviderResult` in
// 'hackc/decl_provider/external.rs' so they both are layout compatible.
struct DeclProviderResult {
  enum class Tag {
    Missing,
    Decls,
    Bytes,
  };
  struct DeclProviderDecls_Body {
    Decls const* _0;
  };
  struct DeclProviderBytes_Body {
    ::rust::Vec<uint8_t> const* _0;
  };
  Tag tag;
  union {
    DeclProviderDecls_Body decl_provider_decls_result;
    DeclProviderBytes_Body decl_provider_bytes_result;
  };

  // Construct a DeclProviderResult::Missing
  static DeclProviderResult missing() {
    return DeclProviderResult{DeclProviderResult::Tag::Missing, {}};
  }

  // Construct a DeclProviderResult::Decls from DeclResult
  static DeclProviderResult from_decls(const DeclResult& decl_result) {
    DeclProviderResult r;
    r.tag = DeclProviderResult::Tag::Decls;
    r.decl_provider_decls_result._0 = &(*decl_result.decls);
    return r;
  }

  // Construct a DeclProviderResult::Bytes from DeclResult
  static DeclProviderResult from_bytes(const DeclResult& decl_result) {
    DeclProviderResult r;
    r.tag = DeclProviderResult::Tag::Bytes;
    r.decl_provider_bytes_result._0 = &decl_result.serialized;
    return r;
  }
};
