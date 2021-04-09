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

pub trait Annot {
    type Ex;
    type Fb;
    type En;
    type Hi;
}

pub trait ClonableAnnot {
    type Ex: Clone;
    type Fb: Clone;
    type En: Clone;
    type Hi: Clone;
}

impl<CA: ClonableAnnot> Annot for CA {
    type Ex = <Self as ClonableAnnot>::Ex;
    type Fb = <Self as ClonableAnnot>::Fb;
    type En = <Self as ClonableAnnot>::En;
    type Hi = <Self as ClonableAnnot>::Hi;
}

pub struct AstAnnot;

impl ClonableAnnot for AstAnnot {
    type Ex = Pos;
    type Fb = ();
    type En = ();
    type Hi = ();
}

pub type Afield<A> =
    aast::Afield<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type AsExpr<A> =
    aast::AsExpr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Block<A> =
    aast::Block<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type CaField<A> =
    aast::CaField<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Case<A> =
    aast::Case<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Catch<A> =
    aast::Catch<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Class_<A> =
    aast::Class_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type ClassAttr<A> =
    aast::ClassAttr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type ClassConst<A> =
    aast::ClassConst<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type ClassGetExpr<A> =
    aast::ClassGetExpr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type ClassId<A> =
    aast::ClassId<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type ClassId_<A> =
    aast::ClassId_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type ClassTypeconstDef<A> =
    aast::ClassTypeconstDef<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type ClassVar<A> =
    aast::ClassVar<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type CollectionTarg<A> = aast::CollectionTarg<<A as Annot>::Hi>;
pub type Def<A> = aast::Def<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Expr<A> =
    aast::Expr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Expr_<A> =
    aast::Expr_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Field<A> =
    aast::Field<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type FileAttribute<A> =
    aast::FileAttribute<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Fun_<A> =
    aast::Fun_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type FuncBody<A> =
    aast::FuncBody<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type FunDef<A> =
    aast::FunDef<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type FunParam<A> =
    aast::FunParam<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type FunVariadicity<A> =
    aast::FunVariadicity<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Gconst<A> =
    aast::Gconst<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Method_<A> =
    aast::Method_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Program<A> =
    aast::Program<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type RecordDef<A> =
    aast::RecordDef<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Stmt<A> =
    aast::Stmt<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Stmt_<A> =
    aast::Stmt_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Targ<A> = aast::Targ<<A as Annot>::Hi>;
pub type Tparam<A> =
    aast::Tparam<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type Typedef<A> =
    aast::Typedef<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type TypeHint<A> = aast::TypeHint<<A as Annot>::Hi>;
pub type UserAttribute<A> =
    aast::UserAttribute<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type UsingStmt<A> =
    aast::UsingStmt<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type XhpAttr<A> =
    aast::XhpAttr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
pub type XhpAttribute<A> =
    aast::XhpAttribute<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En, <A as Annot>::Hi>;
