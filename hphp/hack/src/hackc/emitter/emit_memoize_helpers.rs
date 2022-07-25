// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use error::Error;
use error::Result;
use ffi::Slice;
use hhbc::FCallArgs;
use hhbc::FCallArgsFlags;
use hhbc::Local;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::aast::FunParam;
use oxidized::pos::Pos;

pub const MEMOIZE_SUFFIX: &str = "$memoize_impl";

pub fn get_memo_key_list<'arena>(temp_local: Local, param_local: Local) -> Vec<InstrSeq<'arena>> {
    vec![
        instr::get_memo_key_l(param_local),
        instr::set_l(temp_local),
        instr::pop_c(),
    ]
}

pub fn param_code_sets<'arena>(num_params: usize, first_unnamed: Local) -> InstrSeq<'arena> {
    InstrSeq::gather(
        (0..num_params)
            .flat_map(|i| {
                let param_local = Local::new(i);
                let temp_local = Local::new(first_unnamed.idx as usize + i);
                get_memo_key_list(temp_local, param_local)
            })
            .collect(),
    )
}

pub fn param_code_gets<'arena>(num_params: usize) -> InstrSeq<'arena> {
    InstrSeq::gather(
        (0..num_params)
            .map(|i| instr::c_get_l(Local::new(i)))
            .collect(),
    )
}

pub fn check_memoize_possible<Ex, En>(
    pos: &Pos,
    params: &[FunParam<Ex, En>],
    is_method: bool,
) -> Result<()> {
    if !is_method && params.iter().any(|param| param.is_variadic) {
        return Err(Error::fatal_runtime(
            pos,
            String::from("<<__Memoize>> cannot be used on functions with variable arguments"),
        ));
    }
    Ok(())
}

pub fn get_implicit_context_memo_key<'arena>(
    alloc: &'arena bumpalo::Bump,
    local: Local,
) -> InstrSeq<'arena> {
    InstrSeq::gather(vec![
        instr::null_uninit(),
        instr::null_uninit(),
        instr::f_call_func_d(
            FCallArgs::new(
                FCallArgsFlags::default(),
                1,
                0,
                Slice::empty(),
                Slice::empty(),
                None,
                None,
            ),
            hhbc::FunctionName::from_raw_string(
                alloc,
                "HH\\ImplicitContext\\_Private\\get_implicit_context_memo_key",
            ),
        ),
        instr::set_l(local),
        instr::pop_c(),
    ])
}
