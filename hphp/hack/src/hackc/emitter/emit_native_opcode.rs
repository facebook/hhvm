// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_scope::Scope;
use env::emitter::Emitter;
use error::Error;
use error::Result;
use ffi::Maybe;
use hhbc::Attr;
use hhbc::Attribute;
use hhbc::Body;
use hhbc::Coeffects;
use hhbc::Local;
use hhbc::ParamEntry;
use instruction_sequence::InstrSeq;
use instruction_sequence::instr;
use oxidized::aast;
use oxidized::ast;
use oxidized::pos::Pos;

use crate::emit_body;
use crate::emit_param;

pub fn emit_body<'a, 'd>(
    emitter: &mut Emitter<'d>,
    scope: &Scope<'a>,
    class_name: &ast::Sid,
    class_attrs: &[ast::UserAttribute],
    name: &ast::Sid,
    params: &[ast::FunParam],
    attributes: Vec<Attribute>,
    attrs: Attr,
    coeffects: Coeffects,
    ret: Option<&aast::Hint>,
) -> Result<Body> {
    let instrs = emit_native_opcode_impl(&name.1, params, &class_name.1, class_attrs)?;
    let mut tparams = scope
        .get_tparams()
        .iter()
        .map(|tp| tp.name.1.as_str())
        .collect::<Vec<_>>();
    let params = emit_param::from_asts(emitter, &mut tparams, false, scope, params)?;
    let rti = emit_body::emit_return_type(tparams.as_slice(), false, ret)?;

    let instrs = Vec::from_iter(instrs.iter().cloned());
    let params = Vec::from_iter(params.into_iter().map(|(param, _)| ParamEntry {
        param,
        dv: Maybe::Nothing,
    }));
    let stack_depth =
        stack_depth::compute_stack_depth(&params, &instrs).map_err(error::Error::from_error)?;

    Ok(Body {
        attributes: attributes.into(),
        attrs,
        coeffects,
        return_type: Maybe::Just(rti),
        doc_comment: Default::default(),
        is_memoize_wrapper: Default::default(),
        is_memoize_wrapper_lsb: Default::default(),
        num_iters: Default::default(),
        shadowed_tparams: Default::default(),
        upper_bounds: Default::default(),
        span: Default::default(),
        repr: hhbc::BcRepr {
            instrs: instrs.into(),
            params: params.into(),
            decl_vars: Default::default(),
            stack_depth,
        },
    })
}

fn emit_native_opcode_impl(
    name: &str,
    params: &[ast::FunParam],
    class_name: &str,
    class_user_attrs: &[ast::UserAttribute],
) -> Result<InstrSeq> {
    if let [ua] = class_user_attrs {
        if ua.name.1 == "__NativeData"
            && (class_name == "\\HH\\AsyncGenerator" || class_name == "\\Generator")
        {
            return emit_generator_method(name, params);
        }
    };
    Err(Error::fatal_runtime(
        &Pos::NONE,
        format!(
            "OpCodeImpl attribute is not applicable to {} in {}",
            name, class_name
        ),
    ))
}

fn emit_generator_method(name: &str, params: &[ast::FunParam]) -> Result<InstrSeq> {
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
                &Pos::NONE,
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
