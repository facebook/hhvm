/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/
#if !defined(COMPILE_FFI_H)
#  define COMPILE_FFI_H

#  include<stdint.h>
#  include<stddef.h>

#  include "hphp/hack/src/hhbc/compile_ffi_types_fwd.h"

enum hackc_compile_env_flags {
    IS_SYSTEMLIB=1 << 0
  , IS_EVALED=1 << 1
  , FOR_DEBUGGER_EVAL=1 << 2
  , DUMP_SYMBOL_REFS=1 << 3
  , DISABLE_TOPLEVEL_ENUMERATION=1 << 4
};

enum hackc_compile_hhbc_flags {
  LTR_ASSIGN=1 << 0
  , UVS=1 << 1
  , HACK_ARR_COMPAT_NOTICES=1 << 2
  , HACK_ARR_DV_ARRS=1 << 3
  , AUTHORITATIVE=1 << 4
  , JIT_ENABLE_RENAME_FUNCTION=1 << 5
  , LOG_EXTERN_COMPILER_PERF=1 << 6
  , ENABLE_INTRINSICS_EXTENSION=1 << 7
  , DISABLE_NONTOPLEVEL_DECLARATIONS=1 << 8
  , DISABLE_STATIC_CLOSURES=1 << 9
  , EMIT_CLS_METH_POINTERS=1 << 10
  , EMIT_METH_CALLER_FUNC_POINTERS=1 << 11
  , RX_IS_ENABLED=1 << 12
  , ARRAY_PROVENANCE=1 << 13
  // No longer using bit 14.
  , FOLD_LAZY_CLASS_KEYS=1 << 15
  , EMIT_INST_METH_POINTERS=1 << 16
};

enum hackc_compile_parser_flags {
   ABSTRACT_STATIC_PROPS=1 << 0
  , ALLOW_NEW_ATTRIBUTE_SYNTAX=1 << 1
  , ALLOW_UNSTABLE_FEATURES=1 << 2
  , CONST_DEFAULT_FUNC_ARGS=1 << 3
  , CONST_STATIC_PROPS=1 << 4
  , DISABLE_ARRAY=1 << 5
  , DISABLE_ARRAY_CAST=1 << 6
  , DISABLE_ARRAY_TYPEHINT=1 << 7
  , DISABLE_LVAL_AS_AN_EXPRESSION=1 << 8
  , DISABLE_UNSET_CLASS_CONST=1 << 9
  , DISALLOW_INST_METH=1 << 10
  , DISABLE_XHP_ELEMENT_MANGLING=1 << 11
  , DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS=1 << 12
  , DISALLOW_FUNC_PTRS_IN_CONSTANTS=1 << 13
  , DISALLOW_HASH_COMMENTS=1 << 14
  , ENABLE_ENUM_CLASSES=1 << 16
  , ENABLE_XHP_CLASS_MODIFIER=1 << 17
  , DISALLOW_DYNAMIC_METH_CALLER_ARGS=1 << 18
  , ENABLE_CLASS_LEVEL_WHERE_CLAUSES=1 << 19
  , ENABLE_READONLY_ENFORCEMENT=1 << 20
};

#  if defined(__cplusplus)
extern "C" {
#  endif /*defined(__cplusplus)*/
char const* hackc_compile_from_text_cpp_ffi(
       hackc_compile_native_environment const* env
     , char const* source_text
     , hackc_compile_output_config const* config
     , hackc_error_buf_t* error_buf );

void hackc_compile_from_text_free_string_cpp_ffi(char const*);
#  if defined(__cplusplus)
}

#  include <memory>

namespace HPHP {

using hackc_compile_from_text_ptr =
  std::unique_ptr<char const, void(*)(char const*)>;

inline hackc_compile_from_text_ptr
  hackc_compile_from_text(
      hackc_compile_native_environment const* env
    , char const* source_text
    , hackc_compile_output_config const* config
    , hackc_error_buf_t* error_buf
  ) {
  return hackc_compile_from_text_ptr {
      hackc_compile_from_text_cpp_ffi(env, source_text, config, error_buf)
    , hackc_compile_from_text_free_string_cpp_ffi
  };
}

}//namespace HPHP
#  endif /*defined(__cplusplus)*/

#endif/*!defined(COMPILE_FFI_H)*/
