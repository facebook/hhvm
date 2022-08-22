// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Pair;
use ffi::Slice;
use ffi::Str;
use hhvm_types_ffi::ffi::Attr;
use serde::Serialize;

use crate::Attribute;
use crate::ClassName;
use crate::Constant;
use crate::CtxConstant;
use crate::Method;
use crate::Property;
use crate::Span;
use crate::TypeConstant;
use crate::TypeInfo;

#[derive(Copy, Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub enum TraitReqKind {
    MustExtend,
    MustImplement,
    MustBeClass,
}

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Class<'arena> {
    pub attributes: Slice<'arena, Attribute<'arena>>,
    pub base: Maybe<ClassName<'arena>>,
    pub implements: Slice<'arena, ClassName<'arena>>,
    pub enum_includes: Slice<'arena, ClassName<'arena>>,
    pub name: ClassName<'arena>,
    pub span: Span,
    pub uses: Slice<'arena, ClassName<'arena>>,
    pub enum_type: Maybe<TypeInfo<'arena>>,
    pub methods: Slice<'arena, Method<'arena>>,
    pub properties: Slice<'arena, Property<'arena>>,
    pub constants: Slice<'arena, Constant<'arena>>,
    pub type_constants: Slice<'arena, TypeConstant<'arena>>,
    pub ctx_constants: Slice<'arena, CtxConstant<'arena>>, // TODO(SF, 2021-0811): CtxConstant is part of Steve's Coeffect work
    pub requirements: Slice<'arena, Pair<ClassName<'arena>, TraitReqKind>>,
    pub upper_bounds: Slice<'arena, Pair<Str<'arena>, Slice<'arena, TypeInfo<'arena>>>>,
    pub doc_comment: Maybe<Str<'arena>>,
    pub flags: Attr,
}

impl<'arena> Class<'arena> {
    pub fn is_closure(&self) -> bool {
        self.methods.as_ref().iter().any(|x| x.is_closure_body())
    }
}
