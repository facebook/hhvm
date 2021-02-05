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

char const* extract_as_json_cpp_ffi(
    int32_t flags
  , char const* filename
  , char const* source_text
  , bool mangle_xhp );

void extract_as_json_free_string_cpp_ffi(char const*);
#  if defined(__cplusplus)
}
#  endif /*defined(__cplusplus)*/

#endif/*!defined(RUST_FACTS_FFI_H)*/
