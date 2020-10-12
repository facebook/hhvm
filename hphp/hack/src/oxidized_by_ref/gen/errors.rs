// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<883260e02ee0f31474875dce17457e58>>
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

pub use crate::error_codes::Naming;
pub use crate::error_codes::NastCheck;
pub use crate::error_codes::Parsing;
pub use crate::error_codes::Typing;

pub use oxidized::errors::ErrorCode;

/// We use `Pos.t message` on the server and convert to `Pos.absolute message`
/// before sending it to the client
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
pub struct Error_<'a, A> {
    pub code: oxidized::errors::ErrorCode,
    pub claim: &'a Message<'a, A>,
    pub reasons: &'a [&'a Message<'a, A>],
}
impl<'a, A: TrivialDrop> TrivialDrop for Error_<'a, A> {}

pub type Error<'a> = Error_<'a, &'a pos::Pos<'a>>;

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
