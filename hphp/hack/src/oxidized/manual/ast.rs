// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub use aast::*;
pub use aast_defs::*;
pub use ast_defs::*;

use crate::aast;
use crate::aast_defs;
use crate::ast_defs;

// Expressions have no type annotation.
type Ex = ();

// Toplevel definitions and methods have no "environment" annotation.
type En = ();

pub type Afield = aast::Afield<Ex, En>;
pub type AsExpr = aast::AsExpr<Ex, En>;
pub type Block = aast::Block<Ex, En>;
pub type Case = aast::Case<Ex, En>;
pub type Catch = aast::Catch<Ex, En>;
pub type Class_ = aast::Class_<Ex, En>;
pub type ClassConstKind = aast::ClassConstKind<Ex, En>;
pub type ClassConst = aast::ClassConst<Ex, En>;
pub type ClassGetExpr = aast::ClassGetExpr<Ex, En>;
pub type ClassId = aast::ClassId<Ex, En>;
pub type ClassId_ = aast::ClassId_<Ex, En>;
pub type ClassTypeconstDef = aast::ClassTypeconstDef<Ex, En>;
pub type ClassVar = aast::ClassVar<Ex, En>;
pub type CollectionTarg = aast::CollectionTarg<Ex>;
pub type Def = aast::Def<Ex, En>;
pub type DefaultCase = aast::DefaultCase<Ex, En>;
pub type Expr = aast::Expr<Ex, En>;
pub type Expr_ = aast::Expr_<Ex, En>;
pub type ExpressionTree = aast::ExpressionTree<Ex, En>;
pub type Field = aast::Field<Ex, En>;
pub type FileAttribute = aast::FileAttribute<Ex, En>;
pub type Fun_ = aast::Fun_<Ex, En>;
pub type FuncBody = aast::FuncBody<Ex, En>;
pub type FunctionPtrId = aast::FunctionPtrId<Ex, En>;
pub type FunDef = aast::FunDef<Ex, En>;
pub type FunParam = aast::FunParam<Ex, En>;
pub type Gconst = aast::Gconst<Ex, En>;
pub type GenCase = aast::GenCase<Ex, En>;
pub type Method_ = aast::Method_<Ex, En>;
pub type Module = aast::ModuleDef<Ex, En>;
pub type Program = aast::Program<Ex, En>;
pub type Stmt = aast::Stmt<Ex, En>;
pub type Stmt_ = aast::Stmt_<Ex, En>;
pub type Targ = aast::Targ<Ex>;
pub type Tparam = aast::Tparam<Ex, En>;
pub type Typedef = aast::Typedef<Ex, En>;
pub type TypeHint = aast::TypeHint<Ex>;
pub type UserAttribute = aast::UserAttribute<Ex, En>;
pub type UsingStmt = aast::UsingStmt<Ex, En>;
pub type XhpAttr = aast::XhpAttr<Ex, En>;
pub type XhpAttribute = aast::XhpAttribute<Ex, En>;
