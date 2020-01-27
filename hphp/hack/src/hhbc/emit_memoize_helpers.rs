// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_fatal_rust as emit_fatal;
use hhas_param_rust::HhasParam;
use instruction_sequence_rust::{InstrSeq, Result};
use local_rust as local;
use oxidized::{aast::FunParam, pos::Pos};

pub const MEMOIZE_SUFFIX: &str = "$memoize_impl";

fn get_memo_key_list(local: local::Id, index: usize, name: String) -> Vec<InstrSeq> {
    vec![
        InstrSeq::make_getmemokeyl(local::Type::Named(name)),
        InstrSeq::make_setl(local::Type::Unnamed(local + index)),
        InstrSeq::make_popc(),
    ]
}

pub fn param_code_sets(params: Vec<HhasParam>, local: local::Id) -> InstrSeq {
    InstrSeq::gather(
        params
            .into_iter()
            .enumerate()
            .map(|(i, param)| get_memo_key_list(local, i, param.name))
            .flatten()
            .collect(),
    )
}

pub fn param_code_gets(params: Vec<HhasParam>) -> InstrSeq {
    InstrSeq::gather(
        params
            .into_iter()
            .map(|param| InstrSeq::make_cgetl(local::Type::Named(param.name)))
            .collect(),
    )
}

pub fn check_memoize_possible<Ex, Fb, En, Hi>(
    pos: &Pos,
    params: &[&FunParam<Ex, Fb, En, Hi>],
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
