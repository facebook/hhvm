// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<67847ba71185abff24f58bbf0a4b0435>>


#pragma once



#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "hphp/hack/src/utils/ffi/ffi.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_runtime.h"


namespace HPHP {
namespace hackc {
namespace hhbc {
namespace ast {

/// Attributes with a name from [naming_special_names::user_attributes] and
/// a series of arguments.  Emitter code can match on an attribute as follows:
/// ```
///   use naming_special_names::user_attributes as ua;
///   fn is_memoized(attr: &HhasAttribute) -> bool {
///      attr.is(ua::memoized)
///   }
///   fn has_dynamically_callable(attrs: &Vec<HhasAttribute>) {
///       attrs.iter().any(|a| a.name == ua::DYNAMICALLY_CALLABLE)
///   }
/// ```
struct HhasAttribute {
  Str name;
  Slice<TypedValue> arguments;
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhas_attribute(HhasAttribute);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
