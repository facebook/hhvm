// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<5fdd5b9b03c090aff40b0cf2b457064c>>
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
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    PartialEq,
    Serialize,
    ToOcamlRep
)]
pub struct Error_<'a, PrimPos, Pos> {
    pub code: oxidized::errors::ErrorCode,
    pub claim: &'a Message<'a, PrimPos>,
    pub reasons: &'a [&'a Message<'a, Pos>],
}
impl<'a, PrimPos: TrivialDrop, Pos: TrivialDrop> TrivialDrop for Error_<'a, PrimPos, Pos> {}

pub type Error<'a> = Error_<'a, &'a pos::Pos<'a>, &'a pos_or_decl::PosOrDecl<'a>>;

#[derive(
    Clone,
    Copy,
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
pub struct AppliedFixme<'a>(pub &'a pos::Pos<'a>, pub isize);
impl<'a> TrivialDrop for AppliedFixme<'a> {}

#[derive(
    Clone,
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
pub struct Errors<'a>(
    pub FilesT<'a, &'a Error<'a>>,
    pub FilesT<'a, AppliedFixme<'a>>,
);
impl<'a> TrivialDrop for Errors<'a> {}
