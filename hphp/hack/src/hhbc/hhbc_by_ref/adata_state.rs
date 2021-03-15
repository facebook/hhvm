// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Debug, Default)]
pub struct AdataState<'arena> {
    pub array_identifier_counter: usize,
    pub array_identifier_map:
        std::collections::BTreeMap<hhbc_by_ref_runtime::TypedValue<'arena>, &'arena str>,
    pub adata: Vec<hhbc_by_ref_hhas_adata::HhasAdata<'arena>>,
}

impl<'arena> AdataState<'arena> {
    pub fn init(_alloc: &'arena bumpalo::Bump) -> Self {
        AdataState {
            array_identifier_counter: 0,
            array_identifier_map: std::collections::BTreeMap::new(),
            adata: vec![],
        }
    }
}
