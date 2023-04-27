// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Interestingly, HHAS does not represent the declared types of constants,
// unlike formal parameters and return types. We might consider fixing this.
// Also interestingly, abstract constants are not emitted at all.

use ffi::Maybe;
use serde::Serialize;

use crate::typed_value::TypedValue;
use crate::ConstName;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Constant<'arena> {
    pub name: ConstName<'arena>,
    pub value: Maybe<TypedValue<'arena>>,
    pub is_abstract: bool,
}
