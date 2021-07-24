// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<45e1359d65c2f774e0643a5b176112bb>>


#pragma once



#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>


namespace HPHP {
namespace hackc {
namespace hhbc {
namespace ast {

using Id = size_t;

struct Label {
  enum class Tag {
    Regular,
    DefaultArg,
  };

  struct Regular_Body {
    Id _0;
  };

  struct DefaultArg_Body {
    Id _0;
  };

  Tag tag;
  union {
    Regular_Body regular;
    DefaultArg_Body default_arg;
  };
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhbc_label(Label);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
