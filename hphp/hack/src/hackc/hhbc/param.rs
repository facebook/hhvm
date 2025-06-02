// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Maybe::*;
use ffi::Vector;
use serde::Serialize;

use crate::Attribute;
use crate::Constraint;
use crate::StringId;
use crate::TypeInfo;

#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Param {
    pub name: StringId,
    pub is_variadic: bool,
    pub is_splat: bool,
    pub is_inout: bool,
    pub is_readonly: bool,
    pub is_optional: bool,
    pub user_attributes: Vector<Attribute>,
    pub type_info: Maybe<TypeInfo>,
}

impl Param {
    pub fn clear_type(&mut self) {
        if let Just(ti) = self.type_info.as_mut() {
            ti.type_constraint = Constraint::default()
        }
    }

    pub fn set_name(&mut self, name: &str) {
        self.name = crate::intern(name)
    }

    pub fn ty(&self) -> TypeInfo {
        match &self.type_info {
            Maybe::Just(ty) => ty.clone(),
            Maybe::Nothing => TypeInfo::empty(),
        }
    }
}
