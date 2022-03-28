// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::reason::Reason;
use crate::tast;
use crate::typing::typing_block::TCBlock;
use crate::typing::typing_env::TEnv;
use crate::typing::typing_error::Result;
use crate::typing::typing_trait::TC;

impl<'a, R: Reason> TC<R> for oxidized::aast::FuncBody<(), ()> {
    type Params = ();
    type Typed = tast::FuncBody<R>;

    fn infer(&self, env: &TEnv<R>, _params: ()) -> Result<Self::Typed> {
        let fb_ast = TCBlock(&self.fb_ast).infer(env, ())?;
        Ok(oxidized::aast::FuncBody { fb_ast })
    }
}
