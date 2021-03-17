// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc_by_ref_env::Env;
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_instruction_sequence::InstrSeq;
use oxidized::doc_comment::DocComment;

#[derive(Debug)] //Cannot be Default...
pub struct HhasBody<'a, 'arena> {
    pub body_instrs: InstrSeq<'arena>, //... because InstrSeq not Default.
    pub decl_vars: Vec<String>,
    pub num_iters: usize,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub upper_bounds: Vec<(String, Vec<hhbc_by_ref_hhas_type::Info>)>,
    pub shadowed_tparams: Vec<String>,
    pub params: Vec<HhasParam<'arena>>,
    pub return_type_info: Option<hhbc_by_ref_hhas_type::Info>,
    pub doc_comment: Option<DocComment>,
    pub env: Option<Env<'a, 'arena>>,
}
