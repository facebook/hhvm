// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{fun_env, stmt_env};
use decl_provider_rust as decl_provider;
use oxidized::ast;
use typing_defs_rust::tast;
use typing_defs_rust::typing_make_type::TypeBuilder;

mod typing;
mod typing_env;
mod typing_env_from_def;
mod typing_env_return_info;
mod typing_env_types;
mod typing_phase;
pub mod typing_print;
mod typing_subtype;

pub use typing_env::*;
pub use typing_env_from_def::*;
pub use typing_env_types::*;

pub fn program<'a>(
    builder: &'a TypeBuilder<'a>,
    provider: &'a dyn decl_provider::DeclProvider,
    ast: &'a ast::Program,
) -> tast::Program<'a> {
    ast.iter()
        .filter_map(|x| def(builder, provider, x))
        .collect()
}

fn def<'a>(
    builder: &'a TypeBuilder<'a>,
    provider: &'a dyn decl_provider::DeclProvider,
    def: &'a ast::Def,
) -> Option<tast::Def<'a>> {
    match def {
        ast::Def::Fun(x) => {
            let mut env = fun_env(builder, provider, x);
            Some(tast::Def::mk_fun(typing::fun(&mut env, x)))
        }
        ast::Def::Stmt(x) => {
            let mut env = stmt_env(builder, provider, x);
            Some(tast::Def::mk_stmt(typing::stmt(&mut env, x)))
        }
        ast::Def::Class(_) => None,
        _ => unimplemented!(),
    }
}
