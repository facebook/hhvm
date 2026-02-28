// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct RemoveMemoAttr {}

impl Pass for RemoveMemoAttr {
    fn on_ty_fun__top_down(&mut self, _: &Env, elem: &mut nast::Fun_) -> ControlFlow<()> {
        remove_memo_attr(&mut elem.user_attributes);
        Continue(())
    }

    fn on_ty_method__top_down(&mut self, _: &Env, elem: &mut nast::Method_) -> ControlFlow<()> {
        remove_memo_attr(&mut elem.user_attributes);
        Continue(())
    }
}

/// Removes any memoization attributes
fn remove_memo_attr(input_attrs: &mut nast::UserAttributes) {
    input_attrs
        .0
        .retain(|attr| !sn::user_attributes::is_memoized(attr.name.name()));
}
