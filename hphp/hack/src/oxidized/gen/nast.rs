// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8761a95cde0c0033cacdf77269b27e50>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::ast_defs::shape_map;

pub use aast::ClassHint;
pub use aast::RecordHint;
pub use aast::Sid;
pub use aast::TraitHint;
pub use aast::XhpAttrHint;
pub use ast_defs::ShapeFieldName;

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

pub type ClassVar = aast::ClassVar<pos::Pos, FuncBodyAnn, (), ()>;

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

pub type ClassConst = aast::ClassConst<pos::Pos, FuncBodyAnn, (), ()>;

pub type ClassId = aast::ClassId<pos::Pos, FuncBodyAnn, (), ()>;

pub type Catch = aast::Catch<pos::Pos, FuncBodyAnn, (), ()>;

pub type Case = aast::Case<pos::Pos, FuncBodyAnn, (), ()>;

pub type Field = aast::Field<pos::Pos, FuncBodyAnn, (), ()>;

pub type Afield = aast::Afield<pos::Pos, FuncBodyAnn, (), ()>;

pub type ExpressionTree = aast::ExpressionTree<pos::Pos, FuncBodyAnn, (), ()>;

pub type Targ = aast::Targ<()>;

pub type TypeHint = aast::TypeHint<()>;

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
pub struct IgnoreAttributeEnv {
    pub ignored_attributes: Vec<String>,
}
