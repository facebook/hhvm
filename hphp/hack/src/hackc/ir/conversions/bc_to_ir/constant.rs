// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Constant;
use ir::StringInterner;

use crate::convert;

pub(crate) fn convert_constant(constant: &Constant, strings: &StringInterner) -> ir::HackConstant {
    let Constant {
        name,
        ref value,
        attrs,
    } = *constant;

    let value = value
        .as_ref()
        .map(|tv| convert::convert_typed_value(tv, strings))
        .into();

    let name = ir::ConstId::from_hhbc(name, strings);

    ir::HackConstant { name, value, attrs }
}
