// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::VecDeque;
use std::ops::ControlFlow;

use oxidized::aast_defs::Block;
use oxidized::aast_defs::Stmt;
use oxidized::aast_defs::Stmt_;
use oxidized::aast_defs::UsingStmt;
use oxidized::naming_phase_error::NamingPhaseError;
use transform::Pass;

use crate::context::Context;

pub struct ElabBlockPass;

impl Pass for ElabBlockPass {
    type Ctx = Context;
    type Err = NamingPhaseError;

    fn on_ty_block<Ex, En>(
        &self,
        elem: &mut Block<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        let mut q: VecDeque<_> = elem.drain(0..).collect();
        while let Some(Stmt(pos, stmt_)) = q.pop_front() {
            match stmt_ {
                Stmt_::Block(xs) => xs.into_iter().rev().for_each(|x| q.push_front(x)),
                _ => elem.push(Stmt(pos, stmt_)),
            }
        }
        ControlFlow::Continue(())
    }

    fn on_ty_using_stmt<Ex, En>(
        &self,
        elem: &mut UsingStmt<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        elem.is_block_scoped = false;
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use oxidized::aast_defs::Block;
    use oxidized::aast_defs::Stmt;
    use oxidized::aast_defs::Stmt_;
    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::tast::Pos;
    use transform::Pass;

    use super::*;

    pub struct Identity;
    impl Pass for Identity {
        type Err = NamingPhaseError;
        type Ctx = Context;
    }

    #[test]
    fn test() {
        let mut ctx = Context::default();
        let mut errs = Vec::default();
        let top_down = ElabBlockPass;
        let bottom_up = Identity;

        let mut elem: Block<(), ()> = vec![Stmt(
            Pos::make_none(),
            Stmt_::Block(vec![
                Stmt(Pos::make_none(), Stmt_::Noop),
                Stmt(
                    Pos::make_none(),
                    Stmt_::Block(vec![
                        Stmt(Pos::make_none(), Stmt_::Noop),
                        Stmt(
                            Pos::make_none(),
                            Stmt_::Block(vec![
                                Stmt(Pos::make_none(), Stmt_::Noop),
                                Stmt(
                                    Pos::make_none(),
                                    Stmt_::Block(vec![
                                        Stmt(Pos::make_none(), Stmt_::Noop),
                                        Stmt(
                                            Pos::make_none(),
                                            Stmt_::Block(vec![Stmt(Pos::make_none(), Stmt_::Noop)]),
                                        ),
                                        Stmt(Pos::make_none(), Stmt_::Noop),
                                    ]),
                                ),
                                Stmt(Pos::make_none(), Stmt_::Noop),
                            ]),
                        ),
                        Stmt(Pos::make_none(), Stmt_::Noop),
                    ]),
                ),
                Stmt(Pos::make_none(), Stmt_::Noop),
            ]),
        )];
        transform::transform_ty_block(&mut elem, &mut ctx, &mut errs, &top_down, &bottom_up);

        assert_eq!(elem.len(), 9);
        assert!(elem.into_iter().all(|s| matches!(s.1, Stmt_::Noop)));
    }
}
