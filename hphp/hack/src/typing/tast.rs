// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::aast;
use oxidized::aast_defs;
use oxidized::ast_defs;

pub use aast::*;
pub use aast_defs::*;
pub use ast_defs::*;

type Ex<'a> = (
    &'a oxidized_by_ref::pos::Pos<'a>,
    crate::typing_defs::Ty<'a>,
);

type En = crate::typing_defs::SavedEnv;

type Hi<'a> = crate::typing_defs::Ty<'a>;

pub type Afield<'a> = aast::Afield<Ex<'a>, (), En, Hi<'a>>;
pub type AsExpr<'a> = aast::AsExpr<Ex<'a>, (), En, Hi<'a>>;
pub type Block<'a> = aast::Block<Ex<'a>, (), En, Hi<'a>>;
pub type CaField<'a> = aast::CaField<Ex<'a>, (), En, Hi<'a>>;
pub type Case<'a> = aast::Case<Ex<'a>, (), En, Hi<'a>>;
pub type Catch<'a> = aast::Catch<Ex<'a>, (), En, Hi<'a>>;
pub type Class_<'a> = aast::Class_<Ex<'a>, (), En, Hi<'a>>;
pub type ClassAttr<'a> = aast::ClassAttr<Ex<'a>, (), En, Hi<'a>>;
pub type ClassConst<'a> = aast::ClassConst<Ex<'a>, (), En, Hi<'a>>;
pub type ClassGetExpr<'a> = aast::ClassGetExpr<Ex<'a>, (), En, Hi<'a>>;
pub type ClassId<'a> = aast::ClassId<Ex<'a>, (), En, Hi<'a>>;
pub type ClassId_<'a> = aast::ClassId_<Ex<'a>, (), En, Hi<'a>>;
pub type ClassTypeconstDef<'a> = aast::ClassTypeconstDef<Ex<'a>, (), En, Hi<'a>>;
pub type ClassVar<'a> = aast::ClassVar<Ex<'a>, (), En, Hi<'a>>;
pub type CollectionTarg<'a> = aast::CollectionTarg<Hi<'a>>;
pub type Def<'a> = aast::Def<Ex<'a>, (), En, Hi<'a>>;
pub type Expr<'a> = aast::Expr<Ex<'a>, (), En, Hi<'a>>;
pub type Expr_<'a> = aast::Expr_<Ex<'a>, (), En, Hi<'a>>;
pub type Field<'a> = aast::Field<Ex<'a>, (), En, Hi<'a>>;
pub type FileAttribute<'a> = aast::FileAttribute<Ex<'a>, (), En, Hi<'a>>;
pub type Fun_<'a> = aast::Fun_<Ex<'a>, (), En, Hi<'a>>;
pub type FuncBody<'a> = aast::FuncBody<Ex<'a>, (), En, Hi<'a>>;
pub type FunDef<'a> = aast::FunDef<Ex<'a>, (), En, Hi<'a>>;
pub type FunParam<'a> = aast::FunParam<Ex<'a>, (), En, Hi<'a>>;
pub type FunVariadicity<'a> = aast::FunVariadicity<Ex<'a>, (), En, Hi<'a>>;
pub type Gconst<'a> = aast::Gconst<Ex<'a>, (), En, Hi<'a>>;
pub type Method_<'a> = aast::Method_<Ex<'a>, (), En, Hi<'a>>;
pub type Program<'a> = aast::Program<Ex<'a>, (), En, Hi<'a>>;
pub type RecordDef<'a> = aast::RecordDef<Ex<'a>, (), En, Hi<'a>>;
pub type Stmt<'a> = aast::Stmt<Ex<'a>, (), En, Hi<'a>>;
pub type Stmt_<'a> = aast::Stmt_<Ex<'a>, (), En, Hi<'a>>;
pub type Targ<'a> = aast::Targ<Hi<'a>>;
pub type Tparam<'a> = aast::Tparam<Ex<'a>, (), En, Hi<'a>>;
pub type Typedef<'a> = aast::Typedef<Ex<'a>, (), En, Hi<'a>>;
pub type TypeHint<'a> = aast::TypeHint<Hi<'a>>;
pub type UserAttribute<'a> = aast::UserAttribute<Ex<'a>, (), En, Hi<'a>>;
pub type UsingStmt<'a> = aast::UsingStmt<Ex<'a>, (), En, Hi<'a>>;
pub type XhpAttr<'a> = aast::XhpAttr<Ex<'a>, (), En, Hi<'a>>;
pub type XhpAttribute<'a> = aast::XhpAttribute<Ex<'a>, (), En, Hi<'a>>;
