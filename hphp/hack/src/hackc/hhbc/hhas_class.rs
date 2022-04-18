// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    hhas_attribute::HhasAttribute, hhas_coeffects::HhasCtxConstant, hhas_constant::HhasConstant,
    hhas_method::HhasMethod, hhas_pos::HhasSpan, hhas_property::HhasProperty,
    hhas_type::HhasTypeInfo, hhas_type_const::HhasTypeConstant, ClassName,
};
use ffi::{Maybe, Pair, Slice, Str};
use hhvm_types_ffi::ffi::Attr;

#[derive(Debug)]
#[repr(C)]
pub enum TraitReqKind {
    MustExtend,
    MustImplement,
    MustBeClass,
}

#[derive(Debug)]
#[repr(C)]
pub struct HhasClass<'arena> {
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub base: Maybe<ClassName<'arena>>,
    pub implements: Slice<'arena, ClassName<'arena>>,
    pub enum_includes: Slice<'arena, ClassName<'arena>>,
    pub name: ClassName<'arena>,
    pub span: HhasSpan,
    pub uses: Slice<'arena, Str<'arena>>,
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
