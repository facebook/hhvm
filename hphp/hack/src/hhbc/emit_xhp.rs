// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhas_method_rust::HhasMethod;
use hhas_property_rust::HhasProperty;
use hhas_xhp_attribute_rust::HhasXhpAttribute;
use instruction_sequence_rust::{Error::Unrecoverable, Result};
use oxidized::{
    ast::{self as tast},
    ast_defs, namespace_env,
};

pub fn properties_for_cache<'a>(
    _ns: &namespace_env::Env,
    _class: &tast::Class_,
    _class_is_const: bool,
) -> Result<Option<HhasProperty<'a>>> {
    unimplemented!()
}

pub fn from_attribute_declaration<'a>(
    _class: &tast::Class_,
    _xal: &[HhasXhpAttribute],
    _xual: &[tast::Hint],
) -> Result<Option<HhasMethod<'a>>> {
    unimplemented!()
}

pub fn from_children_declaration<'a>(
    _ast_class: &tast::Class_,
    _children: &(&ast_defs::Pos, Vec<&tast::XhpChild>),
) -> Result<Option<HhasMethod<'a>>> {
    unimplemented!()
}

pub fn from_category_declaration<'a>(
    _ast_class: &tast::Class_,
    _categories: &(&ast_defs::Pos, Vec<&String>),
) -> Result<Option<HhasMethod<'a>>> {
    unimplemented!()
}
