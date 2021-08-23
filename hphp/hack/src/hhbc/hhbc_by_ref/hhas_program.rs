// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Maybe, Slice, Str, Triple};
use hhbc_by_ref_hhas_adata::HhasAdata;
use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_hhas_class::HhasClass;
use hhbc_by_ref_hhas_constant::HhasConstant;
use hhbc_by_ref_hhas_function::HhasFunction;
use hhbc_by_ref_hhas_pos::HhasPos;
use hhbc_by_ref_hhas_record_def::HhasRecord;
use hhbc_by_ref_hhas_symbol_refs::HhasSymbolRefs;
use hhbc_by_ref_hhas_typedef::HhasTypedef;
use hhbc_by_ref_hhbc_ast::FatalOp;

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

#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhas_program<'arena>(
    _: HhasProgram<'arena>,
) {
    unimplemented!()
}
