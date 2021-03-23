// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_scope_rust::Scope;
use emit_body_rust as emit_body;
use emit_fatal_rust as emit_fatal;
use emit_param_rust as emit_param;
use env::emitter::Emitter;
use hhas_body_rust::HhasBody;
use instruction_sequence::{instr, Error::Unrecoverable, InstrSeq, Result};
use oxidized::{aast, ast as tast, namespace_env, pos::Pos};

pub fn emit_body<'a>(
    emitter: &mut Emitter,
    scope: &Scope,
    namespace: &namespace_env::Env,
    class_attrs: &[tast::UserAttribute],
    name: &tast::Sid,
    params: &[tast::FunParam],
    ret: Option<&aast::Hint>,
) -> Result<HhasBody> {
    let body_instrs = emit_native_opcode_impl(&name.1, params, class_attrs);
    let mut tparams = scope
        .get_tparams()
        .iter()
        .map(|tp| tp.name.1.as_str())
        .collect::<Vec<_>>();
    let params = emit_param::from_asts(emitter, &mut tparams, namespace, false, scope, params);
    let return_type_info = emit_body::emit_return_type_info(tparams.as_slice(), false, ret);

    body_instrs.and_then(|body_instrs| {
        params.and_then(|params| {
            return_type_info.and_then(|rti| {
                Ok(HhasBody {
                    body_instrs,
                    params,
                    return_type_info: Some(rti),
                    ..HhasBody::default()
                })
            })
        })
    })
}

fn emit_native_opcode_impl(
    name: &str,
    params: &[tast::FunParam],
    user_attrs: &[tast::UserAttribute],
) -> Result {
    if let [ua] = user_attrs {
        if ua.name.1 == "__NativeData" {
            if let [p] = ua.params.as_slice() {
                match p.1.as_string() {
                    Some(s) if s == "HH\\AsyncGenerator" || s == "Generator" => {
                        return emit_generator_method(name, params);
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

fn emit_generator_method(name: &str, params: &[tast::FunParam]) -> Result {
    let instrs = match name {
        "send" => {
            let local = local::Type::Named((get_first_param_name(params)?).to_string());
            InstrSeq::gather(vec![
                instr::contcheck_check(),
                instr::pushl(local),
                instr::contenter(),
            ])
        }
        "raise" | "throw" => {
            let local = local::Type::Named((get_first_param_name(params)?).to_string());
            InstrSeq::gather(vec![
                instr::contcheck_check(),
                instr::pushl(local),
                instr::contraise(),
            ])
        }
        "next" | "rewind" => InstrSeq::gather(vec![
            instr::contcheck_ignore(),
            instr::null(),
            instr::contenter(),
        ]),
        "valid" => instr::contvalid(),
        "current" => instr::contcurrent(),
        "key" => instr::contkey(),
        "getReturn" => instr::contgetreturn(),
        _ => {
            return Err(emit_fatal::raise_fatal_runtime(
                &Pos::make_none(),
                String::from("incorrect native generator function"),
            ));
        }
    };
    Ok(InstrSeq::gather(vec![instrs, instr::retc()]))
}

fn get_first_param_name(params: &[tast::FunParam]) -> Result<&str> {
    match params {
        [p, ..] => Ok(&p.name),
        _ => Err(Unrecoverable(String::from(
            "native generator requires params",
        ))),
    }
}
