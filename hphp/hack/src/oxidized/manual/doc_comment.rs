// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use ocamlrep_derive::OcamlRep;
use serde::{Deserialize, Serialize};

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct DocComment(pub Rc<String>);

impl DocComment {
    pub fn new(s: String) -> Self {
        Self(Rc::new(s))
    }
}
