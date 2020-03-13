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

type Ex<'a> = crate::typing_defs::Ty<'a>;

// TODO(hrust) this is func_body_ann<'a> = HasUnsafeBlocks | NoUnsafeBlocks in OCaml
type Fb = ();

// TODO(hrust): this is saved_env in OCaml
type En = ();

type Hi = ();

pub type Afield<'a> = aast::Afield<Ex<'a>, Fb, En, Hi>;
pub type AsExpr<'a> = aast::AsExpr<Ex<'a>, Fb, En, Hi>;
pub type AssertExpr<'a> = aast::AssertExpr<Ex<'a>, Fb, En, Hi>;
pub type Block<'a> = aast::Block<Ex<'a>, Fb, En, Hi>;
pub type CaField<'a> = aast::CaField<Ex<'a>, Fb, En, Hi>;
pub type Case<'a> = aast::Case<Ex<'a>, Fb, En, Hi>;
pub type Catch<'a> = aast::Catch<Ex<'a>, Fb, En, Hi>;
pub type Class_<'a> = aast::Class_<Ex<'a>, Fb, En, Hi>;
pub type ClassAttr<'a> = aast::ClassAttr<Ex<'a>, Fb, En, Hi>;
pub type ClassConst<'a> = aast::ClassConst<Ex<'a>, Fb, En, Hi>;
pub type ClassGetExpr<'a> = aast::ClassGetExpr<Ex<'a>, Fb, En, Hi>;
pub type ClassId<'a> = aast::ClassId<Ex<'a>, Fb, En, Hi>;
pub type ClassId_<'a> = aast::ClassId_<Ex<'a>, Fb, En, Hi>;
pub type ClassTparams<'a> = aast::ClassTparams<Ex<'a>, Fb, En, Hi>;
pub type ClassTypeconst<'a> = aast::ClassTypeconst<Ex<'a>, Fb, En, Hi>;
pub type ClassVar<'a> = aast::ClassVar<Ex<'a>, Fb, En, Hi>;
pub type CollectionTarg<'a> = aast::CollectionTarg<Hi>;
pub type Def<'a> = aast::Def<Ex<'a>, Fb, En, Hi>;
pub type Expr<'a> = aast::Expr<Ex<'a>, Fb, En, Hi>;
pub type Expr_<'a> = aast::Expr_<Ex<'a>, Fb, En, Hi>;
pub type Field<'a> = aast::Field<Ex<'a>, Fb, En, Hi>;
pub type FileAttribute<'a> = aast::FileAttribute<Ex<'a>, Fb, En, Hi>;
pub type Fun_<'a> = aast::Fun_<Ex<'a>, Fb, En, Hi>;
pub type FuncBody<'a> = aast::FuncBody<Ex<'a>, Fb, En, Hi>;
pub type FunDef<'a> = aast::FunDef<Ex<'a>, Fb, En, Hi>;
pub type FunParam<'a> = aast::FunParam<Ex<'a>, Fb, En, Hi>;
pub type FunVariadicity<'a> = aast::FunVariadicity<Ex<'a>, Fb, En, Hi>;
pub type Gconst<'a> = aast::Gconst<Ex<'a>, Fb, En, Hi>;
pub type Method_<'a> = aast::Method_<Ex<'a>, Fb, En, Hi>;
pub type MethodRedeclaration<'a> = aast::MethodRedeclaration<Ex<'a>, Fb, En, Hi>;
pub type Program<'a> = aast::Program<Ex<'a>, Fb, En, Hi>;
pub type PuEnum<'a> = aast::PuEnum<Ex<'a>, Fb, En, Hi>;
pub type PuMember<'a> = aast::PuMember<Ex<'a>, Fb, En, Hi>;
pub type RecordDef<'a> = aast::RecordDef<Ex<'a>, Fb, En, Hi>;
pub type Stmt<'a> = aast::Stmt<Ex<'a>, Fb, En, Hi>;
pub type Stmt_<'a> = aast::Stmt_<Ex<'a>, Fb, En, Hi>;
pub type Targ<'a> = aast::Targ<Hi>;
pub type Tparam<'a> = aast::Tparam<Ex<'a>, Fb, En, Hi>;
pub type Typedef<'a> = aast::Typedef<Ex<'a>, Fb, En, Hi>;
pub type TypeHint<'a> = aast::TypeHint<Hi>;
pub type UserAttribute<'a> = aast::UserAttribute<Ex<'a>, Fb, En, Hi>;
pub type UsingStmt<'a> = aast::UsingStmt<Ex<'a>, Fb, En, Hi>;
pub type XhpAttr<'a> = aast::XhpAttr<Ex<'a>, Fb, En, Hi>;
pub type XhpAttribute<'a> = aast::XhpAttribute<Ex<'a>, Fb, En, Hi>;
