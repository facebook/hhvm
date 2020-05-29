// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_env::{empty_global_env, Env};
use crate::typing_phase;
use arena_trait::Arena;
use decl_provider_rust as decl_provider;
use oxidized::ast;
use oxidized_by_ref::pos::Pos;
use typing_defs_rust::typing_make_type::TypeBuilder;
use typing_defs_rust::typing_reason::*;
use typing_defs_rust::Ty_;

// Given a decl provider, a type builder, and the AST for a function definition,
// produce the initial environment for checking a top-level function
pub fn fun_env<'a>(
    builder: &'a TypeBuilder<'a>,
    provider: &'a dyn decl_provider::DeclProvider,
    f: &'a ast::FunDef,
) -> Env<'a> {
    let genv = empty_global_env(builder, provider, f.name.0.filename().clone());
    let mut env = Env::new(&f.name.0, genv);
    // Terrible hacky conversion from hint, via provider!
    let ty = match provider.get_fun(&f.name.1) {
        None => unimplemented!(),
        Some(f) => {
            let fty = f.type_;
            let ety_env = builder.env_with_self();
            let fty = typing_phase::localize(ety_env, &mut env, fty);
            match fty.get_node() {
                Ty_::Tfun(x) => x.ret.type_,
                x => unimplemented!("{:#?}", x),
            }
        }
    };
    env.set_return_type(ty);
    env
}

pub fn stmt_env<'a>(
    builder: &'a TypeBuilder<'a>,
    provider: &'a dyn decl_provider::DeclProvider,
    s: &'a ast::Stmt,
) -> Env<'a> {
    let r = builder.alloc(Reason::hint(Pos::from_oxidized_in(&s.0, builder.bumpalo())));
    let rty = builder.void(r);
    let genv = empty_global_env(builder, provider, s.0.filename().clone());
    let mut env = Env::new(&s.0, genv);
    env.set_return_type(rty);
    env
}
