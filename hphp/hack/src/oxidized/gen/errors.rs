// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<6168d852c4bef2a6798df881088272ae>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
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

pub type ErrorCode = isize;

/// We use `Pos.t message` on the server and convert to `Pos.absolute message`
/// before sending it to the client
pub type Message<A> = (A, String);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Phase {
    Init,
    Parsing,
    Naming,
    Decl,
    Typing,
}
impl TrivialDrop for Phase {}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Severity {
    Warning,
    Error,
}
impl TrivialDrop for Severity {}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Format {
    Context,
    Raw,
    Highlighted,
}
impl TrivialDrop for Format {}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum NameContext {
    FunctionNamespace,
    ConstantNamespace,
    /// Classes, interfaces, traits, records and type aliases.
    TypeNamespace,
    TraitContext,
    ClassContext,
    RecordContext,
}
impl TrivialDrop for NameContext {}

/// Results of single file analysis.
pub type FileT<A> = phase_map::PhaseMap<Vec<A>>;

/// Results of multi-file analysis.
pub type FilesT<A> = relative_path::map::Map<FileT<A>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    PartialEq,
    Serialize,
    ToOcamlRep
)]
pub struct Error_<A> {
    pub code: ErrorCode,
    pub claim: Message<A>,
    pub reasons: Vec<Message<A>>,
}

pub type Error = Error_<pos::Pos>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct AppliedFixme(pub pos::Pos, pub isize);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Errors(pub FilesT<Error>, pub FilesT<AppliedFixme>);
