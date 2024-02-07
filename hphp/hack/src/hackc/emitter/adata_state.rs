// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hash::HashMap;
use hhbc::Adata;
use hhbc::AdataId;
use hhbc::TypedValue;

#[derive(Debug, Default)]
pub struct AdataState<'a> {
    shared: HashMap<TypedValue<'a>, AdataId>,
    adata: Vec<Adata<'a>>,
}

impl<'a> AdataState<'a> {
    pub fn push(&mut self, value: TypedValue<'a>) -> AdataId {
        push(&mut self.adata, value)
    }

    pub fn intern(&mut self, tv: TypedValue<'a>) -> AdataId {
        *self
            .shared
            .entry(tv)
            .or_insert_with_key(|tv| push(&mut self.adata, tv.clone()))
    }

    pub fn take_adata(&mut self) -> Vec<Adata<'a>> {
        self.shared = Default::default();
        std::mem::take(&mut self.adata)
    }
}

fn push<'a>(adata: &mut Vec<Adata<'a>>, value: TypedValue<'a>) -> AdataId {
    let id = AdataId::new(adata.len());
    adata.push(Adata { id, value });
    id
}
