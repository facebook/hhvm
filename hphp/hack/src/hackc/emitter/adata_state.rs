// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hash::HashMap;
use hhbc::Adata;
use hhbc::TypedValue;

#[derive(Debug, Default)]
pub struct AdataState<'a> {
    shared: HashMap<TypedValue<'a>, &'a str>,
    adata: Vec<Adata<'a>>,
}

impl<'a> AdataState<'a> {
    pub fn push(&mut self, alloc: &'a bumpalo::Bump, value: TypedValue<'a>) -> &'a str {
        push(alloc, &mut self.adata, value)
    }

    pub fn intern(&mut self, alloc: &'a bumpalo::Bump, tv: TypedValue<'a>) -> &'a str {
        self.shared
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
) -> &'a str {
    let id: &str = alloc.alloc_str(&format!("A_{}", adata.len()));
    adata.push(Adata {
        id: id.into(),
        value,
    });
    id
}
