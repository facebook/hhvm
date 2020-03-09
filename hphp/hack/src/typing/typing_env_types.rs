// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::*;
pub use typing_defs_rust::{Ty, *};

use crate::typing_env_return_info;
use crate::typing_inference_env;

pub struct Env<'a> {
    pub function_pos: &'a pos::Pos,
    pub fresh_typarams: s_set::SSet,
    pub genv: Genv<'a>,
    // pub decl_env: &'a decl_env::Env,
    pub log_levels: s_map::SMap<isize>,
    pub inference_env: typing_inference_env::TypingInferenceEnv<'a>,
}

pub struct Genv<'a> {
    pub tcopt: typechecker_options::TypecheckerOptions,
    pub return_: typing_env_return_info::TypingEnvReturnInfo<'a>,
    pub params: local_id::map::Map<(Ty<'a>, ParamMode)>,
    pub file: relative_path::RelativePath,
}
