// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_scope::Scope;
use emit_fatal::raise_fatal_runtime;
use env::emitter::Emitter;
use ffi::{Maybe::Just, Slice, Str};
use hhas_body::HhasBody;
use instruction_sequence::{instr, Error::Unrecoverable, InstrSeq, Result};
use local::Local;
use oxidized::{aast, ast, pos::Pos};

pub fn emit_body<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    scope: &Scope<'a, 'arena>,
    class_attrs: &[ast::UserAttribute],
    name: &ast::Sid,
    params: &[ast::FunParam],
    ret: Option<&aast::Hint>,
) -> Result<HhasBody<'arena>> {
    let body_instrs = emit_native_opcode_impl(emitter.alloc, &name.1, params, class_attrs);
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
    alloc: &'arena bumpalo::Bump,
    name: &str,
    params: &[ast::FunParam],
    user_attrs: &[ast::UserAttribute],
) -> Result<InstrSeq<'arena>> {
    if let [ua] = user_attrs {
        if ua.name.1 == "__NativeData" {
            if let [p] = ua.params.as_slice() {
                match p.2.as_string() {
                    Some(s) if s == "HH\\AsyncGenerator" || s == "Generator" => {
                        return emit_generator_method(alloc, name, params);
                    }
                    _ => {}
                }
            };
        }
    };
    Err(emit_fatal::raise_fatal_runtime(
        &Pos::make_none(),
        format!("OpCodeImpl attribute is not applicable to {}", name),
    ))
}

fn emit_generator_method<'arena>(
    alloc: &'arena bumpalo::Bump,
    name: &str,
    params: &[ast::FunParam],
) -> Result<InstrSeq<'arena>> {
    let instrs = match name {
        "send" => {
            let local = Local::Named(Str::new_str(alloc, get_first_param_name(params)?));
            InstrSeq::gather(
                alloc,
                vec![
                    instr::contcheck_check(alloc),
                    instr::pushl(alloc, local),
                    instr::contenter(alloc),
                ],
            )
        }
        "raise" | "throw" => {
            let local = Local::Named(Str::new_str(alloc, get_first_param_name(params)?));
            InstrSeq::gather(
                alloc,
                vec![
                    instr::contcheck_check(alloc),
                    instr::pushl(alloc, local),
                    instr::contraise(alloc),
                ],
            )
        }
        "next" | "rewind" => InstrSeq::gather(
            alloc,
            vec![
                instr::contcheck_ignore(alloc),
                instr::null(alloc),
                instr::contenter(alloc),
            ],
        ),
        "valid" => instr::contvalid(alloc),
        "current" => instr::contcurrent(alloc),
        "key" => instr::contkey(alloc),
        "getReturn" => instr::contgetreturn(alloc),
        _ => {
            return Err(raise_fatal_runtime(
                &Pos::make_none(),
                "incorrect native generator function",
            ));
        }
    };
    Ok(InstrSeq::gather(alloc, vec![instrs, instr::retc(alloc)]))
}

fn get_first_param_name(params: &[ast::FunParam]) -> Result<&str> {
    match params {
        [p, ..] => Ok(&p.name),
        _ => Err(Unrecoverable(String::from(
            "native generator requires params",
        ))),
    }
}
