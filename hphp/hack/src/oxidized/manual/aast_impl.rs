// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast::*;
use crate::pos::Pos;
use std::boxed::Box;

impl<Ex, Fb, En, Hi> Stmt<Ex, Fb, En, Hi> {
    pub fn new(pos: Pos, s: Stmt_<Ex, Fb, En, Hi>) -> Self {
        Self(pos, Box::new(s))
    }

    pub fn noop(pos: Pos) -> Self {
        Self::new(pos, Stmt_::Noop)
    }
}

impl<Ex, Fb, En, Hi> Expr<Ex, Fb, En, Hi> {
    pub fn new(ex: Ex, e: Expr_<Ex, Fb, En, Hi>) -> Self {
        Self(ex, Box::new(e))
    }
}
