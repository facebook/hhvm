// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Slice;
use ffi::Str;
use serde::Serialize;

use crate::Attribute;
use crate::ClassName;
use crate::Span;

#[derive(Clone, Copy, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub enum RuleKind {
    Global,
    Prefix,
    Exact,
}

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Rule<'arena> {
    pub kind: RuleKind,
    pub name: Maybe<Str<'arena>>,
}

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Module<'arena> {
    pub attributes: Slice<'arena, Attribute<'arena>>,
    pub name: ClassName<'arena>,
    pub span: Span,
    pub doc_comment: Maybe<Str<'arena>>,
    pub exports: Maybe<Slice<'arena, Rule<'arena>>>,
    pub imports: Maybe<Slice<'arena, Rule<'arena>>>,
}
