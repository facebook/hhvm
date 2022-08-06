// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;
pub use hhvm_types_ffi::ffi::TypeConstraintFlags;

#[derive(Debug, Eq, PartialEq, Hash)]
pub enum Type<'a> {
    Empty,
    Flags(TypeConstraintFlags, Box<Type<'a>>),
    None,
    User(Str<'a>),
    UserNoConstraint(Str<'a>),
    UserWithConstraint(Str<'a>, Str<'a>),
    Void,
}

impl<'a> Default for Type<'a> {
    fn default() -> Self {
        Type::None
    }
}
