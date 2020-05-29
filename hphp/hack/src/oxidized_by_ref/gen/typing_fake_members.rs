// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d050b4e531d9c56231d20750b5d4c0b7>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Blame<'a> {
    BlameCall(&'a pos::Pos<'a>),
    BlameLambda(&'a pos::Pos<'a>),
}
impl<'a> TrivialDrop for Blame<'a> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum TypingFakeMembers<'a> {
    Valid(local_id::set::Set<'a>),
    Invalidated {
        valid: local_id::set::Set<'a>,
        invalid: local_id::set::Set<'a>,
        blame: Blame<'a>,
    },
}
impl<'a> TrivialDrop for TypingFakeMembers<'a> {}
