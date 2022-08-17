// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e4bdc778d0e399ed0a7b6c7706d68895>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use eq_modulo_pos::EqModuloPosAndReason;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

pub use crate::error_codes::GlobalWriteCheck;
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
    EqModuloPosAndReason,
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
#[rust_to_ocaml(attr = "deriving (eq, show, enum)")]
#[repr(u8)]
pub enum Phase {
    Init,
    Parsing,
    Naming,
    Decl,
    Typing,
}
impl TrivialDrop for Phase {}
arena_deserializer::impl_deserialize_in_arena!(Phase);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
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
    Context,
    Raw,
    Highlighted,
}
impl TrivialDrop for Format {}
arena_deserializer::impl_deserialize_in_arena!(Format);

/// Results of single file analysis.
#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type FileT<A> = phase_map::PhaseMap<Vec<A>>;

/// Results of multi-file analysis.
#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type FilesT<A> = relative_path::map::Map<FileT<A>>;

#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
pub type Error = user_error::UserError<pos::Pos, pos_or_decl::PosOrDecl>;

pub type PerFileErrors = FileT<Error>;

#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type Errors = FilesT<Error>;
