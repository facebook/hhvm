// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hcons::Hc;

use std::convert::From;

use crate::pos::{PosId, Symbol};
use crate::reason::Reason;

pub type Prim = oxidized::aast::Tprim;
pub type Abstraction = oxidized::ast_defs::Abstraction; // `Concrete` or `Abstract`.
pub type Variance = oxidized::ast_defs::Variance;

// note(sf, 2022-01-31): c.f. oxidized_by_ref::ast_defs::ClassishKind<'_>
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum ClassishKind {
    Class(Abstraction),
    Interface,
    Trait,
    Enum,
    EnumClass(Abstraction),
}

impl From<oxidized_by_ref::ast_defs::ClassishKind<'_>> for ClassishKind {
    fn from(kind: oxidized_by_ref::ast_defs::ClassishKind<'_>) -> Self {
        use oxidized_by_ref::ast_defs;
        match kind {
            ast_defs::ClassishKind::Cclass(a) => ClassishKind::Class(*a),
            ast_defs::ClassishKind::Cinterface => ClassishKind::Interface,
            ast_defs::ClassishKind::Ctrait => ClassishKind::Trait,
            ast_defs::ClassishKind::Cenum => ClassishKind::Enum,
            ast_defs::ClassishKind::CenumClass(a) => ClassishKind::EnumClass(*a),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunParam<R: Reason, TY> {
    pub pos: R::Pos,
    pub name: Option<Symbol>,
    pub ty: TY,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunType<R: Reason, TY> {
    pub params: Vec<FunParam<R, TY>>,
    pub ret: TY,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum DeclTy_<R: Reason, TY> {
    /// A primitive type.
    DTprim(Prim),
    /// Either an object type or a type alias, ty list are the arguments.
    DTapply(PosId<R::Pos>, Vec<TY>),
    /// A wrapper around `FunType`, which contains the full type information
    /// for a function, method, lambda, etc.
    DTfun(FunType<R, TY>),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct DeclTy<R: Reason>(R, Hc<DeclTy_<R, DeclTy<R>>>);

impl<R: Reason> DeclTy<R> {
    pub fn new(reason: R, ty: Hc<DeclTy_<R, DeclTy<R>>>) -> Self {
        Self(reason, ty)
    }

    pub fn pos(&self) -> &R::Pos {
        self.0.pos()
    }

    pub fn reason(&self) -> &R {
        &self.0
    }

    pub fn node(&self) -> &Hc<DeclTy_<R, DeclTy<R>>> {
        &self.1
    }

    pub fn unwrap_class_type(&self) -> Option<(&R, &PosId<R::Pos>, &[DeclTy<R>])> {
        use DeclTy_::*;
        let r = self.reason();
        match &**self.node() {
            DTapply(pos_id, tyl) => Some((r, pos_id, tyl)),
            _ => None,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum CeVisibility<R: Reason> {
    Public,
    Private(Symbol),
    Protected(Symbol),
    Internal(PosId<R::Pos>),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct UserAttribute<R: Reason> {
    pub name: PosId<R::Pos>,
    pub classname_params: Vec<Symbol>,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Tparam<R: Reason> {
    pub variance: Variance,
    pub name: PosId<R::Pos>,
    pub tparams: Vec<Tparam<R>>,
    pub constraints: Vec<(oxidized::ast_defs::ConstraintKind, DeclTy<R>)>,
    pub reified: oxidized::aast::ReifyKind,
    pub user_attributes: Vec<UserAttribute<R>>,
}
