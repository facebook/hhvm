/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/
#pragma once

#include "hphp/hack/src/decl/cpp_ffi/decl_ffi_types_fwd.h"

extern "C" {

HPHP::hackc::decl::bump_allocator const* hackc_create_arena();

void hackc_free_arena(HPHP::hackc::decl::bump_allocator const* bump);

HPHP::hackc::decl::decl_parser_options const* hackc_create_direct_decl_parse_options(
    // TODO: cxx doesn't support tuple,
    //auto_namespace_map: &'a [(&'a str, &'a str)],
    bool disable_xhp_element_mangling,
    bool interpret_soft_types_as_like_types
);

HPHP::hackc::decl::decl_result hackc_direct_decl_parse(
    HPHP::hackc::decl::decl_parser_options const* opts,
    char const* filename,
    char const* text,
    HPHP::hackc::decl::bump_allocator const* arena
);

void hackc_print_decls(HPHP::hackc::decl::decls const* decls);

bool hackc_verify_deserialization(
    HPHP::hackc::decl::bytes const* serialized,
    HPHP::hackc::decl::decls const* expected
);

}
