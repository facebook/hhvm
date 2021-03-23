// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_hhas_adata::HhasAdata;
use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_hhas_class::HhasClass;
use hhbc_by_ref_hhas_constant::HhasConstant;
use hhbc_by_ref_hhas_function::HhasFunction;
use hhbc_by_ref_hhas_record_def::HhasRecord;
use hhbc_by_ref_hhas_symbol_refs::HhasSymbolRefs;
use hhbc_by_ref_hhas_typedef::Typedef;
use hhbc_by_ref_hhbc_ast::FatalOp;
use oxidized::pos::Pos;

#[derive(Default, Debug)]
pub struct HhasProgram<'a, 'arena> {
    pub adata: Vec<HhasAdata<'arena>>,
    pub functions: Vec<HhasFunction<'arena>>,
    pub classes: Vec<HhasClass<'a, 'arena>>,
    pub record_defs: Vec<HhasRecord<'a, 'arena>>,
    pub typedefs: Vec<Typedef<'arena>>,
    pub file_attributes: Vec<HhasAttribute<'arena>>,
    pub symbol_refs: HhasSymbolRefs,
    pub constants: Vec<HhasConstant<'arena>>,
    pub fatal: Option<(FatalOp, Pos, String)>,
}
