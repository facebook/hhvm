// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ee793f3555cabb46004caf625f0fd60f>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use user_diagnostic::Severity;

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
pub type Diagnostic = user_diagnostic::UserDiagnostic<pos::Pos, pos_or_decl::PosOrDecl>;

/// Type representing the errors for a single file.
pub type PerFileDiagnostics = Vec<Diagnostic>;

/// The type of collections of errors
#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type Diagnostics = relative_path::map::Map<Vec<Diagnostic>>;

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
