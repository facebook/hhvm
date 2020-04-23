// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use arena_trait::Arena;

use decl_provider_rust::DeclProvider;
use oxidized::pos::Pos;
use oxidized::{local_id, relative_path, typechecker_options};

use crate::typing_env_return_info;
use crate::typing_make_type::TypeBuilder;
use crate::typing_per_cont_env::{PerContEntry, TypingPerContEnv};
use typing_ast_rust::typing_inference_env::InferenceEnv;
pub use typing_collections_rust::*;
pub use typing_defs_rust::typing_logic::{SubtypeProp, *};
pub use typing_defs_rust::{Ty, *};

pub struct Env<'a> {
    pub ident_counter: isize,
    pub function_pos: &'a Pos,
    pub fresh_typarams: SSet<'a>,
    pub lenv: LocalEnv<'a>,
    pub genv: Genv<'a>,
    // pub decl_env: &'a decl_env::Env,
    pub log_levels: SMap<'a, isize>,
    pub inference_env: InferenceEnv<'a>,
}

impl<'a> Env<'a> {
    pub fn new(function_pos: &'a Pos, genv: Genv<'a>) -> Self {
        Env {
            ident_counter: 0,
            function_pos,
            fresh_typarams: SSet::empty(),
            inference_env: InferenceEnv::new(genv.builder),
            lenv: LocalEnv::initial_local(genv.builder),
            genv,
            log_levels: SMap::empty(),
        }
    }

    pub fn builder(&self) -> &'a TypeBuilder<'a> {
        self.genv.builder
    }

    pub fn bld(&self) -> &'a TypeBuilder<'a> {
        self.builder()
    }

    pub fn provider(&self) -> &'a dyn DeclProvider {
        self.genv.provider
    }

    pub fn set_function_pos(&mut self, pos: &'a Pos) {
        self.function_pos = pos;
    }

    pub fn set_return_type(&mut self, ty: Ty<'a>) {
        self.genv.return_info.type_.type_ = ty;
    }
}

pub struct Genv<'a> {
    pub tcopt: typechecker_options::TypecheckerOptions,
    pub return_info: typing_env_return_info::TypingEnvReturnInfo<'a>,
    pub params: LocalIdMap<'a, (Ty<'a>, ParamMode)>,
    pub file: relative_path::RelativePath,
    pub builder: &'a TypeBuilder<'a>,
    pub provider: &'a dyn DeclProvider,
}

#[derive(Debug)]
pub struct LocalEnv<'a> {
    pub per_cont_env: TypingPerContEnv<'a>,
    // TODO(hrust): mutability
    // TODO(hrust): reactivity
    // TODO(hrust): using vars
}

impl<'a> LocalEnv<'a> {
    pub fn initial_local<A: Arena>(arena: &'a A) -> Self {
        LocalEnv {
            per_cont_env: TypingPerContEnv::initial_locals(arena, PerContEntry::empty()),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct LocalId<'a>(pub isize, pub &'a str);

impl<'a> LocalId<'a> {
    pub fn make_unscoped(name: &'a str) -> LocalId<'a> {
        LocalId(0, name)
    }

    pub fn name(&self) -> &'a str {
        self.1
    }
}

impl<'a> From<&'a local_id::LocalId> for LocalId<'a> {
    fn from(x: &'a local_id::LocalId) -> LocalId<'a> {
        let (a, b) = x;
        LocalId(*a, b.as_str())
    }
}

pub type LocalIdMap<'a, V> = Map<'a, LocalId<'a>, V>;
