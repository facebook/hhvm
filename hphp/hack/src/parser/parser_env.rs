// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Clone, Debug, Default)]
pub struct ParserEnv {
    pub codegen: bool,
    pub hhvm_compat_mode: bool,
    pub php5_compat_mode: bool,
    pub enable_xhp_class_modifier: bool,
    pub disable_xhp_element_mangling: bool,
    pub disable_xhp_children_declarations: bool,
    pub interpret_soft_types_as_like_types: bool,
    pub nameof_precedence: bool,
}
