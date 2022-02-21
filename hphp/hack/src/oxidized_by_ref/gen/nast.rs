// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ad126dfd48a42c390cee4b9a2b3fd94d>>
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
pub use aast::Hint;
pub use aast::Sid;
pub use aast::TraitHint;
pub use aast::XhpAttrHint;
pub use ast_defs::ShapeFieldName;

pub type Program<'a> = aast::Program<'a, (), ()>;

pub type Def<'a> = aast::Def<'a, (), ()>;

pub type Expr<'a> = aast::Expr<'a, (), ()>;

pub type Expr_<'a> = aast::Expr_<'a, (), ()>;

pub type Stmt<'a> = aast::Stmt<'a, (), ()>;

pub type Block<'a> = aast::Block<'a, (), ()>;

pub type UserAttribute<'a> = aast::UserAttribute<'a, (), ()>;

pub type ClassId_<'a> = aast::ClassId_<'a, (), ()>;

pub type Class_<'a> = aast::Class_<'a, (), ()>;

pub type ClassVar<'a> = aast::ClassVar<'a, (), ()>;

pub type Method_<'a> = aast::Method_<'a, (), ()>;

pub type FileAttribute<'a> = aast::FileAttribute<'a, (), ()>;

pub type Fun_<'a> = aast::Fun_<'a, (), ()>;

pub type FunDef<'a> = aast::FunDef<'a, (), ()>;

pub type FuncBody<'a> = aast::FuncBody<'a, (), ()>;

pub type FunParam<'a> = aast::FunParam<'a, (), ()>;

pub type Typedef<'a> = aast::Typedef<'a, (), ()>;

pub type Tparam<'a> = aast::Tparam<'a, (), ()>;

pub type Gconst<'a> = aast::Gconst<'a, (), ()>;

pub type ClassConst<'a> = aast::ClassConst<'a, (), ()>;

pub type ClassId<'a> = aast::ClassId<'a, (), ()>;

pub type Catch<'a> = aast::Catch<'a, (), ()>;

pub type Case<'a> = aast::Case<'a, (), ()>;

pub type DefaultCase<'a> = aast::DefaultCase<'a, (), ()>;

pub type GenCase<'a> = aast::GenCase<'a, (), ()>;

pub type Field<'a> = aast::Field<'a, (), ()>;

pub type Afield<'a> = aast::Afield<'a, (), ()>;

pub type XhpAttribute<'a> = aast::XhpAttribute<'a, (), ()>;

pub type ExpressionTree<'a> = aast::ExpressionTree<'a, (), ()>;

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
#[repr(C)]
pub struct IgnoreAttributeEnv<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ignored_attributes: &'a [&'a str],
}
impl<'a> TrivialDrop for IgnoreAttributeEnv<'a> {}
arena_deserializer::impl_deserialize_in_arena!(IgnoreAttributeEnv<'arena>);
