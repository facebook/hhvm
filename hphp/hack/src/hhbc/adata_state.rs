// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Debug, Default)]
pub struct AdataState {
    pub array_identifier_counter: usize,
    pub array_identifier_map: std::collections::BTreeMap<runtime::TypedValue, String>,
    pub adata: Vec<hhas_adata_rust::HhasAdata>,
}

impl AdataState {
    pub fn init() -> Self {
        AdataState {
            array_identifier_counter: 0,
            array_identifier_map: std::collections::BTreeMap::new(),
            adata: vec![],
        }
    }
}
