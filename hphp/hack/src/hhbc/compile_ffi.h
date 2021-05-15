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
