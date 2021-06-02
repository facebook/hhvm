/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/
#if !defined(DECL_FFI_H)
#  define DECL_FFI_H

#  include<stdint.h>
#  include<stddef.h>

#  include "hphp/hack/src/decl/cpp_ffi/decl_ffi_types_fwd.h"

#  if defined(__cplusplus)
extern "C" {
#  endif /*defined(__cplusplus)*/

bump const* create_arena();

void free_arena(bump const* bump);

decl_parser_options const* create_direct_decl_parse_options(
    bool hack_arr_dv_arrs,
    // TODO: cxx doesn't support tuple,
    //auto_namespace_map: &'a [(&'a str, &'a str)],
    bool disable_xhp_element_mangling,
    bool interpret_soft_types_as_like_types
);

decl_result direct_decl_parse(
    decl_parser_options const* opts,
    char const* filename,
    char const* text,
    bump const* arena
);

void print_decls(decls const* decls);

bool verify_deserialization(
    bytes const* serialized,
    decls const* expected
);

#  if defined(__cplusplus)
}
#  endif /*defined(__cplusplus)*/
#endif/*!defined(DECL_FFI_H)*/
