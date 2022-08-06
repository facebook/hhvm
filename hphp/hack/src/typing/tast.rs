// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub use aast::*;
pub use aast_defs::*;
pub use ast_defs::*;
use oxidized::aast;
use oxidized::aast_defs;
use oxidized::ast_defs;

type Ex<'a> = (
    &'a oxidized_by_ref::pos::Pos<'a>,
    crate::typing_defs::Ty<'a>,
);

type En = crate::typing_defs::SavedEnv;

pub type Afield<'a> = aast::Afield<Ex<'a>, En>;
pub type AsExpr<'a> = aast::AsExpr<Ex<'a>, En>;
pub type Block<'a> = aast::Block<Ex<'a>, En>;
pub type Case<'a> = aast::Case<Ex<'a>, En>;
pub type Catch<'a> = aast::Catch<Ex<'a>, En>;
pub type Class_<'a> = aast::Class_<Ex<'a>, En>;
pub type ClassConst<'a> = aast::ClassConst<Ex<'a>, En>;
pub type ClassGetExpr<'a> = aast::ClassGetExpr<Ex<'a>, En>;
pub type ClassId<'a> = aast::ClassId<Ex<'a>, En>;
pub type ClassId_<'a> = aast::ClassId_<Ex<'a>, En>;
pub type ClassTypeconstDef<'a> = aast::ClassTypeconstDef<Ex<'a>, En>;
pub type ClassVar<'a> = aast::ClassVar<Ex<'a>, En>;
pub type CollectionTarg<'a> = aast::CollectionTarg<Ex<'a>>;
pub type Def<'a> = aast::Def<Ex<'a>, En>;
pub type Expr<'a> = aast::Expr<Ex<'a>, En>;
pub type Expr_<'a> = aast::Expr_<Ex<'a>, En>;
pub type Field<'a> = aast::Field<Ex<'a>, En>;
pub type FileAttribute<'a> = aast::FileAttribute<Ex<'a>, En>;
pub type Fun_<'a> = aast::Fun_<Ex<'a>, En>;
pub type FuncBody<'a> = aast::FuncBody<Ex<'a>, En>;
pub type FunDef<'a> = aast::FunDef<Ex<'a>, En>;
pub type FunParam<'a> = aast::FunParam<Ex<'a>, En>;
pub type Gconst<'a> = aast::Gconst<Ex<'a>, En>;
pub type Method_<'a> = aast::Method_<Ex<'a>, En>;
pub type Program<'a> = aast::Program<Ex<'a>, En>;
pub type Stmt<'a> = aast::Stmt<Ex<'a>, En>;
pub type Stmt_<'a> = aast::Stmt_<Ex<'a>, En>;
pub type Targ<'a> = aast::Targ<Ex<'a>>;
pub type Tparam<'a> = aast::Tparam<Ex<'a>, En>;
pub type Typedef<'a> = aast::Typedef<Ex<'a>, En>;
pub type TypeHint<'a> = aast::TypeHint<Ex<'a>>;
pub type UserAttribute<'a> = aast::UserAttribute<Ex<'a>, En>;
pub type UsingStmt<'a> = aast::UsingStmt<Ex<'a>, En>;
pub type XhpAttr<'a> = aast::XhpAttr<Ex<'a>, En>;
pub type XhpAttribute<'a> = aast::XhpAttribute<Ex<'a>, En>;
