// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use hhas_function_rust::HhasFunction;
use hhbc_id_rust::function::Type as FunId;
use options::{HhvmFlags, Options, RepoFlags};
use oxidized::aast as a;
use runtime::TypedValue;

pub(crate) fn is_interceptable(fun_id: FunId, opts: &Options) -> bool {
    let difs = opts.hhvm.dynamic_invoke_functions.get();
    opts.hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION)
        && !opts.repo_flags.contains(RepoFlags::AUTHORITATIVE)
        || !difs.is_empty() && {
            let mut name: String = fun_id.into();
            name.make_ascii_lowercase(); // in-place is more efficient if fun_id is owned
            difs.get(&name).is_some()
        }
}

// TODO(hrust) implement the rest here; it's only used by emit_function.rs

pub(crate) fn emit_wrapper_function<Ex, Fb, En, Hi>(
    _original_id: FunId,
    _renamed_id: &FunId,
    _deprecation_info: &Option<&[TypedValue]>,
    _fun: &a::Fun_<Ex, Fb, En, Hi>,
) -> HhasFunction<'static> {
    unimplemented!("TODO(hrust) later")
}
