/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/
#if !defined(RUST_FACTS_FFI_H)
#  define RUST_FACTS_FFI_H

#  include<stdint.h>
#  include<stddef.h>

#  if defined(__cplusplus)
extern "C" {
#  endif /*defined(__cplusplus)*/

char const* hackc_extract_as_json_cpp_ffi(
    int32_t flags
  , char const* filename
  , char const* source_text
  , bool mangle_xhp );

void hackc_extract_as_json_free_string_cpp_ffi(char const*);
#  if defined(__cplusplus)
}

#  include <memory>

namespace HPHP {
using hackc_extract_as_json_ptr =
  std::unique_ptr<char const  , void(*)(char const*)>;

inline hackc_extract_as_json_ptr
  hackc_extract_as_json(
      int32_t flags
    , char const* filename
    , char const* source_text
    , bool mangle_xhp
  ) {
  return hackc_extract_as_json_ptr {
      hackc_extract_as_json_cpp_ffi(flags, filename, source_text, mangle_xhp)
    , hackc_extract_as_json_free_string_cpp_ffi
  };
}

}//namespace HPHP
#  endif /*defined(__cplusplus)*/

#endif/*!defined(RUST_FACTS_FFI_H)*/
