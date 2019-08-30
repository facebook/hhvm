// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::namespace_env::Env;
use crate::map::Map;

impl Env {
    pub fn empty(auto_ns_map: Vec<(String, String)>, is_codegen: bool) -> Self {
        Env {
            ns_uses: Map::empty(),
            class_uses: Map::empty(),
            fun_uses: Map::empty(),
            const_uses: Map::empty(),
            name: None,
            auto_ns_map,
            is_codegen,
        }
    }
}
