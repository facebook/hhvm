// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(is_some_and)]

/// Used to combine multiple types implementing `Pass` into nested `Passes` types
/// without requiring them to hand write it so :
/// `passes![p1, p2, p3]` => `Passes(p1, Passes(p2, p3))`
macro_rules! passes {
    ( $p:expr $(,$ps:expr)+ $(,)? ) => {
        $crate::pass::Passes { fst: $p, snd: passes!($($ps),*) }
    };
    ( $p:expr $(,)? ) => {
        $p
    };
}

mod config;
mod pass;
mod passes;
mod transform;

use oxidized::ast;
use oxidized::naming_phase_error::NamingPhaseError;
use pass::Pass;
use transform::Transform;

pub fn elaborate_program(program: &mut ast::Program) -> Vec<NamingPhaseError> {
    #[derive(Clone, Default)]
    struct NoPass;
    impl Pass for NoPass {
        type Cfg = config::Config;
        type Err = NamingPhaseError;
    }
    let mut errs = Vec::default();
    program.transform(&Default::default(), &mut errs, &mut passes![NoPass, NoPass]);
    errs
}
