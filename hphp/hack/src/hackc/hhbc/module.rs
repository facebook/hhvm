// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Vector;
use serde::Serialize;

use crate::Attribute;
use crate::ModuleName;
use crate::Span;

#[derive(Debug, Clone, Serialize)]
#[repr(C)]
pub struct Module {
    pub attributes: Vector<Attribute>,
    pub name: ModuleName,
    pub span: Span,
    pub doc_comment: Maybe<Vector<u8>>,
}
