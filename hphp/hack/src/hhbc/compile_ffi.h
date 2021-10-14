/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/
#pragma once

#include "hphp/hack/src/hhbc/compile_ffi_types_fwd.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc-ast.h"

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
char const* hackc_hhas_to_string_cpp_ffi(
  native_environment const* env,
  HPHP::hackc::hhbc::HhasProgram const* prog,
  error_buf_t* error_buf
);
void hackc_hhas_to_string_free_string_cpp_ffi(char const*);
} // extern "C"

using hackc_hhas_to_string_ptr =
  std::unique_ptr<char const, void(*)(char const*)>;

inline hackc_hhas_to_string_ptr hackc_hhas_to_string(
  native_environment const* env,
  HPHP::hackc::hhbc::HhasProgram const* prog,
  error_buf_t* error_buf
) {
  return hackc_hhas_to_string_ptr {
    hackc_hhas_to_string_cpp_ffi(env, prog, error_buf),
    hackc_hhas_to_string_free_string_cpp_ffi
  };
}
}}} //namespace HPHP::hackc::compile
