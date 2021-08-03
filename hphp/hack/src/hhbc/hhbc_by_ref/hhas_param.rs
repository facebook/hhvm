// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;
use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_hhas_type::{constraint, Info};
use hhbc_by_ref_label::Label;
use oxidized::ast as tast;

#[derive(Clone, Debug)]
pub struct HhasParam<'arena> {
    pub name: Str<'arena>,
    pub is_variadic: bool,
    pub is_inout: bool,
    pub user_attributes: Vec<HhasAttribute<'arena>>,
    pub type_info: Option<Info<'arena>>,
    pub default_value: Option<(Label, tast::Expr)>,
}

impl<'arena> HhasParam<'arena> {
    pub fn replace_default_value_label(&mut self, new_label: Label) {
        if let Some((label, _)) = self.default_value.as_mut() {
            *label = new_label;
        }
    }

    pub fn without_type(&mut self) {
        if let Some(ti) = self.type_info.as_mut() {
            ti.type_constraint = constraint::Constraint::default()
        }
    }

    pub fn with_name(&mut self, alloc: &'arena bumpalo::Bump, name: &str) {
        self.name = Str::new_str(alloc, name);
    }
}
