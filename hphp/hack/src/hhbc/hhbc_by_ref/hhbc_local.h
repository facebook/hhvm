// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>


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

using Id = size_t;

/// Type of locals as they appear in instructions. Named variables are
/// those appearing in the .declvars declaration. These can also be
/// referenced by number (0 to n-1), but we use Unnamed only for
/// variables n and above not appearing in .declvars
struct Local {
  enum class Tag {
    Unnamed,
    /// Named local, necessarily starting with `$`
    Named,
  };

  struct Unnamed_Body {
    Id _0;
  };

  struct Named_Body {
    Str _0;
  };

  Tag tag;
  union {
    Unnamed_Body unnamed;
    Named_Body named;
  };
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhbc_local(Local);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
