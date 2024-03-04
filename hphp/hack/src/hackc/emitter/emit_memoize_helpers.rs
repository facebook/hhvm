// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::LabelGen;
use error::Error;
use error::Result;
use hhbc::FCallArgs;
use hhbc::FCallArgsFlags;
use hhbc::Local;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::aast::FunParam;
use oxidized::pos::Pos;
use scope::create_try_catch;

pub const MEMOIZE_SUFFIX: &str = "$memoize_impl";

pub fn get_memo_key_list(temp_local: Local, param_local: Local) -> Vec<InstrSeq> {
    vec![
        instr::get_memo_key_l(param_local),
        instr::set_l(temp_local),
        instr::pop_c(),
    ]
}

pub fn param_code_sets(num_params: usize, first_unnamed: Local) -> InstrSeq {
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

pub fn param_code_gets(num_params: usize) -> InstrSeq {
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

pub fn get_implicit_context_memo_key(local: Local) -> InstrSeq {
    InstrSeq::gather(vec![
        instr::null_uninit(),
        instr::null_uninit(),
        instr::f_call_func_d(
            FCallArgs::new(FCallArgsFlags::default(), 1, 0, vec![], vec![], None, None),
            hhbc::FunctionName::intern(
                "HH\\ImplicitContext\\_Private\\get_implicit_context_memo_key",
            ),
        ),
        instr::set_l(local),
        instr::pop_c(),
    ])
}

fn ic_set(local: Local, soft: bool) -> InstrSeq {
    if soft {
        InstrSeq::gather(vec![
            instr::cns_e(hhbc::ConstName::intern(
                "HH\\MEMOIZE_IC_TYPE_SOFT_INACCESSIBLE",
            )),
            instr::null(),
            instr::create_special_implicit_context(),
            instr::set_implicit_context_by_value(),
            instr::set_l(local),
            instr::pop_c(),
        ])
    } else {
        InstrSeq::gather(vec![
            instr::null_uninit(),
            instr::null_uninit(),
            instr::f_call_func_d(
                FCallArgs::new(FCallArgsFlags::default(), 1, 0, vec![], vec![], None, None),
                hhbc::FunctionName::intern(
                    "HH\\ImplicitContext\\_Private\\create_ic_inaccessible_context",
                ),
            ),
            instr::set_implicit_context_by_value(),
            instr::set_l(local),
            instr::pop_c(),
        ])
    }
}

pub fn ic_restore(local: Local, should_make_ic_inaccessible: Option<bool>) -> InstrSeq {
    if should_make_ic_inaccessible.is_some() {
        InstrSeq::gather(vec![
            instr::c_get_l(local),
            instr::set_implicit_context_by_value(),
            instr::pop_c(),
        ])
    } else {
        instr::empty()
    }
}

pub fn with_possible_ic<'arena>(
    label_gen: &mut LabelGen,
    local: Local,
    instrs: InstrSeq,
    should_make_ic_inaccessible: Option<bool>,
) -> InstrSeq {
    if let Some(soft) = should_make_ic_inaccessible {
        InstrSeq::gather(vec![
            ic_set(local, soft),
            create_try_catch(
                label_gen,
                None,
                false,
                instrs,
                ic_restore(local, should_make_ic_inaccessible),
            ),
            ic_restore(local, should_make_ic_inaccessible),
        ])
    } else {
        instrs
    }
}
