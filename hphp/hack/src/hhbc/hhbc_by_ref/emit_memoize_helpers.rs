// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_emit_fatal::raise_fatal_runtime;
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_instruction_sequence::{instr, InstrSeq, Result};
use oxidized::{aast::FunParam, pos::Pos};

pub const MEMOIZE_SUFFIX: &str = "$memoize_impl";

pub fn get_memo_key_list<'arena>(
    alloc: &'arena bumpalo::Bump,
    local: hhbc_by_ref_local::Id,
    index: usize,
    name: impl AsRef<str>,
) -> Vec<InstrSeq<'arena>> {
    vec![
        instr::getmemokeyl(
            alloc,
            hhbc_by_ref_local::Type::Named(alloc.alloc_str(name.as_ref())),
        ),
        instr::setl(alloc, hhbc_by_ref_local::Type::Unnamed(local + index)),
        instr::popc(alloc),
    ]
}

pub fn param_code_sets<'arena>(
    alloc: &'arena bumpalo::Bump,
    params: &[HhasParam<'arena>],
    local: hhbc_by_ref_local::Id,
) -> InstrSeq<'arena> {
    InstrSeq::gather(
        alloc,
        params
            .iter()
            .enumerate()
            .map(|(i, param)| get_memo_key_list(alloc, local, i, &param.name))
            .flatten()
            .collect(),
    )
}

pub fn param_code_gets<'arena>(
    alloc: &'arena bumpalo::Bump,
    params: &[HhasParam],
) -> InstrSeq<'arena> {
    InstrSeq::gather(
        alloc,
        params
            .iter()
            .map(|param| {
                instr::cgetl(
                    alloc,
                    hhbc_by_ref_local::Type::Named(alloc.alloc_str(param.name.as_ref())),
                )
            })
            .collect(),
    )
}

pub fn check_memoize_possible<Ex, Fb, En, Hi>(
    pos: &Pos,
    params: &[FunParam<Ex, Fb, En, Hi>],
    is_method: bool,
) -> Result<()> {
    if !is_method && params.iter().any(|param| param.is_variadic) {
        return Err(raise_fatal_runtime(
            pos,
            String::from("<<__Memoize>> cannot be used on functions with variable arguments"),
        ));
    }
    Ok(())
}
