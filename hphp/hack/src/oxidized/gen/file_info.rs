// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<080a85de4e8c5d37666aacfdc36e0129>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::IntoOcamlRep;
use ocamlvalue_macro::Ocamlvalue;

use crate::opaque_digest;
use crate::pos;
use crate::relative_path;
use crate::s_set;

use crate::prim_defs::*;

#[derive(Clone, Copy, Debug, Eq, IntoOcamlRep, Ocamlvalue, PartialEq)]
pub enum Mode {
    Mphp,
    Mdecl,
    Mstrict,
    Mpartial,
    Mexperimental,
}

#[derive(Clone, Copy, Debug, Eq, IntoOcamlRep, Ocamlvalue, PartialEq)]
pub enum NameType {
    Fun,
    Class,
    Typedef,
    Const,
}

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub enum Pos {
    Full(pos::Pos),
    File(NameType, relative_path::RelativePath),
}

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub struct Id(pub Pos, pub String);

pub type HashType = Option<opaque_digest::OpaqueDigest>;

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub struct FileInfo {
    pub hash: HashType,
    pub file_mode: Option<Mode>,
    pub funs: Vec<Id>,
    pub classes: Vec<Id>,
    pub typedefs: Vec<Id>,
    pub consts: Vec<Id>,
    pub comments: Option<Vec<(pos::Pos, Comment)>>,
}

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub struct Names {
    pub funs: s_set::SSet,
    pub classes: s_set::SSet,
    pub types: s_set::SSet,
    pub consts: s_set::SSet,
}

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub struct Saved {
    pub names: Names,
    pub hash: Option<opaque_digest::OpaqueDigest>,
    pub mode: Option<Mode>,
}
