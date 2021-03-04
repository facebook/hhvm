// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhas_symbol_refs_rust::HhasSymbolRefs;
use std::collections::BTreeSet;

#[derive(Debug, Default)]
pub struct SymbolRefsState {
    pub symbol_refs: HhasSymbolRefs,
}

impl SymbolRefsState {
    pub fn init() -> Self {
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
