/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/

#pragma once

#include <cstdint>
#include <cstddef>

struct decls;

struct bump;

struct decl_parser_options;

struct bytes {
    std::uint8_t const* data;
    std::size_t len;
    std::size_t cap;
};

struct decl_result {
    std::size_t hash;
    bytes serialized;
    decls const* decl_list;
};
