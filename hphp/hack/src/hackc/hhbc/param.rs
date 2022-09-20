// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Maybe::*;
use ffi::Slice;
use ffi::Str;
use serde::Serialize;

use crate::Attribute;
use crate::Constraint;
use crate::Label;
use crate::TypeInfo;

#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Param<'arena> {
    pub name: Str<'arena>,
    pub is_variadic: bool,
    pub is_inout: bool,
    pub is_readonly: bool,
    pub user_attributes: Slice<'arena, Attribute<'arena>>,
    pub type_info: Maybe<TypeInfo<'arena>>,
    pub default_value: Maybe<DefaultValue<'arena>>,
}

#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct DefaultValue<'arena> {
    pub label: Label,
    pub expr: Str<'arena>,
}

impl<'arena> Param<'arena> {
    pub fn replace_default_value_label(&mut self, new_label: Label) {
        if let Just(dv) = self.default_value.as_mut() {
            dv.label = new_label;
        }
    }

    pub fn without_type(&mut self) {
        if let Just(ti) = self.type_info.as_mut() {
            ti.type_constraint = Constraint::default()
        }
    }

    pub fn with_name(&mut self, alloc: &'arena bumpalo::Bump, name: &str) {
        self.name = Str::new_str(alloc, name);
    }
}
