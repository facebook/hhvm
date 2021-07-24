// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<6efc6363aacd77acc94e951305d3d5ae>>


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

struct F64 {
  uint8_t _0[8];
};

/// We introduce a type for Hack/PHP values, mimicking what happens at
/// runtime. Currently this is used for constant folding. By defining
/// a special type, we ensure independence from usage: for example, it
/// can be used for optimization on ASTs, or on bytecode, or (in
/// future) on a compiler intermediate language. HHVM takes a similar
/// approach: see runtime/base/typed-value.h
struct TypedValue {
  enum class Tag {
    /// Used for fields that are initialized in the 86pinit method
    Uninit,
    /// Hack/PHP integers are 64-bit
    Int,
    Bool,
    /// Both Hack/PHP and Caml floats are IEEE754 64-bit
    Float,
    String,
    LazyClass,
    Null,
    HhasAdata,
    Vec,
    Keyset,
    Dict,
  };

  struct Int_Body {
    int64_t _0;
  };

  struct Bool_Body {
    bool _0;
  };

  struct Float_Body {
    F64 _0;
  };

  struct String_Body {
    Str _0;
  };

  struct LazyClass_Body {
    Str _0;
  };

  struct HhasAdata_Body {
    Str _0;
  };

  struct Vec_Body {
    Slice<TypedValue> _0;
  };

  struct Keyset_Body {
    Slice<TypedValue> _0;
  };

  struct Dict_Body {
    Slice<Pair<TypedValue, TypedValue>> _0;
  };

  Tag tag;
  union {
    Int_Body int_;
    Bool_Body bool_;
    Float_Body float_;
    String_Body string;
    LazyClass_Body lazy_class;
    HhasAdata_Body hhas_adata;
    Vec_Body vec;
    Keyset_Body keyset;
    Dict_Body dict;
  };
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhbc_runtime(TypedValue);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
