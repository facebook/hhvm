// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_block::TCBlock;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::shared::typing_return::TypingReturn;
use crate::typing::typing_error::Result;

impl<'a, R: Reason> Infer<R> for oxidized::aast::FuncBody<(), ()> {
    type Params = ();
    type Typed = tast::FuncBody<R>;

    fn infer(&self, env: &mut TEnv<R>, _params: ()) -> Result<Self::Typed> {
        let ret = env.get_return();
        let params = env.get_params();
        let res = infer_func_body(env, self);
        env.set_params(params);
        env.set_return(ret);
        res
    }
}

fn infer_func_body<R: Reason>(
    env: &mut TEnv<R>,
    func_body: &oxidized::aast::FuncBody<(), ()>,
) -> Result<tast::FuncBody<R>> {
    rupro_todo_mark!(Hhi);
    rupro_todo_mark!(SavedEnv);
    rupro_todo_mark!(DisableTypecheckerInternalAttribute);
    rupro_todo_mark!(Abstract);

    let fb_ast = TCBlock(&func_body.fb_ast).infer(env, ())?;

    let has_implicit_return = env.has_next();
    if has_implicit_return {
        TypingReturn::fun_implicit_return(env);
    }

    Ok(oxidized::aast::FuncBody { fb_ast })
}
