// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::{Maybe::Just, Slice, Str};
use hhbc_by_ref_ast_scope::Scope;
use hhbc_by_ref_emit_body as emit_body;
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_emit_fatal::raise_fatal_runtime;
use hhbc_by_ref_emit_param as emit_param;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_body::HhasBody;
use hhbc_by_ref_instruction_sequence::{instr, Error::Unrecoverable, InstrSeq, Result};
use hhbc_by_ref_local::Local;
use oxidized::{aast, ast, pos::Pos};

pub fn emit_body<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    scope: &Scope<'a, 'arena>,
    class_attrs: &[ast::UserAttribute],
    name: &ast::Sid,
    params: &[ast::FunParam],
    ret: Option<&aast::Hint>,
) -> Result<HhasBody<'arena>> {
    let body_instrs = emit_native_opcode_impl(alloc, &name.1, params, class_attrs);
    let mut tparams = scope
        .get_tparams()
        .iter()
        .map(|tp| tp.name.1.as_str())
        .collect::<Vec<_>>();
    let params = emit_param::from_asts(alloc, emitter, &mut tparams, false, scope, params);
    let return_type_info = emit_body::emit_return_type_info(alloc, tparams.as_slice(), false, ret);

    body_instrs.and_then(|body_instrs| {
        params.and_then(|params| {
            return_type_info.map(|rti| {
                let mut body = hhbc_by_ref_hhas_body::default_with_body_instrs(body_instrs);
                body.params = Slice::fill_iter(alloc, params.into_iter().map(|p| p.0));
                body.return_type_info = Just(rti);
                body
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
