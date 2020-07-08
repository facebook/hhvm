// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::Serialize;

use ocamlrep_derive::ToOcamlRep;

#[derive(
    Copy, Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct DocComment<'a>(pub &'a str);

impl arena_trait::TrivialDrop for DocComment<'_> {}

impl<'a> DocComment<'a> {
    pub fn new(s: &'a str) -> Self {
        Self(s)
    }
}
