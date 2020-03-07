// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d4ba2020f01a84efb303c42cf893e8a1>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::ast_defs::shape_map;

pub use aast::PuLoc;
pub use aast::Sid;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum FuncBodyAnn {
    Named,
    NamedWithUnsafeBlocks,
    Unnamed(namespace_env::Env),
}

pub type Program = aast::Program<pos::Pos, FuncBodyAnn, (), ()>;

pub type Def = aast::Def<pos::Pos, FuncBodyAnn, (), ()>;

pub type Expr = aast::Expr<pos::Pos, FuncBodyAnn, (), ()>;

pub type Expr_ = aast::Expr_<pos::Pos, FuncBodyAnn, (), ()>;

pub type Stmt = aast::Stmt<pos::Pos, FuncBodyAnn, (), ()>;

pub type Block = aast::Block<pos::Pos, FuncBodyAnn, (), ()>;

pub type UserAttribute = aast::UserAttribute<pos::Pos, FuncBodyAnn, (), ()>;

pub type ClassId_ = aast::ClassId_<pos::Pos, FuncBodyAnn, (), ()>;

pub type Class_ = aast::Class_<pos::Pos, FuncBodyAnn, (), ()>;

pub type Method_ = aast::Method_<pos::Pos, FuncBodyAnn, (), ()>;

pub type FileAttribute = aast::FileAttribute<pos::Pos, FuncBodyAnn, (), ()>;

pub type Fun_ = aast::Fun_<pos::Pos, FuncBodyAnn, (), ()>;

pub type FunDef = aast::FunDef<pos::Pos, FuncBodyAnn, (), ()>;

pub type FuncBody = aast::FuncBody<pos::Pos, FuncBodyAnn, (), ()>;

pub type FunParam = aast::FunParam<pos::Pos, FuncBodyAnn, (), ()>;

pub type FunVariadicity = aast::FunVariadicity<pos::Pos, FuncBodyAnn, (), ()>;

pub type Typedef = aast::Typedef<pos::Pos, FuncBodyAnn, (), ()>;

pub type RecordDef = aast::RecordDef<pos::Pos, FuncBodyAnn, (), ()>;

pub type Tparam = aast::Tparam<pos::Pos, FuncBodyAnn, (), ()>;

pub type Gconst = aast::Gconst<pos::Pos, FuncBodyAnn, (), ()>;

pub type ClassId = aast::ClassId<pos::Pos, FuncBodyAnn, (), ()>;

pub type Catch = aast::Catch<pos::Pos, FuncBodyAnn, (), ()>;

pub type Case = aast::Case<pos::Pos, FuncBodyAnn, (), ()>;

pub type Field = aast::Field<pos::Pos, FuncBodyAnn, (), ()>;

pub type Afield = aast::Afield<pos::Pos, FuncBodyAnn, (), ()>;

pub type MethodRedeclaration = aast::MethodRedeclaration<pos::Pos, FuncBodyAnn, (), ()>;

pub type Targ = aast::Targ<()>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct IgnoreAttributeEnv {
    pub ignored_attributes: Vec<String>,
}
