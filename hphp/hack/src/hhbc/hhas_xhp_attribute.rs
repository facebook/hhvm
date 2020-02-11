// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{ast as tast, pos::Pos};

#[derive(Debug)]
pub struct HhasXhpAttribute<'a> {
    pub type_: Option<&'a tast::Hint>,
    pub class_var: &'a tast::ClassVar,
    pub tag: Option<tast::XhpAttrTag>,
    pub maybe_enum: Option<&'a (Pos, bool, Vec<tast::Expr>)>,
}
