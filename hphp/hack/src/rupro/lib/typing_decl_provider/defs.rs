// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::FoldedClass;
use crate::reason::Reason;
use crate::typing_defs::ClassElt;
use dashmap::DashMap;
use pos::Symbol;
use std::sync::Arc;

#[derive(Debug)]
struct EagerMembers<R: Reason> {
    #[allow(dead_code)]
    methods: DashMap<Symbol, ClassElt<R>>,
}

#[derive(Debug)]
pub struct Class<R: Reason>(Arc<FoldedClass<R>>, EagerMembers<R>);

impl<R: Reason> EagerMembers<R> {
    fn new() -> Self {
        Self {
            methods: DashMap::new(),
        }
    }
}

impl<R: Reason> Class<R> {
    pub fn new(folded_class: Arc<FoldedClass<R>>) -> Self {
        Self(folded_class, EagerMembers::new())
    }
}
