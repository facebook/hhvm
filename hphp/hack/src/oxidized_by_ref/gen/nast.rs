// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<19ddb8714eeb16007d80582b0dc962bb>>
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
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum FuncBodyAnn<'a> {
    Named,
    NamedWithUnsafeBlocks,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Unnamed(&'a namespace_env::Env<'a>),
}
impl<'a> TrivialDrop for FuncBodyAnn<'a> {}
arena_deserializer::impl_deserialize_in_arena!(FuncBodyAnn<'arena>);

pub type Program<'a> = aast::Program<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Def<'a> = aast::Def<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Expr<'a> = aast::Expr<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Expr_<'a> = aast::Expr_<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Stmt<'a> = aast::Stmt<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Block<'a> = aast::Block<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type UserAttribute<'a> = aast::UserAttribute<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type ClassId_<'a> = aast::ClassId_<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Class_<'a> = aast::Class_<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type ClassVar<'a> = aast::ClassVar<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Method_<'a> = aast::Method_<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type FileAttribute<'a> = aast::FileAttribute<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Fun_<'a> = aast::Fun_<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type FunDef<'a> = aast::FunDef<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type FuncBody<'a> = aast::FuncBody<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type FunParam<'a> = aast::FunParam<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type FunVariadicity<'a> = aast::FunVariadicity<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Typedef<'a> = aast::Typedef<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type RecordDef<'a> = aast::RecordDef<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Tparam<'a> = aast::Tparam<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Gconst<'a> = aast::Gconst<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type ClassConst<'a> = aast::ClassConst<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type ClassId<'a> = aast::ClassId<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Catch<'a> = aast::Catch<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Case<'a> = aast::Case<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Field<'a> = aast::Field<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Afield<'a> = aast::Afield<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type ExpressionTree<'a> = aast::ExpressionTree<'a, (), FuncBodyAnn<'a>, (), ()>;

pub type Targ<'a> = aast::Targ<'a, ()>;

pub type TypeHint<'a> = aast::TypeHint<'a, ()>;

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
pub struct IgnoreAttributeEnv<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ignored_attributes: &'a [&'a str],
}
impl<'a> TrivialDrop for IgnoreAttributeEnv<'a> {}
arena_deserializer::impl_deserialize_in_arena!(IgnoreAttributeEnv<'arena>);
