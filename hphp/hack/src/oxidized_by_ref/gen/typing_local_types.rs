// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<84b39e849d8c9bc53e8919a3f47406ba>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub type ExpressionId = ident::Ident;

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
pub struct Local<'a>(pub &'a Ty<'a>, pub &'a pos::Pos<'a>, pub &'a ExpressionId);
impl<'a> TrivialDrop for Local<'a> {}

pub type TypingLocalTypes<'a> = local_id::map::Map<'a, &'a Local<'a>>;
