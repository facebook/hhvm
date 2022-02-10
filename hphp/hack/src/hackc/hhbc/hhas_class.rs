// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Maybe, Pair, Quadruple, Slice, Str, Triple};
use hhas_attribute::HhasAttribute;
use hhas_coeffects::HhasCtxConstant;
use hhas_constant::HhasConstant;
use hhas_method::HhasMethod;
use hhas_pos::HhasSpan;
use hhas_property::HhasProperty;
use hhas_type::HhasTypeInfo;
use hhas_type_const::HhasTypeConstant;
use hhbc_id::class::ClassType;
use hhvm_types_ffi::ffi::Attr;

#[derive(Debug)]
#[repr(C)]
pub enum TraitReqKind {
    MustExtend,
    MustImplement,
}

#[derive(Debug)]
#[repr(C)]
pub struct HhasClass<'arena> {
    pub attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub base: Maybe<ClassType<'arena>>,
    pub implements: Slice<'arena, ClassType<'arena>>,
    pub enum_includes: Slice<'arena, ClassType<'arena>>,
    pub name: ClassType<'arena>,
    pub span: HhasSpan,
    pub uses: Slice<'arena, Str<'arena>>,
    // Deprecated - kill please
    pub use_aliases: Slice<
        'arena,
        Quadruple<Maybe<ClassType<'arena>>, ClassType<'arena>, Maybe<ClassType<'arena>>, Attr>,
    >,
    // Deprecated - kill please
    pub use_precedences: Slice<
        'arena,
        Triple<ClassType<'arena>, ClassType<'arena>, Slice<'arena, ClassType<'arena>>>,
    >,
    pub enum_type: Maybe<HhasTypeInfo<'arena>>,
    pub methods: Slice<'arena, HhasMethod<'arena>>,
    pub properties: Slice<'arena, HhasProperty<'arena>>,
    pub constants: Slice<'arena, HhasConstant<'arena>>,
    pub type_constants: Slice<'arena, HhasTypeConstant<'arena>>,
    pub ctx_constants: Slice<'arena, HhasCtxConstant<'arena>>, // TODO(SF, 2021-0811): HhasCtxConstant is part of Steve's HhasCoeffect work
    pub requirements: Slice<'arena, Pair<ClassType<'arena>, TraitReqKind>>,
    pub upper_bounds: Slice<'arena, Pair<Str<'arena>, Slice<'arena, HhasTypeInfo<'arena>>>>,
    pub doc_comment: Maybe<Str<'arena>>,
    pub flags: Attr,
}

impl<'arena> HhasClass<'arena> {
    pub fn is_closure(&self) -> bool {
        self.methods.as_ref().iter().any(|x| x.is_closure_body())
    }
}
