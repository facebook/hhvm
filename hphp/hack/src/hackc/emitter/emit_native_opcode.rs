// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_scope::Scope;
use env::emitter::Emitter;
use error::Error;
use error::Result;
use ffi::Maybe::Just;
use ffi::Slice;
use hhbc::hhas_body::HhasBody;
use hhbc::Local;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::aast;
use oxidized::ast;
use oxidized::pos::Pos;

use crate::emit_body;
use crate::emit_param;

pub fn emit_body<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    scope: &Scope<'a, 'arena>,
    class_attrs: &[ast::UserAttribute],
    name: &ast::Sid,
    params: &[ast::FunParam],
    ret: Option<&aast::Hint>,
) -> Result<HhasBody<'arena>> {
    let body_instrs = emit_native_opcode_impl(&name.1, params, class_attrs);
    let mut tparams = scope
        .get_tparams()
        .iter()
        .map(|tp| tp.name.1.as_str())
        .collect::<Vec<_>>();
    let params = emit_param::from_asts(emitter, &mut tparams, false, scope, params);
    let return_type_info =
        emit_body::emit_return_type_info(emitter.alloc, tparams.as_slice(), false, ret);

    body_instrs.and_then(|body_instrs| {
        params.and_then(|params| {
            return_type_info.map(|rti| HhasBody {
                body_instrs: Slice::from_vec(emitter.alloc, body_instrs.iter().cloned().collect()),
                params: Slice::fill_iter(emitter.alloc, params.into_iter().map(|p| p.0)),
                return_type_info: Just(rti),
                ..Default::default()
            })
        })
    })
}

fn emit_native_opcode_impl<'arena>(
    name: &str,
    params: &[ast::FunParam],
    user_attrs: &[ast::UserAttribute],
) -> Result<InstrSeq<'arena>> {
    if let [ua] = user_attrs {
        if ua.name.1 == "__NativeData" {
            if let [p] = ua.params.as_slice() {
                match p.2.as_string() {
                    Some(s) if s == "HH\\AsyncGenerator" || s == "Generator" => {
                        return emit_generator_method(name, params);
                    }
                    _ => {}
                }
            };
        }
    };
    Err(Error::fatal_runtime(
        &Pos::make_none(),
        format!("OpCodeImpl attribute is not applicable to {}", name),
    ))
}

fn emit_generator_method<'arena>(name: &str, params: &[ast::FunParam]) -> Result<InstrSeq<'arena>> {
    let instrs = match name {
        "send" => {
            let local = get_first_param_local(params)?;
            InstrSeq::gather(vec![
                instr::cont_check_check(),
                instr::push_l(local),
                instr::cont_enter(),
            ])
        }
        "raise" | "throw" => {
            let local = get_first_param_local(params)?;
            InstrSeq::gather(vec![
                instr::cont_check_check(),
                instr::push_l(local),
                instr::cont_raise(),
            ])
        }
        "next" | "rewind" => InstrSeq::gather(vec![
            instr::cont_check_ignore(),
            instr::null(),
            instr::cont_enter(),
        ]),
        "valid" => instr::cont_valid(),
        "current" => instr::cont_current(),
        "key" => instr::cont_key(),
        "getReturn" => instr::cont_get_return(),
        _ => {
            return Err(Error::fatal_runtime(
                &Pos::make_none(),
                "incorrect native generator function",
            ));
        }
    };
    Ok(InstrSeq::gather(vec![instrs, instr::ret_c()]))
}

fn get_first_param_local(params: &[ast::FunParam]) -> Result<Local> {
    match params {
        [_, ..] => Ok(Local::new(0)),
        _ => Err(Error::unrecoverable("native generator requires params")),
    }
}
