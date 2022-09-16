// Copyright (c) Facebook, Inc. and its affiliates.

use ffi::Str;
// Types imported from HHAS
pub use hhbc::TypedValue;
pub use hhvm_types_ffi::ffi::Attr;

/// Attributes are the bit of metadata that appears in double angle-brackets
/// before a definition. They look like either raw identifiers or function
/// calls.
///
/// Attributes can appear on files, classes, functions, or parameters.
///
/// ```
/// <<Attribute1, Attribute2(1, 2, 3)>>
/// class MyAttributedClass { ... }
/// ```
#[derive(Clone, Debug)]
pub struct Attribute<'a> {
    pub name: Str<'a>,
    pub arguments: Vec<TypedValue<'a>>,
}
