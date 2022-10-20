// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<7cda7729c75e95a12be1bbbac4b7582e>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
pub type TByteString<'a> = str;

/// We use `Pos.t message` and `Pos_or_decl.t message` on the server
/// and convert to `Pos.absolute message` before sending it to the client
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(deserialize = "A: 'de + arena_deserializer::DeserializeInArena<'de>"))]
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C)]
pub struct Message<'a, A>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub A,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a bstr::BStr,
);
impl<'a, A: TrivialDrop> TrivialDrop for Message<'a, A> {}
arena_deserializer::impl_deserialize_in_arena!(Message<'arena, A>);
