// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<654b7bd1cce62e48e9cbe82fe03e89fd>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::typing_reason as reason;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct TypingFakeMembers<'a> {
    pub valid: blame_set::BlameSet<'a>,
    pub invalid: blame_set::BlameSet<'a>,
}
impl<'a> TrivialDrop for TypingFakeMembers<'a> {}
