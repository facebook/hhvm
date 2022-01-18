// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(dead_code)]
use std::collections::HashMap;
use std::rc::Rc;

use crate::decl_defs::FoldedClass;
use crate::pos::Symbol;
use crate::reason::Reason;
use crate::typing_defs::ClassElt;

#[derive(Debug)]
struct EagerMembers<R: Reason> {
    methods: HashMap<Symbol, ClassElt<R>>,
}

#[derive(Debug)]
pub struct Class<R: Reason>(Rc<FoldedClass<R>>, EagerMembers<R>);

impl<R: Reason> EagerMembers<R> {
    fn new() -> Self {
        Self {
            methods: HashMap::new(),
        }
    }
}

impl<R: Reason> Class<R> {
    pub fn new(folded_class: Rc<FoldedClass<R>>) -> Self {
        Self(folded_class, EagerMembers::new())
    }
}
