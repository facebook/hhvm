// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::tast_collector::ByNames;
use crate::tast_collector::TastCollector;

impl ByNames {
    pub fn merge(&mut self, other: &mut Self) {
        let Self {
            fun_tasts,
            class_tasts,
            typedef_tasts,
            gconst_tasts,
            module_tasts,
        } = self;
        fun_tasts.append(&mut other.fun_tasts);
        class_tasts.append(&mut other.class_tasts);
        typedef_tasts.append(&mut other.typedef_tasts);
        gconst_tasts.append(&mut other.gconst_tasts);
        module_tasts.append(&mut other.module_tasts);
    }
}

pub fn merge_tast_collectors(x: &mut TastCollector, y: &mut TastCollector) {
    for (path, tasts) in y {
        x.entry(path.clone()).or_default().merge(tasts);
    }
}
