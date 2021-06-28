/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/
#pragma once

#include "hphp/hack/src/hhbc/compile_ffi_types_fwd.h"

#include <cstdint>
#include <cstddef>
#include <memory>

namespace HPHP { namespace hackc { namespace compile {

enum env_flags {
    IS_SYSTEMLIB=1 << 0
  , IS_EVALED=1 << 1
  , FOR_DEBUGGER_EVAL=1 << 2
  , DUMP_SYMBOL_REFS=1 << 3
  , DISABLE_TOPLEVEL_ENUMERATION=1 << 4
  , ENABLE_DECL=1 << 5
};

extern "C" {
char const* hackc_compile_from_text_cpp_ffi(
       native_environment const* env
     , char const* source_text
     , output_config const* config
     , error_buf_t* error_buf );

void hackc_compile_from_text_free_string_cpp_ffi(char const*);
}//extern"C"

using hackc_compile_from_text_ptr =
  std::unique_ptr<char const, void(*)(char const*)>;

inline hackc_compile_from_text_ptr
  hackc_compile_from_text(
      native_environment const* env
    , char const* source_text
    , output_config const* config
    , error_buf_t* error_buf
  ) {
  return hackc_compile_from_text_ptr {
      hackc_compile_from_text_cpp_ffi(env, source_text, config, error_buf)
    , hackc_compile_from_text_free_string_cpp_ffi
  };
}

}}} //namespace HPHP::hackc::compile
