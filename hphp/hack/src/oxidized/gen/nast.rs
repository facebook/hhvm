// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<7b173aa0a3853e4b95a591841b3cf0d5>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
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
pub enum FuncBodyAnn {
    Named,
    NamedWithUnsafeBlocks,
}
impl TrivialDrop for FuncBodyAnn {}
arena_deserializer::impl_deserialize_in_arena!(FuncBodyAnn);

pub type Program = aast::Program<(), FuncBodyAnn, ()>;

pub type Def = aast::Def<(), FuncBodyAnn, ()>;

pub type Expr = aast::Expr<(), FuncBodyAnn, ()>;

pub type Expr_ = aast::Expr_<(), FuncBodyAnn, ()>;

pub type Stmt = aast::Stmt<(), FuncBodyAnn, ()>;

pub type Block = aast::Block<(), FuncBodyAnn, ()>;

pub type UserAttribute = aast::UserAttribute<(), FuncBodyAnn, ()>;

pub type ClassId_ = aast::ClassId_<(), FuncBodyAnn, ()>;

pub type Class_ = aast::Class_<(), FuncBodyAnn, ()>;

pub type ClassVar = aast::ClassVar<(), FuncBodyAnn, ()>;

pub type Method_ = aast::Method_<(), FuncBodyAnn, ()>;

pub type FileAttribute = aast::FileAttribute<(), FuncBodyAnn, ()>;

pub type Fun_ = aast::Fun_<(), FuncBodyAnn, ()>;

pub type FunDef = aast::FunDef<(), FuncBodyAnn, ()>;

pub type FuncBody = aast::FuncBody<(), FuncBodyAnn, ()>;

pub type FunParam = aast::FunParam<(), FuncBodyAnn, ()>;

pub type FunVariadicity = aast::FunVariadicity<(), FuncBodyAnn, ()>;

pub type Typedef = aast::Typedef<(), FuncBodyAnn, ()>;

pub type RecordDef = aast::RecordDef<(), FuncBodyAnn, ()>;

pub type Tparam = aast::Tparam<(), FuncBodyAnn, ()>;

pub type Gconst = aast::Gconst<(), FuncBodyAnn, ()>;

pub type ClassConst = aast::ClassConst<(), FuncBodyAnn, ()>;

pub type ClassId = aast::ClassId<(), FuncBodyAnn, ()>;

pub type Catch = aast::Catch<(), FuncBodyAnn, ()>;

pub type Case = aast::Case<(), FuncBodyAnn, ()>;

pub type Field = aast::Field<(), FuncBodyAnn, ()>;

pub type Afield = aast::Afield<(), FuncBodyAnn, ()>;

pub type XhpAttribute = aast::XhpAttribute<(), FuncBodyAnn, ()>;

pub type ExpressionTree = aast::ExpressionTree<(), FuncBodyAnn, ()>;

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
