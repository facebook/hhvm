// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::IndexSet;
use serde::Serialize;

use crate::AdataId;
use crate::TypedValue;

#[derive(Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Adata {
    pub id: AdataId,
    pub value: TypedValue,
}

impl Adata {
    pub const VEC_PREFIX: &'static str = "v";
    pub const DICT_PREFIX: &'static str = "D";
    pub const KEYSET_PREFIX: &'static str = "k";
}

#[derive(Debug, Default)]
pub struct AdataState {
    shared: IndexSet<TypedValue>,
}

impl AdataState {
    pub fn intern(&mut self, tv: TypedValue) -> AdataId {
        let (i, _) = self.shared.insert_full(tv);
        AdataId::new(i)
    }

    pub fn finish(self) -> Vec<Adata> {
        self.shared
            .into_iter()
            .enumerate()
            .map(|(i, value)| Adata {
                id: AdataId::new(i),
                value,
            })
            .collect()
    }
}
