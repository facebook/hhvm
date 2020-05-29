// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<fe4bb18b7f04aa6fd0746eb9cf0ccc5f>>
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
pub struct TypingEnvReturnInfo<'a> {
    pub type_: typing_defs::PossiblyEnforcedTy<'a>,
    pub disposable: bool,
    pub mutable: bool,
    pub explicit: bool,
    pub void_to_rx: bool,
}
impl<'a> TrivialDrop for TypingEnvReturnInfo<'a> {}
