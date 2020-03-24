// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use decl_provider_rust::DeclProvider;
use oxidized::pos::Pos;
use oxidized::{relative_path, typechecker_options};

pub use typing_collections_rust::*;
pub use typing_defs_rust::typing_logic::{SubtypeProp, *};
pub use typing_defs_rust::{Ty, *};

use crate::typing_env_return_info;
use crate::typing_inference_env::InferenceEnv;
use crate::typing_make_type::TypeBuilder;

pub struct Env<'a> {
    pub function_pos: &'a Pos,
    pub fresh_typarams: SSet<'a>,
    pub genv: Genv<'a>,
    // pub decl_env: &'a decl_env::Env,
    pub log_levels: SMap<'a, isize>,
    pub inference_env: InferenceEnv<'a>,
}

pub struct Genv<'a> {
    pub tcopt: typechecker_options::TypecheckerOptions,
    pub return_info: typing_env_return_info::TypingEnvReturnInfo<'a>,
    pub params: LocalIdMap<'a, (Ty<'a>, ParamMode)>,
    pub file: relative_path::RelativePath,
    pub builder: &'a TypeBuilder<'a>,
    pub provider: &'a dyn DeclProvider,
}

impl<'a> Env<'a> {
    pub fn new(function_pos: &'a Pos, genv: Genv<'a>) -> Self {
        Env {
            function_pos,
            fresh_typarams: SSet::empty(),
            inference_env: InferenceEnv::new(genv.builder),
            genv,
            log_levels: SMap::empty(),
        }
    }

    pub fn builder(&self) -> &'a TypeBuilder<'a> {
        self.genv.builder
    }

    pub fn provider(&self) -> &'a dyn DeclProvider {
        self.genv.provider
    }
}
