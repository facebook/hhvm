// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized_by_ref::shallow_decl_defs::{ShallowClass, ShallowMethod};
use oxidized_by_ref::typing_defs_core::Tparam;

pub struct Class<'a> {
    // TODO(hrust) Class is implemented directly as a lazy_class_type.
    // Make it an enum and add eager version if still necessary
    sc: ShallowClass<'a>,
    // TODO(hrust) missing fields from lazy_class_type
}

impl<'a> Class<'a> {
    pub fn tparams(&self) -> &'a [Tparam] {
        // TODO(hrust) missing logic: count + lazy / eager
        self.sc.tparams
    }

    pub fn get_method<'b>(class: &'b ShallowClass<'b>, id: &str) -> Option<&'b ShallowMethod<'b>> {
        // TODO(hrust) missing logic: count + lazy / eager
        class.methods.iter().find(|m| id == m.name.name())
    }
}
