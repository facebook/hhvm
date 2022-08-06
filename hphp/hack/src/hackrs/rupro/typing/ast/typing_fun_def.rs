// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;

impl<R: Reason> Infer<R> for oxidized::aast::FunDef<(), ()> {
    type Params = ();
    type Typed = tast::FunDef<R>;

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
    fd: &oxidized::aast::FunDef<(), ()>,
) -> Result<tast::FunDef<R>> {
    rupro_todo_assert!(fd.file_attributes.is_empty(), AST);
    rupro_todo_mark!(Modules);
    let fun = fd.fun.infer(env, ())?;
    let res = oxidized::aast::FunDef {
        namespace: fd.namespace.clone(),
        file_attributes: vec![],
        mode: fd.mode,
        fun,
        // TODO(T116039119): Populate value with presence of internal attribute
        internal: false,
        module: None,
    };
    Ok(res)
}
