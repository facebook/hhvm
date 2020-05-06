// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_env_types::Env;
use oxidized_by_ref::decl_defs::Ty;
use oxidized_by_ref::shallow_decl_defs::ShallowMethod;

pub fn member_type<'a>(_env: &Env, member: &'a ShallowMethod<'a>) -> &'a Ty<'a> {
    // TODO(hrust) xhp case
    &member.type_
}
