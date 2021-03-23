// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_fatal_rust as emit_fatal;
use hhas_param_rust::HhasParam;
use instruction_sequence::{instr, InstrSeq, Result};
use oxidized::{aast::FunParam, pos::Pos};

pub const MEMOIZE_SUFFIX: &str = "$memoize_impl";

pub fn get_memo_key_list(local: local::Id, index: usize, name: String) -> Vec<InstrSeq> {
    vec![
        instr::getmemokeyl(local::Type::Named(name)),
        instr::setl(local::Type::Unnamed(local + index)),
        instr::popc(),
    ]
}

pub fn param_code_sets(params: &[HhasParam], local: local::Id) -> InstrSeq {
    InstrSeq::gather(
        params
            .iter()
            .enumerate()
            .map(|(i, param)| get_memo_key_list(local, i, param.name.clone()))
            .flatten()
            .collect(),
    )
}

pub fn param_code_gets(params: &[HhasParam]) -> InstrSeq {
    InstrSeq::gather(
        params
            .iter()
            .map(|param| instr::cgetl(local::Type::Named(param.name.clone())))
            .collect(),
    )
}

pub fn check_memoize_possible<Ex, Fb, En, Hi>(
    pos: &Pos,
    params: &[FunParam<Ex, Fb, En, Hi>],
    is_method: bool,
) -> Result<()> {
    if !is_method && params.iter().any(|param| param.is_variadic) {
        return Err(emit_fatal::raise_fatal_runtime(
            pos,
            String::from("<<__Memoize>> cannot be used on functions with variable arguments"),
        ));
    }
    Ok(())
}
