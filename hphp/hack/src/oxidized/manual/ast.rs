// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast;
use crate::aast_defs;
use crate::ast_defs;
use crate::pos::Pos;

pub use aast::*;
pub use aast_defs::*;
pub use ast_defs::*;

// Expressions are annotated with their associated position.
type Ex = Pos;

// Function bodies have no annotation.
type Fb = ();

// Toplevel definitions and methods have no "environment" annotation.
type En = ();

// Type hints have no annotation.
type Hi = ();

pub type Afield = aast::Afield<Ex, Fb, En, Hi>;
pub type AsExpr = aast::AsExpr<Ex, Fb, En, Hi>;
pub type Block = aast::Block<Ex, Fb, En, Hi>;
pub type CaField = aast::CaField<Ex, Fb, En, Hi>;
pub type Case = aast::Case<Ex, Fb, En, Hi>;
pub type Catch = aast::Catch<Ex, Fb, En, Hi>;
pub type Class_ = aast::Class_<Ex, Fb, En, Hi>;
pub type ClassAttr = aast::ClassAttr<Ex, Fb, En, Hi>;
pub type ClassConstKind = aast::ClassConstKind<Ex, Fb, En, Hi>;
pub type ClassConst = aast::ClassConst<Ex, Fb, En, Hi>;
pub type ClassGetExpr = aast::ClassGetExpr<Ex, Fb, En, Hi>;
pub type ClassId = aast::ClassId<Ex, Fb, En, Hi>;
pub type ClassId_ = aast::ClassId_<Ex, Fb, En, Hi>;
pub type ClassTypeconstDef = aast::ClassTypeconstDef<Ex, Fb, En, Hi>;
pub type ClassVar = aast::ClassVar<Ex, Fb, En, Hi>;
pub type CollectionTarg = aast::CollectionTarg<Hi>;
pub type Def = aast::Def<Ex, Fb, En, Hi>;
pub type Expr = aast::Expr<Ex, Fb, En, Hi>;
pub type Expr_ = aast::Expr_<Ex, Fb, En, Hi>;
pub type ExpressionTree = aast::ExpressionTree<Ex, Fb, En, Hi>;
pub type Field = aast::Field<Ex, Fb, En, Hi>;
pub type FileAttribute = aast::FileAttribute<Ex, Fb, En, Hi>;
pub type Fun_ = aast::Fun_<Ex, Fb, En, Hi>;
pub type FuncBody = aast::FuncBody<Ex, Fb, En, Hi>;
pub type FunctionPtrId = aast::FunctionPtrId<Ex, Fb, En, Hi>;
pub type FunDef = aast::FunDef<Ex, Fb, En, Hi>;
pub type FunParam = aast::FunParam<Ex, Fb, En, Hi>;
pub type FunVariadicity = aast::FunVariadicity<Ex, Fb, En, Hi>;
pub type Gconst = aast::Gconst<Ex, Fb, En, Hi>;
pub type Method_ = aast::Method_<Ex, Fb, En, Hi>;
pub type Program = aast::Program<Ex, Fb, En, Hi>;
pub type RecordDef = aast::RecordDef<Ex, Fb, En, Hi>;
pub type Stmt = aast::Stmt<Ex, Fb, En, Hi>;
pub type Stmt_ = aast::Stmt_<Ex, Fb, En, Hi>;
pub type Targ = aast::Targ<Hi>;
pub type Tparam = aast::Tparam<Ex, Fb, En, Hi>;
pub type Typedef = aast::Typedef<Ex, Fb, En, Hi>;
pub type TypeHint = aast::TypeHint<Hi>;
pub type UserAttribute = aast::UserAttribute<Ex, Fb, En, Hi>;
pub type UsingStmt = aast::UsingStmt<Ex, Fb, En, Hi>;
pub type XhpAttr = aast::XhpAttr<Ex, Fb, En, Hi>;
pub type XhpAttribute = aast::XhpAttribute<Ex, Fb, En, Hi>;
