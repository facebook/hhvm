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
use ocamlrep_caml_builtins::Int64;
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

/// And here's a version with more detail and still less structure! This
/// one is good for members as well as top-level symbols. It's used to get
/// a bit more out of direct-decl-parse, used to populate the search indexer
/// (e.g. the icon that apepars in autocomplete suggestions).
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
#[rust_to_ocaml(attr = "deriving (eq, (show { with_path = false }))")]
#[repr(u8)]
pub enum SiKind {
    #[rust_to_ocaml(name = "SI_Class")]
    SIClass,
    #[rust_to_ocaml(name = "SI_Interface")]
    SIInterface,
    #[rust_to_ocaml(name = "SI_Enum")]
    SIEnum,
    #[rust_to_ocaml(name = "SI_Trait")]
    SITrait,
    #[rust_to_ocaml(name = "SI_Unknown")]
    SIUnknown,
    #[rust_to_ocaml(name = "SI_Mixed")]
    SIMixed,
    #[rust_to_ocaml(name = "SI_Function")]
    SIFunction,
    #[rust_to_ocaml(name = "SI_Typedef")]
    SITypedef,
    #[rust_to_ocaml(name = "SI_GlobalConstant")]
    SIGlobalConstant,
    #[rust_to_ocaml(name = "SI_XHP")]
    SIXHP,
    #[rust_to_ocaml(name = "SI_Namespace")]
    SINamespace,
    #[rust_to_ocaml(name = "SI_ClassMethod")]
    SIClassMethod,
    #[rust_to_ocaml(name = "SI_Literal")]
    SILiteral,
    #[rust_to_ocaml(name = "SI_ClassConstant")]
    SIClassConstant,
    #[rust_to_ocaml(name = "SI_Property")]
    SIProperty,
    #[rust_to_ocaml(name = "SI_LocalVariable")]
    SILocalVariable,
    #[rust_to_ocaml(name = "SI_Keyword")]
    SIKeyword,
    #[rust_to_ocaml(name = "SI_Constructor")]
    SIConstructor,
}
impl TrivialDrop for SiKind {}
arena_deserializer::impl_deserialize_in_arena!(SiKind);

/// Yet more details. The "is_abstract" and "is_final" are used e.g. for autocomplete
/// items and to determine whether a type can be suggested e.g. for "$x = new |".
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
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "sia_")]
#[repr(C)]
pub struct SiAddendum {
    /// This is expected not to contain the leading namespace backslash! See [Utils.strip_ns].
    pub name: String,
    pub kind: SiKind,
    pub is_abstract: bool,
    pub is_final: bool,
}

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
    File(NameType, std::sync::Arc<relative_path::RelativePath>),
}

/// An id contains a pos, name and a optional decl hash. The decl hash is None
/// only in the case when we didn't compute it for performance reasons
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
#[repr(C)]
pub struct Id(pub Pos, pub String, pub Option<Int64>);

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
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving eq")]
#[repr(C)]
pub struct HashType(pub Option<Int64>);

/// The record produced by the parsing phase.
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
#[repr(C)]
pub struct FileInfo {
    pub hash: HashType,
    pub file_mode: Option<Mode>,
    pub funs: Vec<Id>,
    pub classes: Vec<Id>,
    pub typedefs: Vec<Id>,
    pub consts: Vec<Id>,
    pub modules: Vec<Id>,
    /// None if loaded from saved state
    pub comments: Option<Vec<(pos::Pos, Comment)>>,
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
