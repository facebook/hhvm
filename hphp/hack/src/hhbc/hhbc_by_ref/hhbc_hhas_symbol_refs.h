// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<073060ef3808b92165d5104152e24efd>>


#pragma once



#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_symbol_refs_state.h"


namespace HPHP {
namespace hackc {
namespace hhbc {
namespace ast {

/// Data structure for keeping track of symbols (and includes) we
/// encounter in the course of emitting bytecode for an AST. We split
/// them into these four categories for the sake of HHVM, which has
/// a dedicated lookup function corresponding to each.
struct HhasSymbolRefs {
  Slice<IncludePath> includes;
  Slice<Str> constants;
  Slice<Str> functions;
  Slice<Str> classes;
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhas_symbol_refs(HhasSymbolRefs);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
