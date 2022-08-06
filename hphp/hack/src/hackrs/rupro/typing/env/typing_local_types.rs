// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(dead_code)]
use im::HashMap;
use ty::local::Ty;
use ty::reason::Reason;
use utils::core::Ident;
use utils::core::LocalId;

#[derive(Debug, Clone)]
pub struct Local<R: Reason> {
    pub ty: Ty<R>,
    pub pos: R::Pos,
    pub expr_id: Ident,
}

#[derive(Debug, Clone)]
pub struct LocalMap<R: Reason>(HashMap<LocalId, Local<R>>);

impl<R: Reason> LocalMap<R> {
    pub fn new() -> Self {
        Self(HashMap::new())
    }

    pub fn add(&mut self, x: LocalId, v: Local<R>) {
        self.0.insert(x, v);
    }

    pub fn get(&self, x: &LocalId) -> Option<&Local<R>> {
        self.0.get(x)
    }
}
