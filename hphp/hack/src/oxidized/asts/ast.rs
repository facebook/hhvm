// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<912f2881dbd37d4ffbe7f8b61eb97cac>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use aast_defs::*;
pub use ast_defs::*;

use crate::aast_defs;
use crate::ast_defs;
#[doc = r" Expressions have no type annotation."]
type Ex = ();
#[doc = r#" Toplevel definitions and methods have no "environment" annotation."#]
type En = ();
pub type Program = aast_defs::Program<Ex, En>;
pub type Stmt = aast_defs::Stmt<Ex, En>;
pub type Stmt_ = aast_defs::Stmt_<Ex, En>;
pub type UsingStmt = aast_defs::UsingStmt<Ex, En>;
pub type AsExpr = aast_defs::AsExpr<Ex, En>;
pub type Block = aast_defs::Block<Ex, En>;
pub type FinallyBlock = aast_defs::FinallyBlock<Ex, En>;
pub type StmtMatch = aast_defs::StmtMatch<Ex, En>;
pub type StmtMatchArm = aast_defs::StmtMatchArm<Ex, En>;
pub type ClassId = aast_defs::ClassId<Ex, En>;
pub type ClassId_ = aast_defs::ClassId_<Ex, En>;
pub type Expr = aast_defs::Expr<Ex, En>;
pub type CollectionTarg = aast_defs::CollectionTarg<Ex>;
pub type FunctionPtrId = aast_defs::FunctionPtrId<Ex, En>;
pub type ExpressionTree = aast_defs::ExpressionTree<Ex, En>;
pub type As_ = aast_defs::As_<Ex, En>;
pub type Expr_ = aast_defs::Expr_<Ex, En>;
pub type Binop = aast_defs::Binop<Ex, En>;
pub type ClassGetExpr = aast_defs::ClassGetExpr<Ex, En>;
pub type Case = aast_defs::Case<Ex, En>;
pub type DefaultCase = aast_defs::DefaultCase<Ex, En>;
pub type GenCase = aast_defs::GenCase<Ex, En>;
pub type Catch = aast_defs::Catch<Ex, En>;
pub type Field = aast_defs::Field<Ex, En>;
pub type Afield = aast_defs::Afield<Ex, En>;
pub type XhpSimple = aast_defs::XhpSimple<Ex, En>;
pub type XhpAttribute = aast_defs::XhpAttribute<Ex, En>;
pub type FunParam = aast_defs::FunParam<Ex, En>;
pub type Fun_ = aast_defs::Fun_<Ex, En>;
pub type CaptureLid = aast_defs::CaptureLid<Ex>;
pub type Efun = aast_defs::Efun<Ex, En>;
pub type FuncBody = aast_defs::FuncBody<Ex, En>;
pub type TypeHint = aast_defs::TypeHint<Ex>;
pub type Targ = aast_defs::Targ<Ex>;
pub type CallExpr = aast_defs::CallExpr<Ex, En>;
pub type UserAttribute = aast_defs::UserAttribute<Ex, En>;
pub type FileAttribute = aast_defs::FileAttribute<Ex, En>;
pub type Tparam = aast_defs::Tparam<Ex, En>;
pub type Class_ = aast_defs::Class_<Ex, En>;
pub type XhpAttr = aast_defs::XhpAttr<Ex, En>;
pub type ClassConstKind = aast_defs::ClassConstKind<Ex, En>;
pub type ClassConst = aast_defs::ClassConst<Ex, En>;
pub type ClassTypeconstDef = aast_defs::ClassTypeconstDef<Ex, En>;
pub type ClassVar = aast_defs::ClassVar<Ex, En>;
pub type Method_ = aast_defs::Method_<Ex, En>;
pub type Typedef = aast_defs::Typedef<Ex, En>;
pub type Gconst = aast_defs::Gconst<Ex, En>;
pub type FunDef = aast_defs::FunDef<Ex, En>;
pub type ModuleDef = aast_defs::ModuleDef<Ex, En>;
pub type Def = aast_defs::Def<Ex, En>;
pub type UserAttributes = aast_defs::UserAttributes<Ex, En>;
