// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Constant;

pub(crate) fn convert_hack_constant(constant: ir::HackConstant) -> Constant {
    let ir::HackConstant { name, value, attrs } = constant;
    let value = value.into();
    Constant { name, value, attrs }
}
