// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_fold_options::DeclFoldOptions;
use crate::global_options::GlobalOptions;

impl DeclFoldOptions {
    pub fn from_global_options(opts: &GlobalOptions) -> Self {
        Self {
            implicit_inherit_sdt: opts.tco_implicit_inherit_sdt,
            everything_sdt: opts.po.everything_sdt,
            enable_strict_const_semantics: opts.tco_enable_strict_const_semantics,
            class_class_type: opts.class_class_type,
        }
    }
}

impl Default for DeclFoldOptions {
    fn default() -> Self {
        DeclFoldOptions::from_global_options(&Default::default())
    }
}
