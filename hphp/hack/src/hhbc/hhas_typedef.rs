// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhas_attribute_rust as hhas_attribute;
use hhas_type;
use hhbc_id_rust as hhas_id;
use runtime;

use hhas_attribute::HhasAttribute;
use runtime::TypedValue;

#[derive(Debug)]
pub struct Typedef<'a> {
    pub name: hhas_id::class::Type<'a>,
    pub attributes: Vec<HhasAttribute>,
    pub type_info: hhas_type::Info,
    pub type_structure: TypedValue,
}
