/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/

struct decls;

struct bump;

struct decl_parser_options;

struct bytes {
    uint8_t const* data;
    size_t len;
};

struct decl_result {
    size_t hash;
    bytes serialized;
    decls const* decls;
};
