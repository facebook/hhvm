// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#pragma once
#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"

// This must be kept in sync with `enum ExternalDeclProviderResult` in
// 'hackc/decl_provider/external.rs' so they both are layout compatible.
struct ExternalDeclProviderResult {
  enum class Tag {
    Missing,
    Decls,
    Bytes,
  };
  struct DeclProviderDecls_Body {
    DeclsHolder const* _0;
  };
  struct DeclProviderBytes_Body {
    ::rust::Vec<uint8_t> const* _0;
  };
  Tag tag;
  union {
    DeclProviderDecls_Body decls;
    DeclProviderBytes_Body bytes;
  };

  // Construct a ExternalDeclProviderResult::Missing
  static ExternalDeclProviderResult missing() {
    return ExternalDeclProviderResult{
        ExternalDeclProviderResult::Tag::Missing, {}};
  }

  // Construct a ExternalDeclProviderResult::Decls from DeclResult
  static ExternalDeclProviderResult from_decls(const DeclResult& decl_result) {
    ExternalDeclProviderResult r;
    r.tag = ExternalDeclProviderResult::Tag::Decls;
    r.decls._0 = &(*decl_result.decls);
    return r;
  }

  // Construct a ExternalDeclProviderResult::Bytes from DeclResult
  static ExternalDeclProviderResult from_bytes(const DeclResult& decl_result) {
    ExternalDeclProviderResult r;
    r.tag = ExternalDeclProviderResult::Tag::Bytes;
    r.bytes._0 = &decl_result.serialized;
    return r;
  }
};

// Virtual base class for C++ DeclProviders. This is the C++ counterpart
// to trait DeclProvider in decl_provider.rs.
struct DeclProvider {
  virtual ~DeclProvider() = default;

  // Look up a type (class, type alias, etc) by name, and return all of the
  // decls in the file that defines it.
  virtual ExternalDeclProviderResult getType(
      std::string_view symbol,
      uint64_t depth) noexcept = 0;

  // Look up a top level function by name, and return all of the decls in the
  // file that defines it.
  virtual ExternalDeclProviderResult getFunc(
      std::string_view symbol) noexcept = 0;

  // Look up a top level constant by name, and return all of the decls in the
  // file that defines it.
  virtual ExternalDeclProviderResult getConst(
      std::string_view symbol) noexcept = 0;

  // Look up a module by name, and return all of the decls in the file that
  // defines it.
  virtual ExternalDeclProviderResult getModule(
      std::string_view symbol) noexcept = 0;
};
