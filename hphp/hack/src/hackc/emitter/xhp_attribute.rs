// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::ast;
use oxidized::pos::Pos;

#[derive(Debug)]
pub struct XhpAttribute<'a> {
    pub type_: Option<&'a ast::Hint>,
    pub class_var: &'a ast::ClassVar,
    pub tag: Option<ast::XhpAttrTag>,
    pub maybe_enum: Option<&'a (Pos, Vec<ast::Expr>)>,
}

impl<'a> XhpAttribute<'a> {
    pub fn is_required(&self) -> bool {
        matches!(
            self.tag,
            Some(ast::XhpAttrTag::Required) | Some(ast::XhpAttrTag::LateInit)
        )
    }
}
