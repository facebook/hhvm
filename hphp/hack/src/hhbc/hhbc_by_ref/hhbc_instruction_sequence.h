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
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_ast.h"


namespace HPHP {
namespace hackc {
namespace hhbc {
namespace ast {

/// The various from_X functions below take some kind of AST
/// (expression, statement, etc.) and produce what is logically a
/// sequence of instructions. This could be represented by a list, but
/// we wish to avoid the quadratic complexity associated with repeated
/// appending. So, we build a tree of instructions which can be
/// flattened when complete.
struct InstrSeq {
  enum class Tag {
    List,
    Concat,
  };

  struct List_Body {
    BumpSliceMut<Instruct> _0;
  };

  struct Concat_Body {
    BumpSliceMut<InstrSeq> _0;
  };

  Tag tag;
  union {
    List_Body list;
    Concat_Body concat;
  };
};


extern "C" {

void no_call_compile_only_USED_TYPES_instruction_sequence(InstrSeq);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
