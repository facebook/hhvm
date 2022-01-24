// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused)]
use std::ops::Deref;
use std::rc::Rc;

use crate::hcons::Consed;
use crate::pos::{PosId, Symbol};
use crate::reason::Reason;

pub type Prim = oxidized::aast::Tprim;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunParam<REASON: Reason, TY> {
    pub fp_pos: REASON::P,
    pub fp_name: Option<Symbol>,
    pub fp_type: TY,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunType<REASON: Reason, TY> {
    pub ft_params: Vec<FunParam<REASON, TY>>,
    pub ft_ret: TY,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum DeclTy_<REASON: Reason, TY> {
    /// A primitive type.
    DTprim(Prim),
    /// Either an object type or a type alias, ty list are the arguments.
    DTapply(PosId<REASON::P>, Vec<TY>),
    /// A wrapper around `FunType`, which contains the full type information
    /// for a function, method, lambda, etc.
    DTfun(FunType<REASON, TY>),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct DeclTy<REASON: Reason>(REASON, Consed<DeclTy_<REASON, DeclTy<REASON>>>);

impl<REASON: Reason> DeclTy<REASON> {
    pub fn new(reason: REASON, consed: Consed<DeclTy_<REASON, DeclTy<REASON>>>) -> Self {
        Self(reason, consed)
    }

    pub fn pos(&self) -> &REASON::P {
        self.0.pos()
    }

    pub fn reason(&self) -> &REASON {
        &self.0
    }

    pub fn node(&self) -> &Consed<DeclTy_<REASON, DeclTy<REASON>>> {
        &self.1
    }

    pub fn unwrap_class_type(&self) -> Option<(&REASON, &PosId<REASON::P>, &[DeclTy<REASON>])> {
        use DeclTy_::*;
        let r = self.reason();
        match &**self.node() {
            DTapply(pos_id, tyl) => Some((r, pos_id, tyl)),
            _ => None,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Visibility<R: Reason> {
    Public,
    Private(Symbol),
    Protected(Symbol),
    Internal(PosId<R::P>),
}
