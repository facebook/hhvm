// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_hhas_symbol_refs::HhasSymbolRefs;
use std::collections::BTreeSet;

#[derive(Debug, Default)]
pub struct SymbolRefsState<'arena> {
    pub symbol_refs: HhasSymbolRefs<'arena>,
}

impl<'arena> SymbolRefsState<'arena> {
    pub fn init(_alloc: &'arena bumpalo::Bump) -> Self {
        SymbolRefsState {
            symbol_refs: HhasSymbolRefs {
                includes: BTreeSet::new(),
                constants: BTreeSet::new(),
                functions: BTreeSet::new(),
                classes: BTreeSet::new(),
            },
        }
    }
}
