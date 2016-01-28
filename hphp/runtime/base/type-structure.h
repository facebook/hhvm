/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TYPE_STRUCTURE_H
#define incl_HPHP_TYPE_STRUCTURE_H

#include <cstdint>
#include "hphp/runtime/vm/class.h"

namespace HPHP {

struct String;
struct Array;

/* Utility for representing full type information in the runtime. */
namespace TypeStructure {

// These values are exposed to the user in
// hphp/runtime/ext/reflection/ext_reflection-TypeInfo.php
enum class Kind : uint8_t {
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
  T_array = 12,
  T_typevar = 13, // corresponds to user OF_GENERIC
  T_shape = 14,

  // These values are only used after resolution in ext_reflection.cpp
  T_class = 15,
  T_interface = 16,
  T_trait = 17,
  T_enum = 18,

  /* The following kinds needs class/alias resolution, and
   * are not exposed to the users. */
  T_unresolved = 101,
  T_typeaccess = 102,
  T_xhp = 103,
};

bool KindOfClass(Kind kind);

String toString(const Array& arr);

Array resolve(const Class::Const& typeCns,
              const Class* typeCnsCls,
              bool& persistent);

Array resolve(const String& aliasName,
              const Array& arr,
              bool& persistent,
              const Array& generics = Array());

}

}

#endif
