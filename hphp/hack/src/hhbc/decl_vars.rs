// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env;
use hhas_param_rust::HhasParam;
use oxidized::ast as tast;

pub fn from_ast(
    _is_closure_body: bool,
    _has_this: bool,
    _is_toplevel: bool,
    _is_in_static_method: bool,
    _explicit_use_set: &env::SSet,
    _params: &[HhasParam],
    _body: &tast::Program,
) -> (bool, Vec<String>) {
    //TODO(hrust) implement
    (false, vec![])
}
