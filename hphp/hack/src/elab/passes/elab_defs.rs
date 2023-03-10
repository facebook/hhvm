// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::VecDeque;
use std::ops::ControlFlow;

use oxidized::nast::Def;
use oxidized::nast::Program;
use oxidized::nast::Stmt;
use oxidized::nast::Stmt_;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabDefsPass;

impl Pass for ElabDefsPass {
    fn on_ty_program_top_down(&mut self, elem: &mut Program, _env: &Env) -> ControlFlow<()> {
        let Program(defs) = elem;
        let mut q: VecDeque<_> = defs.drain(0..).collect();
        while let Some(e) = q.pop_front() {
            match e {
                // Flatten nested namespaces; prepend the elements onto our
                // queue and carry on
                Def::Namespace(ns) => {
                    let (_, ns_defs) = *ns;
                    ns_defs.into_iter().rev().for_each(|x| q.push_front(x))
                }
                // Remove the following top-level definitions
                Def::FileAttributes(_) | Def::NamespaceUse(_) | Def::SetNamespaceEnv(_) => (),
                // Retain the following top-level definitions
                Def::Fun(_)
                | Def::Class(_)
                | Def::Typedef(_)
                | Def::Constant(_)
                | Def::Module(_)
                | Def::SetModule(_) => defs.push(e),
                // Retain all non [Noop] and [Markup] top-level statements
                // note that these statements may still appear in non-top-level
                // positions
                Def::Stmt(ref stmt) => {
                    let Stmt(_, stmt_) = &**stmt;
                    match &stmt_ {
                        Stmt_::Noop => (),
                        Stmt_::Markup(_) => (),
                        _ => defs.push(e),
                    }
                }
            }
        }
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use oxidized::nast::Def;
    use oxidized::nast::Id;
    use oxidized::nast::Pos;
    use oxidized::nast::Program;
    use oxidized::nast::Stmt;
    use oxidized::nast::Stmt_;

    use super::*;
    use crate::Transform;

    #[test]
    fn test() {
        let env = Env::default();

        let mut elem = Program(vec![
            Def::Stmt(Box::new(Stmt(Pos::NONE, Stmt_::Break))),
            Def::NamespaceUse(Vec::default()),
            Def::Stmt(Box::new(Stmt(Pos::NONE, Stmt_::Noop))),
            Def::Namespace(Box::new((
                Id(Pos::NONE, String::default()),
                vec![
                    Def::NamespaceUse(Vec::default()),
                    Def::Stmt(Box::new(Stmt(Pos::NONE, Stmt_::Fallthrough))),
                    Def::Stmt(Box::new(Stmt(Pos::NONE, Stmt_::Noop))),
                    Def::Namespace(Box::new((
                        Id(Pos::NONE, String::default()),
                        vec![
                            Def::Stmt(Box::new(Stmt(Pos::NONE, Stmt_::Break))),
                            Def::NamespaceUse(Vec::default()),
                            Def::Stmt(Box::new(Stmt(Pos::NONE, Stmt_::Noop))),
                        ],
                    ))),
                ],
            ))),
            Def::Stmt(Box::new(Stmt(Pos::NONE, Stmt_::Fallthrough))),
            Def::NamespaceUse(Vec::default()),
            Def::Stmt(Box::new(Stmt(Pos::NONE, Stmt_::Noop))),
        ]);

        let mut pass = ElabDefsPass;

        elem.transform(&env, &mut pass);

        // Given our initial program:
        //
        // [ Break
        // ; NamespaceUse(..)
        // ; Noop
        // ; Namespace(..,
        //     [ NamespaceUse(..)
        //     ; Fallthrough
        //     ; Noop
        //     ; Namespace(..,
        //         [ Break
        //         ; NamespaceUse(..)
        //         ; Noop
        //         ]
        //       )
        //     ]
        //   )
        // ; Fallthrough
        // ; NamespaceUse(..)
        // ; Noop
        // ]
        //
        // We expect the transformed program:
        //
        // [ Break
        // ; Fallthrough
        // ; Break
        // ; Fallthrough
        // ]

        assert_eq!(elem.0.len(), 4);

        let mut q = VecDeque::from(elem.0);

        // First def is Break
        assert!(q.pop_front().is_some_and(|def| match def {
            Def::Stmt(stmt) => match &stmt.1 {
                Stmt_::Break => true,
                _ => false,
            },
            _ => false,
        }));

        // Second def is Fallthrough
        assert!(q.pop_front().is_some_and(|def| match def {
            Def::Stmt(stmt) => match &stmt.1 {
                Stmt_::Fallthrough => true,
                _ => false,
            },
            _ => false,
        }));

        // Third def is Break
        assert!(q.pop_front().is_some_and(|def| match def {
            Def::Stmt(stmt) => match &stmt.1 {
                Stmt_::Break => true,
                _ => false,
            },
            _ => false,
        }));

        // Last def is Fallthrough
        assert!(q.pop_front().is_some_and(|def| match def {
            Def::Stmt(stmt) => match &stmt.1 {
                Stmt_::Fallthrough => true,
                _ => false,
            },
            _ => false,
        }));
    }
}
