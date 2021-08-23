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

#ifndef incl_HPHP_VM_BYTECODE_INL_H_
#error "bytecode-inl.h should only be included by bytecode.h"
#endif

namespace HPHP {

// wrapper for local variable ILA operand
struct local_var {
  tv_lval lval;
  int32_t index;
};

// wrapper for named local variable NLA operand
struct named_local_var {
  LocalName name;
  tv_lval lval;
};

// wrapper to handle unaligned access to variadic immediates
template<class T> struct imm_array {
  uint32_t const size;
  PC const ptr;

  explicit imm_array(uint32_t size, PC pc)
    : size{size}
    , ptr{pc}
  {}

  T operator[](uint32_t i) const {
    T e;
    memcpy(&e, ptr + i * sizeof(T), sizeof(T));
    return e;
  }
};

} // namespace HPHP
