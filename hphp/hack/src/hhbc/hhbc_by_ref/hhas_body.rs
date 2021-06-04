// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_instruction_sequence::InstrSeq;
use oxidized::{ast_defs, doc_comment::DocComment};

#[derive(Default, Debug)]
pub struct HhasBodyEnv {
    pub is_namespaced: bool,
    pub class_info: Option<(ast_defs::ClassKind, String)>,
    pub parent_name: Option<String>,
}

#[derive(Debug)] //Cannot be Default...
pub struct HhasBody<'arena> {
    pub body_instrs: InstrSeq<'arena>, //... because InstrSeq not Default.
    pub decl_vars: Vec<String>,
    pub num_iters: usize,
    pub num_closures: u32,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub upper_bounds: Vec<(String, Vec<hhbc_by_ref_hhas_type::Info>)>,
    pub shadowed_tparams: Vec<String>,
    pub params: Vec<HhasParam<'arena>>,
    pub return_type_info: Option<hhbc_by_ref_hhas_type::Info>,
    pub doc_comment: Option<DocComment>,
    pub env: Option<HhasBodyEnv>,
}

pub fn default_with_body_instrs<'arena>(body_instrs: InstrSeq<'arena>) -> HhasBody<'arena> {
    HhasBody {
        body_instrs,
        decl_vars: Vec::<String>::default(),
        num_iters: usize::default(),
        num_closures: u32::default(),
        is_memoize_wrapper: bool::default(),
        is_memoize_wrapper_lsb: bool::default(),
        upper_bounds: Vec::<(String, Vec<hhbc_by_ref_hhas_type::Info>)>::default(),
        shadowed_tparams: Vec::<String>::default(),
        params: Vec::<HhasParam<'arena>>::default(),
        return_type_info: Option::<hhbc_by_ref_hhas_type::Info>::default(),
        doc_comment: Option::<DocComment>::default(),
        env: Option::<HhasBodyEnv>::default(),
    }
}
