// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#pragma once
#include <string>
#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"

namespace HPHP {
namespace hackc {

// DeclProviders return pointers to data that must outlive the provider.
// This must be kept in sync with `enum ExternalDeclProviderResult` in
// 'hackc/decl_provider/external.rs' so they both are layout compatible.
struct ExternalDeclProviderResult {
  enum class Tag {
    Missing,
    Decls,
    RustVec,
    CppString,
  };
  struct Decls_Body {
    DeclsHolder const* _0;
  };
  struct RustVec_Body {
    ::rust::Vec<uint8_t> const* _0;
  };
  struct CppString_Body {
    std::string const* _0;
  };
  Tag tag;
  union {
    Decls_Body decls;
    RustVec_Body rust_vec;
    CppString_Body cpp_string;
  };

  // Construct Missing
  static ExternalDeclProviderResult missing() {
    return ExternalDeclProviderResult{Tag::Missing, {}};
  }

  // Construct Decls from DeclsAndBlob decls
  static ExternalDeclProviderResult from_decls(
      const DeclsAndBlob& decl_result) {
    ExternalDeclProviderResult r;
    r.tag = Tag::Decls;
    r.decls._0 = &(*decl_result.decls);
    return r;
  }

  // Construct Bytes from DeclsAndBlob serialized bytes in a rust::Vec
  static ExternalDeclProviderResult from_bytes(
      const DeclsAndBlob& decl_result) {
    ExternalDeclProviderResult r;
    r.tag = Tag::RustVec;
    r.rust_vec._0 = &decl_result.serialized;
    return r;
  }

  // Construct Bytes from serialized bytes in a std::string
  static ExternalDeclProviderResult from_string(const std::string& data) {
    ExternalDeclProviderResult r;
    r.tag = Tag::CppString;
    r.cpp_string._0 = &data;
    return r;
  }
};

// Virtual base class for C++ DeclProviders. This is the C++ counterpart
// to trait DeclProvider in decl_provider.rs.
struct DeclProvider {
  virtual ~DeclProvider() = default;

  // Look up a type (class, type alias, etc) by name, and return all of the
  // decls in the file that defines it. The resulting pointers must have at
  // least this provider's lifetime.
  virtual ExternalDeclProviderResult getType(
      std::string_view symbol,
      uint64_t depth) noexcept = 0;

  // Look up a top level function by name, and return all of the decls in the
  // file that defines it. The resulting pointers must have at least this
  // provider's lifetime.
  virtual ExternalDeclProviderResult getFunc(
      std::string_view symbol) noexcept = 0;

  // Look up a top level constant by name, and return all of the decls in the
  // file that defines it. The resulting pointers must have at least this
  // provider's lifetime.
  virtual ExternalDeclProviderResult getConst(
      std::string_view symbol) noexcept = 0;

  // Look up a module by name, and return all of the decls in the file that
  // defines it. The resulting pointers must have at least this provider's
  // lifetime.
  virtual ExternalDeclProviderResult getModule(
      std::string_view symbol) noexcept = 0;
};

} // namespace hackc
} // namespace HPHP
