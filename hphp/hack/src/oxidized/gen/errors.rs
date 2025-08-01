// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<88062700bae19ce091a9bf022c4f9861>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use user_error::Severity;

pub use crate::error_codes::GlobalAccessCheck;
pub use crate::error_codes::Naming;
pub use crate::error_codes::NastCheck;
pub use crate::error_codes::Parsing;
pub use crate::error_codes::Typing;
#[allow(unused_imports)]
use crate::*;

pub type ErrorCode = isize;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
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
#[repr(u8)]
pub enum Format {
    /// Underlined references and color
    Context,
    /// Compact format with color but no references
    Raw,
    /// Numbered and colored references
    Highlighted,
    /// Verbose positions and no color
    Plain,
    /// Verbose context showing expressions, statements, hints, and declarations involved in error
    Extended,
}
impl TrivialDrop for Format {}
arena_deserializer::impl_deserialize_in_arena!(Format);

#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
pub type Error = user_error::UserError<pos::Pos, pos_or_decl::PosOrDecl>;

pub type PerFileErrors = Vec<Error>;

#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type Errors = relative_path::map::Map<Vec<Error>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum SuppressionKind {
    Fixme { forbidden_decl_fixme: bool },
    Ignore,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct FixmeError {
    pub explanation: String,
    pub fixme_pos: pos::Pos,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum FixmeOutcome {
    #[rust_to_ocaml(name = "Not_fixmed")]
    NotFixmed(Option<FixmeError>),
    Fixmed,
}
