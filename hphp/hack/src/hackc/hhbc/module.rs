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
use crate::StringId;

#[derive(Clone, Copy, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub enum RuleKind {
    Global,
    Prefix,
    Exact,
}

#[derive(Debug, Eq, PartialEq, Clone, Serialize)]
#[repr(C)]
pub struct Rule {
    pub kind: RuleKind,
    pub name: Maybe<StringId>,
}

#[derive(Debug, Clone, Serialize)]
#[repr(C)]
pub struct Module {
    pub attributes: Vector<Attribute>,
    pub name: ModuleName,
    pub span: Span,
    pub doc_comment: Maybe<Vector<u8>>,
    pub exports: Maybe<Vector<Rule>>,
    pub imports: Maybe<Vector<Rule>>,
}
