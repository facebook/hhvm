// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<bd18150f575b815b57f09f932225c438>>
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

pub type Program<'a> = aast::Program<'a, (), FuncBodyAnn, ()>;

pub type Def<'a> = aast::Def<'a, (), FuncBodyAnn, ()>;

pub type Expr<'a> = aast::Expr<'a, (), FuncBodyAnn, ()>;

pub type Expr_<'a> = aast::Expr_<'a, (), FuncBodyAnn, ()>;

pub type Stmt<'a> = aast::Stmt<'a, (), FuncBodyAnn, ()>;

pub type Block<'a> = aast::Block<'a, (), FuncBodyAnn, ()>;

pub type UserAttribute<'a> = aast::UserAttribute<'a, (), FuncBodyAnn, ()>;

pub type ClassId_<'a> = aast::ClassId_<'a, (), FuncBodyAnn, ()>;

pub type Class_<'a> = aast::Class_<'a, (), FuncBodyAnn, ()>;

pub type ClassVar<'a> = aast::ClassVar<'a, (), FuncBodyAnn, ()>;

pub type Method_<'a> = aast::Method_<'a, (), FuncBodyAnn, ()>;

pub type FileAttribute<'a> = aast::FileAttribute<'a, (), FuncBodyAnn, ()>;

pub type Fun_<'a> = aast::Fun_<'a, (), FuncBodyAnn, ()>;

pub type FunDef<'a> = aast::FunDef<'a, (), FuncBodyAnn, ()>;

pub type FuncBody<'a> = aast::FuncBody<'a, (), FuncBodyAnn, ()>;

pub type FunParam<'a> = aast::FunParam<'a, (), FuncBodyAnn, ()>;

pub type FunVariadicity<'a> = aast::FunVariadicity<'a, (), FuncBodyAnn, ()>;

pub type Typedef<'a> = aast::Typedef<'a, (), FuncBodyAnn, ()>;

pub type RecordDef<'a> = aast::RecordDef<'a, (), FuncBodyAnn, ()>;

pub type Tparam<'a> = aast::Tparam<'a, (), FuncBodyAnn, ()>;

pub type Gconst<'a> = aast::Gconst<'a, (), FuncBodyAnn, ()>;

pub type ClassConst<'a> = aast::ClassConst<'a, (), FuncBodyAnn, ()>;

pub type ClassId<'a> = aast::ClassId<'a, (), FuncBodyAnn, ()>;

pub type Catch<'a> = aast::Catch<'a, (), FuncBodyAnn, ()>;

pub type Case<'a> = aast::Case<'a, (), FuncBodyAnn, ()>;

pub type Field<'a> = aast::Field<'a, (), FuncBodyAnn, ()>;

pub type Afield<'a> = aast::Afield<'a, (), FuncBodyAnn, ()>;

pub type XhpAttribute<'a> = aast::XhpAttribute<'a, (), FuncBodyAnn, ()>;

pub type ExpressionTree<'a> = aast::ExpressionTree<'a, (), FuncBodyAnn, ()>;

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
