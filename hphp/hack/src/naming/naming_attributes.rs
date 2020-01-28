// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::ast::*;

pub fn mem(x: &str, xs: &Vec<UserAttribute>) -> bool {
    xs.iter().any(|ua| ua.name.1 == x)
}
