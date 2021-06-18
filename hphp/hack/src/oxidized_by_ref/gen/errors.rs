// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<7b563dfdcf9b330554dee4a159a85b2b>>
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

pub use crate::error_codes::Naming;
pub use crate::error_codes::NastCheck;
pub use crate::error_codes::Parsing;
pub use crate::error_codes::Typing;

pub use oxidized::errors::ErrorCode;

/// We use `Pos.t message` and `Pos_or_decl.t message` on the server
/// and convert to `Pos.absolute message` before sending it to the client
pub type Message<'a, A> = (A, &'a str);

pub use oxidized::errors::Phase;

pub use oxidized::errors::Severity;

pub use oxidized::errors::Format;

pub use oxidized::errors::NameContext;

/// Results of single file analysis.
pub type FileT<'a, A> = phase_map::PhaseMap<'a, &'a [A]>;

/// Results of multi-file analysis.
pub type FilesT<'a, A> = relative_path::map::Map<'a, FileT<'a, A>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
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
pub struct Error_<'a, PrimPos, Pos> {
    pub code: oxidized::errors::ErrorCode,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub claim: &'a Message<'a, PrimPos>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub reasons: &'a [&'a Message<'a, Pos>],
}
impl<'a, PrimPos: TrivialDrop, Pos: TrivialDrop> TrivialDrop for Error_<'a, PrimPos, Pos> {}
arena_deserializer::impl_deserialize_in_arena!(Error_<'arena, PrimPos, Pos>);

pub type Error<'a> = Error_<'a, &'a pos::Pos<'a>, &'a pos_or_decl::PosOrDecl<'a>>;

#[derive(
    Clone,
    Copy,
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
pub struct AppliedFixme<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a pos::Pos<'a>,
    pub isize,
);
impl<'a> TrivialDrop for AppliedFixme<'a> {}
arena_deserializer::impl_deserialize_in_arena!(AppliedFixme<'arena>);

pub type PerFileErrors<'a> = FileT<'a, &'a Error<'a>>;

#[derive(
    Clone,
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
pub struct Errors<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub FilesT<'a, &'a Error<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  FilesT<'a, AppliedFixme<'a>>,
);
impl<'a> TrivialDrop for Errors<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Errors<'arena>);
