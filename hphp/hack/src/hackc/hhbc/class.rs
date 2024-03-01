// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
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
pub struct Requirement {
    pub name: ClassName,
    pub kind: TraitReqKind,
}

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Class<'arena> {
    pub attributes: Vector<Attribute>,
    pub base: Maybe<ClassName>,
    pub implements: Vector<ClassName>,
    pub enum_includes: Vector<ClassName>,
    pub name: ClassName,
    pub span: Span,
    pub uses: Vector<ClassName>,
    pub enum_type: Maybe<TypeInfo>,
    pub methods: Vector<Method<'arena>>,
    pub properties: Vector<Property>,
    pub constants: Vector<Constant>,
    pub type_constants: Vector<TypeConstant>,
    pub ctx_constants: Vector<CtxConstant>,
    pub requirements: Vector<Requirement>,
    pub upper_bounds: Vector<UpperBound>,
    pub doc_comment: Maybe<Vector<u8>>,
    pub flags: Attr,
}

impl<'arena> Class<'arena> {
    pub fn is_closure(&self) -> bool {
        self.methods.as_ref().iter().any(|x| x.is_closure_body())
    }
}
