// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::Env;
use hhas_param_rust::HhasParam;
use instruction_sequence_rust::InstrSeq;
use oxidized::doc_comment::DocComment;

#[derive(Default, Debug)]
pub struct HhasBody<'a> {
    pub body_instrs: InstrSeq,
    pub decl_vars: Vec<String>,
    pub num_iters: usize,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub upper_bounds: Vec<(String, Vec<hhas_type::Info>)>,
    pub params: Vec<HhasParam>,
    pub return_type_info: Option<hhas_type::Info>,
    pub doc_comment: Option<DocComment>,
    pub env: Option<Env<'a>>,
}

impl HhasBody<'_> {
    pub fn with_body_instrs(mut self, body_instrs: InstrSeq) -> Self {
        self.body_instrs = body_instrs;
        self
    }

    pub fn with_return_type_info(mut self, return_type_info: hhas_type::Info) -> Self {
        self.return_type_info = Some(return_type_info);
        self
    }

    pub fn with_params(mut self, params: Vec<HhasParam>) -> Self {
        self.params = params;
        self
    }
}
