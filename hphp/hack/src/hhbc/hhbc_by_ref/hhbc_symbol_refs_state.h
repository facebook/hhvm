// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<fa4ddec6504390cc820950a1f0f1b380>>


#pragma once



#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "hphp/hack/src/utils/ffi/ffi.h"


namespace HPHP {
namespace hackc {
namespace hhbc {
namespace ast {

struct IncludePath {
  enum class Tag {
    Absolute,
    SearchPathRelative,
    IncludeRootRelative,
    DocRootRelative,
  };

  struct Absolute_Body {
    Str _0;
  };

  struct SearchPathRelative_Body {
    Str _0;
  };

  struct IncludeRootRelative_Body {
    Str _0;
    Str _1;
  };

  struct DocRootRelative_Body {
    Str _0;
  };

  Tag tag;
  union {
    Absolute_Body absolute;
    SearchPathRelative_Body search_path_relative;
    IncludeRootRelative_Body include_root_relative;
    DocRootRelative_Body doc_root_relative;
  };
};


extern "C" {

void no_call_compile_only_USED_TYPES_symbol_refs_state(IncludePath);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
