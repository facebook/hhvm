// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Constant;

use crate::convert;
use crate::strings::StringCache;

pub(crate) fn convert_hack_constant(constant: ir::HackConstant, strings: &StringCache) -> Constant {
    let ir::HackConstant { name, value, attrs } = constant;

    let value = value
        .map(|v| convert::convert_typed_value(&v, strings))
        .into();

    let name = strings.lookup_const_name(name);

    Constant { name, value, attrs }
}
