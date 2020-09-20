// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhas_adata_rust::HhasAdata;
use hhas_attribute_rust::HhasAttribute;
use hhas_class_rust::HhasClass;
use hhas_constant_rust::HhasConstant;
use hhas_function_rust::HhasFunction;
use hhas_record_def_rust::HhasRecord;
use hhas_symbol_refs_rust::HhasSymbolRefs;
use hhas_typedef_rust::Typedef;
use hhbc_ast_rust::FatalOp;
use oxidized::pos::Pos;

#[derive(Default, Debug)]
pub struct HhasProgram<'a> {
    pub adata: Vec<HhasAdata>,
    pub functions: Vec<HhasFunction<'a>>,
    pub classes: Vec<HhasClass<'a>>,
    pub record_defs: Vec<HhasRecord<'a>>,
    pub typedefs: Vec<Typedef<'a>>,
    pub file_attributes: Vec<HhasAttribute>,
    pub symbol_refs: HhasSymbolRefs,
    pub constants: Vec<HhasConstant<'a>>,
    pub fatal: Option<(FatalOp, Pos, String)>,
}
