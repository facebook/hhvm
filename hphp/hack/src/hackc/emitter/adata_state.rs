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
    shared: HashMap<TypedValue<'a>, AdataId<'a>>,
    adata: Vec<Adata<'a>>,
}

impl<'a> AdataState<'a> {
    pub fn push(&mut self, alloc: &'a bumpalo::Bump, value: TypedValue<'a>) -> AdataId<'a> {
        push(alloc, &mut self.adata, value)
    }

    pub fn intern(&mut self, alloc: &'a bumpalo::Bump, tv: TypedValue<'a>) -> AdataId<'a> {
        *self
            .shared
            .entry(tv)
            .or_insert_with_key(|tv| push(alloc, &mut self.adata, tv.clone()))
    }

    pub fn take_adata(&mut self) -> Vec<Adata<'a>> {
        self.shared = Default::default();
        std::mem::take(&mut self.adata)
    }
}

fn push<'a>(
    alloc: &'a bumpalo::Bump,
    adata: &mut Vec<Adata<'a>>,
    value: TypedValue<'a>,
) -> AdataId<'a> {
    let id = AdataId::from_raw_string(alloc, &format!("A_{}", adata.len()));
    adata.push(Adata { id, value });
    id
}
