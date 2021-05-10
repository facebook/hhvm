// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<853acc32a63031aa1cc3e355e5b8be32>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use prim_defs::*;

pub use oxidized::file_info::Mode;

pub use oxidized::file_info::NameType;

/// We define two types of positions establishing the location of a given name:
/// a Full position contains the exact position of a name in a file, and a
/// File position contains just the file and the type of toplevel entity,
/// allowing us to lazily retrieve the name's exact location if necessary.
#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
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
pub enum Pos<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Full(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    File(
        &'a (
            oxidized::file_info::NameType,
            &'a relative_path::RelativePath<'a>,
        ),
    ),
}
impl<'a> TrivialDrop for Pos<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Pos<'arena>);

pub type Id<'a> = (Pos<'a>, &'a str);

pub type HashType<'a> = Option<isize>;

/// The record produced by the parsing phase.
#[derive(
    Clone,
    Debug,
    Deserialize,
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
pub struct FileInfo<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub hash: &'a HashType<'a>,
    pub file_mode: Option<oxidized::file_info::Mode>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub funs: &'a [&'a Id<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub classes: &'a [&'a Id<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub record_defs: &'a [&'a Id<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub typedefs: &'a [&'a Id<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub consts: &'a [&'a Id<'a>],
    /// None if loaded from saved state
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub comments: Option<&'a [(&'a pos::Pos<'a>, Comment<'a>)]>,
}
impl<'a> TrivialDrop for FileInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(FileInfo<'arena>);

pub use oxidized::file_info::Names;

/// The simplified record stored in saved-state.
#[derive(
    Clone,
    Debug,
    Deserialize,
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
pub struct SavedNames<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub funs: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub classes: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub record_defs: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub types: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub consts: s_set::SSet<'a>,
}
impl<'a> TrivialDrop for SavedNames<'a> {}
arena_deserializer::impl_deserialize_in_arena!(SavedNames<'arena>);

pub use oxidized::file_info::Saved;

#[derive(
    Clone,
    Debug,
    Deserialize,
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
pub struct Diff<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub removed_funs: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub added_funs: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub removed_classes: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub added_classes: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub removed_types: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub added_types: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub removed_consts: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub added_consts: s_set::SSet<'a>,
}
impl<'a> TrivialDrop for Diff<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Diff<'arena>);
