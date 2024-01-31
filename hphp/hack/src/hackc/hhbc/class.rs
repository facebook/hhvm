// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Slice;
use ffi::Str;
use ffi::Vector;
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
use crate::UpperBound;

#[derive(Copy, Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub enum TraitReqKind {
    MustExtend,
    MustImplement,
    MustBeClass,
}

#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Requirement<'arena> {
    pub name: ClassName<'arena>,
    pub kind: TraitReqKind,
}

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Class<'arena> {
    pub attributes: Slice<'arena, Attribute<'arena>>,
    pub base: Maybe<ClassName<'arena>>,
    pub implements: Vector<ClassName<'arena>>,
    pub enum_includes: Vector<ClassName<'arena>>,
    pub name: ClassName<'arena>,
    pub span: Span,
    pub uses: Vector<ClassName<'arena>>,
    pub enum_type: Maybe<TypeInfo<'arena>>,
    pub methods: Vector<Method<'arena>>,
    pub properties: Vector<Property<'arena>>,
    pub constants: Vector<Constant<'arena>>,
    pub type_constants: Vector<TypeConstant<'arena>>,
    pub ctx_constants: Vector<CtxConstant<'arena>>,
    pub requirements: Vector<Requirement<'arena>>,
    pub upper_bounds: Slice<'arena, UpperBound<'arena>>,
    pub doc_comment: Maybe<Str<'arena>>,
    pub flags: Attr,
}

impl<'arena> Class<'arena> {
    pub fn is_closure(&self) -> bool {
        self.methods.as_ref().iter().any(|x| x.is_closure_body())
    }
}
