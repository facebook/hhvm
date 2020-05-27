// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0f2e06b37afbc0ec67fc334f48bd98bd>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

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
pub enum Pos {
    Full(std::rc::Rc<pos::Pos>),
    File(
        oxidized::file_info::NameType,
        ocamlrep::rc::RcOc<relative_path::RelativePath>,
    ),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Id(pub std::rc::Rc<Pos>, pub std::rc::Rc<String>);

/// The hash value of a decl AST.
/// We use this to see if two versions of a file are "similar", i.e. their
/// declarations only differ by position information.
pub type HashType = Option<opaque_digest::OpaqueDigest>;

/// The record produced by the parsing phase.
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FileInfo {
    pub hash: HashType,
    pub file_mode: Option<oxidized::file_info::Mode>,
    pub funs: Vec<Id>,
    pub classes: Vec<Id>,
    pub record_defs: Vec<Id>,
    pub typedefs: Vec<Id>,
    pub consts: Vec<Id>,
    /// None if loaded from saved state
    pub comments: Option<Vec<(std::rc::Rc<pos::Pos>, Comment)>>,
}

pub use oxidized::file_info::Names;

pub use oxidized::file_info::Saved;
