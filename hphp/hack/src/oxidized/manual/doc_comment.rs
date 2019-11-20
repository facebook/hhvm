// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::OcamlRep;
use std::rc::Rc;

#[derive(Clone, Debug, OcamlRep)]
pub struct DocComment(pub Rc<String>);

impl DocComment {
    pub fn new(s: String) -> Self {
        Self(Rc::new(s))
    }
}
