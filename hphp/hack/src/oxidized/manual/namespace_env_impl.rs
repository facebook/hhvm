// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::namespace_env::Env;
use crate::s_map::SMap;

impl Env {
    pub fn empty(auto_ns_map: Vec<(String, String)>, is_codegen: bool) -> Self {
        Env {
            ns_uses: SMap::new(),
            class_uses: SMap::new(),
            fun_uses: SMap::new(),
            const_uses: SMap::new(),
            name: None,
            auto_ns_map,
            is_codegen,
        }
    }
}
