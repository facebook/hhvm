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
            auto_namespace_map: opts.po_auto_namespace_map,
            disable_xhp_element_mangling: opts.po_disable_xhp_element_mangling,
            interpret_soft_types_as_like_types: opts.po_interpret_soft_types_as_like_types,
            everything_sdt: opts.tco_everything_sdt,
        }
    }

    pub fn from_oxidized_parser_options<'a>(
        arena: &'a bumpalo::Bump,
        opts: &'a oxidized::parser_options::ParserOptions,
    ) -> DeclParserOptions<'a> {
        let mut ns_map =
            bumpalo::collections::Vec::with_capacity_in(opts.po_auto_namespace_map.len(), arena);
        ns_map.extend(
            opts.po_auto_namespace_map
                .iter()
                .map(|(a, b)| (a.as_str(), b.as_str())),
        );
        DeclParserOptions {
            auto_namespace_map: ns_map.into_bump_slice(),
            disable_xhp_element_mangling: opts.po_disable_xhp_element_mangling,
            interpret_soft_types_as_like_types: opts.po_interpret_soft_types_as_like_types,
            everything_sdt: opts.tco_everything_sdt,
        }
    }
}

impl<'a> From<&ParserOptions<'a>> for DeclParserOptions<'a> {
    fn from(opts: &ParserOptions<'a>) -> Self {
        DeclParserOptions::from_parser_options(opts)
    }
}
