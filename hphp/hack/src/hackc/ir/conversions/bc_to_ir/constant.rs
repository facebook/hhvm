// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Constant;

pub(crate) fn convert_constant<'a>(constant: &Constant<'a>) -> ir::HackConstant<'a> {
    let Constant {
        name,
        ref value,
        is_abstract,
    } = *constant;

    let value = value.clone().into();

    ir::HackConstant {
        name,
        value,
        is_abstract,
    }
}
