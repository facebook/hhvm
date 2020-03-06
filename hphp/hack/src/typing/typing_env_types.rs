// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::typing_env_return_info;
use oxidized::*;
pub use typing_defs_rust::{Ty, *};

pub struct Genv<'a> {
    pub tcopt: typechecker_options::TypecheckerOptions,
    pub return_: typing_env_return_info::TypingEnvReturnInfo<'a>,
    pub params: local_id::map::Map<(Ty<'a>, ParamMode)>,
    pub file: relative_path::RelativePath,
}
