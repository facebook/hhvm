// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<cf0e451fcd48309bd2f2671fe414c5c4>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::ast_defs::shape_map;

pub use aast::Sid;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub enum FuncBodyAnn<'a> {
    Named,
    NamedWithUnsafeBlocks,
    Unnamed(namespace_env::Env<'a>),
}

pub type Program<'a> = aast::Program<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Def<'a> = aast::Def<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Expr<'a> = aast::Expr<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Expr_<'a> = aast::Expr_<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Stmt<'a> = aast::Stmt<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Block<'a> = aast::Block<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type UserAttribute<'a> = aast::UserAttribute<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type ClassId_<'a> = aast::ClassId_<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Class_<'a> = aast::Class_<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Method_<'a> = aast::Method_<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type FileAttribute<'a> = aast::FileAttribute<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Fun_<'a> = aast::Fun_<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type FunDef<'a> = aast::FunDef<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type FuncBody<'a> = aast::FuncBody<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type FunParam<'a> = aast::FunParam<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type FunVariadicity<'a> = aast::FunVariadicity<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Typedef<'a> = aast::Typedef<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type RecordDef<'a> = aast::RecordDef<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Tparam<'a> = aast::Tparam<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Gconst<'a> = aast::Gconst<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type ClassId<'a> = aast::ClassId<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Catch<'a> = aast::Catch<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Case<'a> = aast::Case<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Field<'a> = aast::Field<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Afield<'a> = aast::Afield<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type MethodRedeclaration<'a> =
    aast::MethodRedeclaration<'a, pos::Pos<'a>, FuncBodyAnn<'a>, (), ()>;

pub type Targ<'a> = aast::Targ<'a, ()>;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub struct IgnoreAttributeEnv<'a> {
    pub ignored_attributes: &'a [&'a str],
}
