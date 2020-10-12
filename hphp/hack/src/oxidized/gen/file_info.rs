// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<45d340fdb3b074aa9241b51b452e627a>>
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

pub use prim_defs::*;

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
pub enum Mode {
    /// just declare signatures, don't check anything
    Mdecl,
    /// check everything!
    Mstrict,
    /// Don't fail if you see a function/class you don't know
    Mpartial,
}
impl TrivialDrop for Mode {}

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
pub enum NameType {
    Fun = 0,
    Class = 1,
    RecordDef = 2,
    Typedef = 3,
    Const = 4,
}
impl TrivialDrop for NameType {}

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
    ToOcamlRep
)]
pub enum Pos {
    Full(pos::Pos),
    File(NameType, ocamlrep::rc::RcOc<relative_path::RelativePath>),
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
    ToOcamlRep
)]
pub struct Id(pub Pos, pub String);

/// The hash value of a decl AST.
/// We use this to see if two versions of a file are "similar", i.e. their
/// declarations only differ by position information.
pub type HashType = Option<opaque_digest::OpaqueDigest>;

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
    ToOcamlRep
)]
pub struct FileInfo {
    pub hash: HashType,
    pub file_mode: Option<Mode>,
    pub funs: Vec<Id>,
    pub classes: Vec<Id>,
    pub record_defs: Vec<Id>,
    pub typedefs: Vec<Id>,
    pub consts: Vec<Id>,
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
    ToOcamlRep
)]
pub struct Names {
    pub funs: s_set::SSet,
    pub classes: s_set::SSet,
    pub record_defs: s_set::SSet,
    pub types: s_set::SSet,
    pub consts: s_set::SSet,
}

/// Data structure stored in the saved state
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
pub struct Saved {
    pub names: Names,
    pub hash: Option<opaque_digest::OpaqueDigest>,
    pub mode: Option<Mode>,
}
