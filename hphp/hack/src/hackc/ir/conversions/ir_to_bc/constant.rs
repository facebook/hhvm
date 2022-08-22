// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Constant;

pub(crate) fn convert_hack_constant<'a>(constant: ir::HackConstant<'a>) -> Constant<'a> {
    let ir::HackConstant {
        name,
        value,
        is_abstract,
    } = constant;

    Constant {
        name,
        value: value.into(),
        is_abstract,
    }
}
