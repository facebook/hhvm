// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e7f3e027ae25fdb1621762fb59522edf>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use eq_modulo_pos::EqModuloPosAndReason;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    PartialEq,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "PrimPos: 'de + arena_deserializer::DeserializeInArena<'de>, Pos: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C)]
pub struct UserError<'a, PrimPos, Pos> {
    pub code: isize,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub claim: &'a message::Message<'a, PrimPos>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub reasons: &'a [&'a message::Message<'a, Pos>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub quickfixes: &'a [&'a quickfix::Quickfix<'a>],
    pub is_fixmed: bool,
}
impl<'a, PrimPos: TrivialDrop, Pos: TrivialDrop> TrivialDrop for UserError<'a, PrimPos, Pos> {}
arena_deserializer::impl_deserialize_in_arena!(UserError<'arena, PrimPos, Pos>);

pub use oxidized::user_error::Severity;
