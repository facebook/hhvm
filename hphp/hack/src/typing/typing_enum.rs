// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_env_types::Env;
use oxidized::decl_defs::Ty as DTy;
use oxidized::shallow_decl_defs::ShallowMethod;

pub fn member_type<'a>(_env: &Env, member: &'a ShallowMethod) -> &'a DTy {
    let member_type = &member.type_;
    // TODO(hrust) xhp case
    member_type
}
