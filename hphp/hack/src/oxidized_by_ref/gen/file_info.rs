// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c506e77e333ab6f76a2ab7d45e637701>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use ocamlrep_derive::ToOcamlRep;
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
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Pos<'a> {
    Full(pos::Pos<'a>),
    File(
        oxidized::file_info::NameType,
        relative_path::RelativePath<'a>,
    ),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Id<'a>(pub Pos<'a>, pub &'a str);

/// The hash value of a decl AST.
/// We use this to see if two versions of a file are "similar", i.e. their
/// declarations only differ by position information.
pub type HashType<'a> = Option<opaque_digest::OpaqueDigest<'a>>;

/// The record produced by the parsing phase.
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FileInfo<'a> {
    pub hash: HashType<'a>,
    pub file_mode: Option<oxidized::file_info::Mode>,
    pub funs: &'a [Id<'a>],
    pub classes: &'a [Id<'a>],
    pub record_defs: &'a [Id<'a>],
    pub typedefs: &'a [Id<'a>],
    pub consts: &'a [Id<'a>],
    /// None if loaded from saved state
    pub comments: Option<&'a [(pos::Pos<'a>, Comment<'a>)]>,
}

pub use oxidized::file_info::Names;

pub use oxidized::file_info::Saved;
