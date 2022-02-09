// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast;
use crate::aast_defs;
use crate::ast_defs;

pub use aast::*;
pub use aast_defs::*;
pub use ast_defs::*;

// Expressions have no type annotation.
type Ex = ();

// Toplevel definitions and methods have no "environment" annotation.
type En = ();

pub type Afield<'a> = aast::Afield<'a, Ex, En>;
pub type AsExpr<'a> = aast::AsExpr<'a, Ex, En>;
pub type Block<'a> = aast::Block<'a, Ex, En>;
pub type CaField<'a> = aast::CaField<'a, Ex, En>;
pub type Case<'a> = aast::Case<'a, Ex, En>;
pub type Catch<'a> = aast::Catch<'a, Ex, En>;
pub type Class_<'a> = aast::Class_<'a, Ex, En>;
pub type ClassAttr<'a> = aast::ClassAttr<'a, Ex, En>;
pub type ClassConst<'a> = aast::ClassConst<'a, Ex, En>;
pub type ClassGetExpr<'a> = aast::ClassGetExpr<'a, Ex, En>;
pub type ClassId<'a> = aast::ClassId<'a, Ex, En>;
pub type ClassId_<'a> = aast::ClassId_<'a, Ex, En>;
pub type ClassTypeconstDef<'a> = aast::ClassTypeconstDef<'a, Ex, En>;
pub type ClassVar<'a> = aast::ClassVar<'a, Ex, En>;
pub type CollectionTarg<'a> = aast::CollectionTarg<'a, Ex>;
pub type Def<'a> = aast::Def<'a, Ex, En>;
pub type Expr<'a> = aast::Expr<'a, Ex, En>;
pub type Expr_<'a> = aast::Expr_<'a, Ex, En>;
pub type Field<'a> = aast::Field<'a, Ex, En>;
pub type FileAttribute<'a> = aast::FileAttribute<'a, Ex, En>;
pub type Fun_<'a> = aast::Fun_<'a, Ex, En>;
pub type FuncBody<'a> = aast::FuncBody<'a, Ex, En>;
pub type FunDef<'a> = aast::FunDef<'a, Ex, En>;
pub type FunParam<'a> = aast::FunParam<'a, Ex, En>;
pub type Gconst<'a> = aast::Gconst<'a, Ex, En>;
pub type Method_<'a> = aast::Method_<'a, Ex, En>;
pub type Program<'a> = aast::Program<'a, Ex, En>;
pub type Stmt<'a> = aast::Stmt<'a, Ex, En>;
pub type Stmt_<'a> = aast::Stmt_<'a, Ex, En>;
pub type Targ<'a> = aast::Targ<'a, Ex>;
pub type Tparam<'a> = aast::Tparam<'a, Ex, En>;
pub type Typedef<'a> = aast::Typedef<'a, Ex, En>;
pub type TypeHint<'a> = aast::TypeHint<'a, Ex>;
pub type UserAttribute<'a> = aast::UserAttribute<'a, Ex, En>;
pub type UsingStmt<'a> = aast::UsingStmt<'a, Ex, En>;
pub type XhpAttr<'a> = aast::XhpAttr<'a, Ex, En>;
pub type XhpAttribute<'a> = aast::XhpAttribute<'a, Ex, En>;
