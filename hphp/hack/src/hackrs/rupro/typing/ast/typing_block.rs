// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;

/// We already have a blanket implementation for [T], however,
/// for `aast::Block` (which is a plain type alias for `Vec<aast::Stmt>`)
/// we want to flatten while type checking.
///
/// To disambiguate between the blanket implementation we provide a wrapper
/// type.
pub struct TCBlock<'a>(pub &'a oxidized::aast::Block<(), ()>);

impl<'a, R: Reason> Infer<R> for TCBlock<'a> {
    type Params = ();
    type Typed = tast::Block<R>;

    fn infer(&self, env: &mut TEnv<R>, _params: ()) -> Result<Self::Typed> {
        // We eliminate nested blocks.
        use oxidized::aast::Stmt_::*;
        let mut typed_stmts = Vec::new();
        for stmt in self.0 {
            let typed_stmt = stmt.infer(env, ())?;
            match typed_stmt.1 {
                Block(nested_typed_stmts) => typed_stmts.extend(nested_typed_stmts),
                _ => typed_stmts.push(typed_stmt),
            }
        }
        Ok(typed_stmts)
    }
}
