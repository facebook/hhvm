// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_parser_options::DeclParserOptions;
use crate::parser_options::ParserOptions;

impl DeclParserOptions<'_> {
    pub const DEFAULT: &'static DeclParserOptions<'static> =
        &DeclParserOptions::from_parser_options(ParserOptions::DEFAULT);
}

impl Default for &DeclParserOptions<'_> {
    fn default() -> Self {
        DeclParserOptions::DEFAULT
    }
}

impl DeclParserOptions<'_> {
    pub const fn from_parser_options<'a>(opts: &ParserOptions<'a>) -> DeclParserOptions<'a> {
        DeclParserOptions {
            hack_arr_dv_arrs: opts.po_hack_arr_dv_arrs,
            auto_namespace_map: opts.po_auto_namespace_map,
            disable_xhp_element_mangling: opts.po_disable_xhp_element_mangling,
            interpret_soft_types_as_like_types: opts.po_interpret_soft_types_as_like_types,
        }
    }
}

impl<'a> From<&ParserOptions<'a>> for DeclParserOptions<'a> {
    fn from(opts: &ParserOptions<'a>) -> Self {
        DeclParserOptions::from_parser_options(opts)
    }
}
