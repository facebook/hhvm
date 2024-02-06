/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once
#include <cstdint>

namespace HPHP {

// These values are exposed to the user in
// hphp/runtime/ext/reflection/ext_reflection-TypeInfo.php
enum class TypeStructureKind : uint8_t {
  T_void = 0,
  T_int = 1,
  T_bool = 2,
  T_float = 3,
  T_string = 4,
  T_resource = 5,
  T_num = 6,
  T_arraykey = 7,
  T_noreturn = 8,
  T_mixed = 9,
  T_tuple = 10,
  T_fun = 11,
  T_typevar = 13, // corresponds to user OF_GENERIC
  T_shape = 14,

  // These values are only used after resolution in ext_reflection.cpp
  T_class = 15,
  T_interface = 16,
  T_trait = 17,
  T_enum = 18,

  // Hack array types
  T_dict = 19,
  T_vec = 20,
  T_keyset = 21,
  T_vec_or_dict = 22,

  T_nonnull = 23,

  T_darray = 24,
  T_varray = 25,
  T_varray_or_darray = 26,
  T_any_array = 27,

  T_null = 28,
  T_nothing = 29,
  T_dynamic = 30,
  T_union = 31,
  T_recursiveUnion = 32,
  // Make sure to update kMaxResolvedKind below if you add a new kind here

  // The following kinds needs class/alias resolution, and
  // are generally not exposed to the users.
  // Unfortunately this is a bit leaky. A few of these are needed by tooling.
  T_unresolved = 101,
  T_typeaccess = 102,
  T_xhp = 103,
  T_reifiedtype = 104,
};

struct Variant typeStructureKindToVariant(TypeStructureKind kind);

namespace TypeStructure {
constexpr uint8_t kMaxResolvedKind = 32;
using Kind = HPHP::TypeStructureKind;
}

}
