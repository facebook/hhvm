// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Slice;
use ffi::Str;
use ffi::Triple;
use serde::Serialize;

use crate::hhas_adata::HhasAdata;
use crate::hhas_attribute::HhasAttribute;
use crate::hhas_class::HhasClass;
use crate::hhas_constant::HhasConstant;
use crate::hhas_function::HhasFunction;
use crate::hhas_module::HhasModule;
use crate::hhas_pos::HhasPos;
use crate::hhas_symbol_refs::HhasSymbolRefs;
use crate::hhas_typedef::HhasTypedef;
use crate::FatalOp;

#[derive(Default, Debug, Serialize)]
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
