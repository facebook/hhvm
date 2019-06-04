// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Clone, Debug, Default)]
pub struct ParserEnv {
    pub codegen: bool,
    pub is_experimental_mode: bool,
    pub disable_unsafe_expr: bool,
    pub disable_unsafe_block: bool,
    pub hhvm_compat_mode: bool,
    pub php5_compat_mode: bool,
}
