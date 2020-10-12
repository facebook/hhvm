// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<953014b12465ff8ec69b1a866a5abff0>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct TypingEnvReturnInfo<'a> {
    pub type_: &'a typing_defs::PossiblyEnforcedTy<'a>,
    pub disposable: bool,
    pub mutable: bool,
    pub explicit: bool,
    pub void_to_rx: bool,
}
impl<'a> TrivialDrop for TypingEnvReturnInfo<'a> {}
