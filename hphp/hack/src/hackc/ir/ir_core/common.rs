// Copyright (c) Facebook, Inc. and its affiliates.

use crate::ClassName;
use crate::TypedValue;

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
pub struct Attribute {
    pub name: ClassName,
    pub arguments: Vec<TypedValue>,
}
