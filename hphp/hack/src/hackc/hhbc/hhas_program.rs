// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Maybe, Slice, Str, Triple};
use hhas_adata::HhasAdata;
use hhas_attribute::HhasAttribute;
use hhas_class::HhasClass;
use hhas_constant::HhasConstant;
use hhas_function::HhasFunction;
use hhas_pos::HhasPos;
use hhas_record_def::HhasRecord;
use hhas_symbol_refs::HhasSymbolRefs;
use hhas_typedef::HhasTypedef;
use hhbc_ast::FatalOp;

#[derive(Default, Debug)]
#[repr(C)]
pub struct HhasProgram<'arena> {
    pub adata: Slice<'arena, HhasAdata<'arena>>,
    pub functions: Slice<'arena, HhasFunction<'arena>>,
    pub classes: Slice<'arena, HhasClass<'arena>>,
    pub record_defs: Slice<'arena, HhasRecord<'arena>>,
    pub typedefs: Slice<'arena, HhasTypedef<'arena>>,
    pub file_attributes: Slice<'arena, HhasAttribute<'arena>>,
    pub symbol_refs: HhasSymbolRefs<'arena>,
    pub constants: Slice<'arena, HhasConstant<'arena>>,
    pub fatal: Maybe<Triple<FatalOp, HhasPos, Str<'arena>>>,
}
