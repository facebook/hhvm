// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhas_param_rust::HhasParam;
use instruction_sequence::InstrSeq;
use oxidized::ast_defs;
use oxidized::doc_comment::DocComment;

#[derive(Default, Debug)]
pub struct HhasBodyEnv {
    pub is_namespaced: bool,
    pub class_info: Option<(ast_defs::ClassKind, String)>,
    pub parent_name: Option<String>,
}

#[derive(Default, Debug)]
pub struct HhasBody {
    pub body_instrs: InstrSeq,
    pub decl_vars: Vec<String>,
    pub num_iters: usize,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub upper_bounds: Vec<(String, Vec<hhas_type::Info>)>,
    pub shadowed_tparams: Vec<String>,
    pub params: Vec<HhasParam>,
    pub return_type_info: Option<hhas_type::Info>,
    pub doc_comment: Option<DocComment>,
    pub env: Option<HhasBodyEnv>,
}
