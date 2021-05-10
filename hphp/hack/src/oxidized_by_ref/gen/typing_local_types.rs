// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<54fbcf8a6f6d36258299560e273c3e25>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub type ExpressionId = ident::Ident;

#[derive(
    Clone,
    Debug,
    Deserialize,
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
pub struct Local<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Ty<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a pos::Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a ExpressionId,
);
impl<'a> TrivialDrop for Local<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Local<'arena>);

pub type TypingLocalTypes<'a> = local_id::map::Map<'a, &'a Local<'a>>;
