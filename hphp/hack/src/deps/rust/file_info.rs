// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>
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

#[allow(unused_imports)]
use crate::*;

pub use prim_defs::*;

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
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving (eq, hash, show, enum, ord, sexp_of)")]
#[repr(u8)]
pub enum Mode {
    /// just declare signatures, don't check anything
    Mhhi,
    /// check everything!
    Mstrict,
}
impl TrivialDrop for Mode {}
arena_deserializer::impl_deserialize_in_arena!(Mode);

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
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving (eq, (show { with_path = false }), enum, ord)")]
#[repr(u8)]
pub enum NameType {
    Fun = 3,
    Class = 0,
    Typedef = 1,
    Const = 4,
    Module = 5,
}
impl TrivialDrop for NameType {}
arena_deserializer::impl_deserialize_in_arena!(NameType);

/// We define two types of positions establishing the location of a given name:
/// a Full position contains the exact position of a name in a file, and a
/// File position contains just the file and the type of toplevel entity,
/// allowing us to lazily retrieve the name's exact location if necessary.
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
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum Pos {
    Full(pos::Pos),
    File(NameType, ocamlrep::rc::RcOc<relative_path::RelativePath>),
}

/// The simplified record used after parsing.
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
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "n_")]
#[repr(C)]
pub struct Names {
    pub funs: s_set::SSet,
    pub classes: s_set::SSet,
    pub types: s_set::SSet,
    pub consts: s_set::SSet,
    pub modules: s_set::SSet,
}

/// The simplified record stored in saved-state.
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
    ToOcamlRep,
)]
#[rust_to_ocaml(prefix = "sn_")]
#[repr(C)]
pub struct SavedNames {
    pub funs: s_set::SSet,
    pub classes: s_set::SSet,
    pub types: s_set::SSet,
    pub consts: s_set::SSet,
    pub modules: s_set::SSet,
}

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
    ToOcamlRep,
)]
#[repr(C)]
pub struct Diff {
    pub removed_funs: s_set::SSet,
    pub added_funs: s_set::SSet,
    pub removed_classes: s_set::SSet,
    pub added_classes: s_set::SSet,
    pub removed_types: s_set::SSet,
    pub added_types: s_set::SSet,
    pub removed_consts: s_set::SSet,
    pub added_consts: s_set::SSet,
    pub removed_modules: s_set::SSet,
    pub added_modules: s_set::SSet,
}
