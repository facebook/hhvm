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

use crate::hhas_attribute::HhasAttribute;
use crate::hhas_coeffects::HhasCtxConstant;
use crate::hhas_constant::HhasConstant;
use crate::hhas_method::HhasMethod;
use crate::hhas_pos::HhasSpan;
use crate::hhas_property::HhasProperty;
use crate::hhas_type::HhasTypeInfo;
use crate::hhas_type_const::HhasTypeConstant;
use crate::ClassName;

#[derive(Copy, Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub enum TraitReqKind {
    MustExtend,
    MustImplement,
    MustBeClass,
}

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct HhasClass<'arena> {
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub base: Maybe<ClassName<'arena>>,
    pub implements: Slice<'arena, ClassName<'arena>>,
    pub enum_includes: Slice<'arena, ClassName<'arena>>,
    pub name: ClassName<'arena>,
    pub span: HhasSpan,
    pub uses: Slice<'arena, ClassName<'arena>>,
    pub enum_type: Maybe<HhasTypeInfo<'arena>>,
    pub methods: Slice<'arena, HhasMethod<'arena>>,
    pub properties: Slice<'arena, HhasProperty<'arena>>,
    pub constants: Slice<'arena, HhasConstant<'arena>>,
    pub type_constants: Slice<'arena, HhasTypeConstant<'arena>>,
    pub ctx_constants: Slice<'arena, HhasCtxConstant<'arena>>, // TODO(SF, 2021-0811): HhasCtxConstant is part of Steve's HhasCoeffect work
    pub requirements: Slice<'arena, Pair<ClassName<'arena>, TraitReqKind>>,
    pub upper_bounds: Slice<'arena, Pair<Str<'arena>, Slice<'arena, HhasTypeInfo<'arena>>>>,
    pub doc_comment: Maybe<Str<'arena>>,
    pub flags: Attr,
}

impl<'arena> HhasClass<'arena> {
    pub fn is_closure(&self) -> bool {
        self.methods.as_ref().iter().any(|x| x.is_closure_body())
    }
}
