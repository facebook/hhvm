// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hash::HashMap;
use hhbc::Adata;
use hhbc::AdataId;
use hhbc::TypedValue;

#[derive(Debug, Default)]
pub struct AdataState {
    shared: HashMap<TypedValue, AdataId>,
    adata: Vec<Adata>,
}

impl AdataState {
    pub fn intern(&mut self, tv: TypedValue) -> AdataId {
        *self
            .shared
            .entry(tv)
            .or_insert_with_key(|tv| push(&mut self.adata, tv.clone()))
    }

    pub fn take_adata(&mut self) -> Vec<Adata> {
        self.shared = Default::default();
        std::mem::take(&mut self.adata)
    }
}

fn push(adata: &mut Vec<Adata>, value: TypedValue) -> AdataId {
    let id = AdataId::new(adata.len());
    adata.push(Adata { id, value });
    id
}
