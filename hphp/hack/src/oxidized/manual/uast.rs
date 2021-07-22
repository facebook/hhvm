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

pub trait Annot {
    type Ex;
    type Fb;
    type En;
}

pub trait ClonableAnnot {
    type Ex: Clone;
    type Fb: Clone;
    type En: Clone;
}

impl<CA: ClonableAnnot> Annot for CA {
    type Ex = <Self as ClonableAnnot>::Ex;
    type Fb = <Self as ClonableAnnot>::Fb;
    type En = <Self as ClonableAnnot>::En;
}

pub struct AstAnnot;

impl ClonableAnnot for AstAnnot {
    type Ex = ();
    type Fb = ();
    type En = ();
}

pub type Afield<A> = aast::Afield<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type AsExpr<A> = aast::AsExpr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Block<A> = aast::Block<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type CaField<A> = aast::CaField<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Case<A> = aast::Case<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Catch<A> = aast::Catch<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Class_<A> = aast::Class_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type ClassAttr<A> = aast::ClassAttr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type ClassConst<A> = aast::ClassConst<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type ClassGetExpr<A> = aast::ClassGetExpr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type ClassId<A> = aast::ClassId<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type ClassId_<A> = aast::ClassId_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type ClassTypeconstDef<A> =
    aast::ClassTypeconstDef<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type ClassVar<A> = aast::ClassVar<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type CollectionTarg<A> = aast::CollectionTarg<A>;
pub type Def<A> = aast::Def<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Expr<A> = aast::Expr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Expr_<A> = aast::Expr_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Field<A> = aast::Field<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type FileAttribute<A> =
    aast::FileAttribute<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Fun_<A> = aast::Fun_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type FuncBody<A> = aast::FuncBody<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type FunDef<A> = aast::FunDef<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type FunParam<A> = aast::FunParam<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type FunVariadicity<A> =
    aast::FunVariadicity<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Gconst<A> = aast::Gconst<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Method_<A> = aast::Method_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Program<A> = aast::Program<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type RecordDef<A> = aast::RecordDef<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Stmt<A> = aast::Stmt<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Stmt_<A> = aast::Stmt_<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Targ<A> = aast::Targ<A>;
pub type Tparam<A> = aast::Tparam<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type Typedef<A> = aast::Typedef<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type TypeHint<A> = aast::TypeHint<A>;
pub type UserAttribute<A> =
    aast::UserAttribute<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type UsingStmt<A> = aast::UsingStmt<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type XhpAttr<A> = aast::XhpAttr<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
pub type XhpAttribute<A> = aast::XhpAttribute<<A as Annot>::Ex, <A as Annot>::Fb, <A as Annot>::En>;
