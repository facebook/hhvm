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

// Function bodies have no annotation.
type Fb = ();

// Toplevel definitions and methods have no "environment" annotation.
type En = ();

pub type Afield = aast::Afield<Ex, Fb, En>;
pub type AsExpr = aast::AsExpr<Ex, Fb, En>;
pub type Block = aast::Block<Ex, Fb, En>;
pub type CaField = aast::CaField<Ex, Fb, En>;
pub type Case = aast::Case<Ex, Fb, En>;
pub type Catch = aast::Catch<Ex, Fb, En>;
pub type Class_ = aast::Class_<Ex, Fb, En>;
pub type ClassAttr = aast::ClassAttr<Ex, Fb, En>;
pub type ClassConstKind = aast::ClassConstKind<Ex, Fb, En>;
pub type ClassConst = aast::ClassConst<Ex, Fb, En>;
pub type ClassGetExpr = aast::ClassGetExpr<Ex, Fb, En>;
pub type ClassId = aast::ClassId<Ex, Fb, En>;
pub type ClassId_ = aast::ClassId_<Ex, Fb, En>;
pub type ClassTypeconstDef = aast::ClassTypeconstDef<Ex, Fb, En>;
pub type ClassVar = aast::ClassVar<Ex, Fb, En>;
pub type CollectionTarg = aast::CollectionTarg<Ex>;
pub type Def = aast::Def<Ex, Fb, En>;
pub type Expr = aast::Expr<Ex, Fb, En>;
pub type Expr_ = aast::Expr_<Ex, Fb, En>;
pub type ExpressionTree = aast::ExpressionTree<Ex, Fb, En>;
pub type Field = aast::Field<Ex, Fb, En>;
pub type FileAttribute = aast::FileAttribute<Ex, Fb, En>;
pub type Fun_ = aast::Fun_<Ex, Fb, En>;
pub type FuncBody = aast::FuncBody<Ex, Fb, En>;
pub type FunctionPtrId = aast::FunctionPtrId<Ex, Fb, En>;
pub type FunDef = aast::FunDef<Ex, Fb, En>;
pub type FunParam = aast::FunParam<Ex, Fb, En>;
pub type FunVariadicity = aast::FunVariadicity<Ex, Fb, En>;
pub type Gconst = aast::Gconst<Ex, Fb, En>;
pub type Method_ = aast::Method_<Ex, Fb, En>;
pub type Program = aast::Program<Ex, Fb, En>;
pub type RecordDef = aast::RecordDef<Ex, Fb, En>;
pub type Stmt = aast::Stmt<Ex, Fb, En>;
pub type Stmt_ = aast::Stmt_<Ex, Fb, En>;
pub type Targ = aast::Targ<Ex>;
pub type Tparam = aast::Tparam<Ex, Fb, En>;
pub type Typedef = aast::Typedef<Ex, Fb, En>;
pub type TypeHint = aast::TypeHint<Ex>;
pub type UserAttribute = aast::UserAttribute<Ex, Fb, En>;
pub type UsingStmt = aast::UsingStmt<Ex, Fb, En>;
pub type XhpAttr = aast::XhpAttr<Ex, Fb, En>;
pub type XhpAttribute = aast::XhpAttribute<Ex, Fb, En>;
