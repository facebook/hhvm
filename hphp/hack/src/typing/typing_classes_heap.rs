// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::shallow_decl_defs::ShallowClass;
use oxidized::typing_defs_core::Tparam;

pub struct Class {
    // TODO(hrust) Class is implemented directly as a lazy_class_type.
    // Make it an enum and add eager version if still necessary
    sc: ShallowClass,
    // TODO(hrust) missing fields from lazy_class_type
}

impl Class {
    pub fn tparams(&self) -> &Vec<Tparam> {
        // TODO(hrust) missing logic: count + lazy / eager
        &self.sc.tparams
    }
}
