// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_fatal::raise_fatal_runtime;
use ffi::Str;
use hhas_param::HhasParam;
use hhbc_ast::FcallArgs;
use hhbc_id::function;
use hhvm_hhbc_defs_ffi::ffi::FCallArgsFlags;
use instruction_sequence::{instr, InstrSeq, Result};
use label::Label;
use local::{Local, LocalId};
use oxidized::{aast::FunParam, ast::Expr, pos::Pos};

use ffi::Slice;

pub const MEMOIZE_SUFFIX: &str = "$memoize_impl";

pub fn get_memo_key_list<'arena>(
    alloc: &'arena bumpalo::Bump,
    local: LocalId,
    index: u32,
    name: impl AsRef<str>,
) -> Vec<InstrSeq<'arena>> {
    vec![
        instr::getmemokeyl(alloc, Local::Named(Str::new_str(alloc, name.as_ref()))),
        instr::setl(
            alloc,
            Local::Unnamed(LocalId {
                idx: local.idx + index,
            }),
        ),
        instr::popc(alloc),
    ]
}

pub fn param_code_sets<'arena>(
    alloc: &'arena bumpalo::Bump,
    params: &[(HhasParam<'arena>, Option<(Label, Expr)>)],
    local: LocalId,
) -> InstrSeq<'arena> {
    InstrSeq::gather(
        alloc,
        params
            .iter()
            .enumerate()
            .map(|(i, (param, _))| {
                get_memo_key_list(
                    alloc,
                    local,
                    i.try_into().unwrap(),
                    &param.name.unsafe_as_str(),
                )
            })
            .flatten()
            .collect(),
    )
}

pub fn param_code_gets<'arena>(
    alloc: &'arena bumpalo::Bump,
    params: &[(HhasParam<'arena>, Option<(Label, Expr)>)],
) -> InstrSeq<'arena> {
    InstrSeq::gather(
        alloc,
        params
            .iter()
            .map(|(param, _)| {
                instr::cgetl(
                    alloc,
                    Local::Named(Str::new_str(alloc, param.name.unsafe_as_str())),
                )
            })
            .collect(),
    )
}

pub fn check_memoize_possible<Ex, En>(
    pos: &Pos,
    params: &[FunParam<Ex, En>],
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

pub fn get_implicit_context_memo_key<'arena>(
    alloc: &'arena bumpalo::Bump,
    local: LocalId,
) -> InstrSeq<'arena> {
    InstrSeq::gather(
        alloc,
        vec![
            instr::nulluninit(alloc),
            instr::nulluninit(alloc),
            instr::fcallfuncd(
                alloc,
                FcallArgs::new(
                    FCallArgsFlags::default(),
                    1,
                    Slice::empty(),
                    Slice::empty(),
                    None,
                    0,
                    None,
                ),
                function::from_raw_string(
                    alloc,
                    "HH\\ImplicitContext\\_Private\\get_implicit_context_memo_key",
                ),
            ),
            instr::setl(alloc, Local::Unnamed(local)),
            instr::popc(alloc),
        ],
    )
}
