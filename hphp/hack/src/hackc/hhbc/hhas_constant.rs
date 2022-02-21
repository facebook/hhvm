// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Interestingly, HHAS does not represent the declared types of constants,
// unlike formal parameters and return types. We might consider fixing this.
// Also interestingly, abstract constants are not emitted at all.

use ffi::Maybe;
use hhbc_id::r#const::ConstType;
use runtime::TypedValue;

#[derive(Debug)]
#[repr(C)]
pub struct HhasConstant<'arena> {
    pub name: ConstType<'arena>,
    pub value: Maybe<TypedValue<'arena>>,
    pub is_abstract: bool,
}
