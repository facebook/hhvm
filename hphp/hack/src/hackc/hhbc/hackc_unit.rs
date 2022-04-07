// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    hhas_adata::HhasAdata, hhas_attribute::HhasAttribute, hhas_class::HhasClass,
    hhas_constant::HhasConstant, hhas_function::HhasFunction, hhas_module::HhasModule,
    hhas_pos::HhasPos, hhas_symbol_refs::HhasSymbolRefs, hhas_typedef::HhasTypedef,
    hhbc_ast::FatalOp,
};
use ffi::{Maybe, Slice, Str, Triple};

#[derive(Default, Debug)]
#[repr(C)]
pub struct HackCUnit<'arena> {
    pub adata: Slice<'arena, HhasAdata<'arena>>,
    pub functions: Slice<'arena, HhasFunction<'arena>>,
    pub classes: Slice<'arena, HhasClass<'arena>>,
    pub modules: Slice<'arena, HhasModule<'arena>>,
    pub typedefs: Slice<'arena, HhasTypedef<'arena>>,
    pub file_attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub module_use: Maybe<Str<'arena>>,
    pub symbol_refs: HhasSymbolRefs<'arena>,
    pub constants: Slice<'arena, HhasConstant<'arena>>,
    pub fatal: Maybe<Triple<FatalOp, HhasPos, Str<'arena>>>,
}
