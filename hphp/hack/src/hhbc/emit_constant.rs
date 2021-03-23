// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use core_utils_rust as utils;
use emit_body_rust as emit_body;
use emit_type_hint_rust::{self as emit_type_hint, Kind};
use env::{emitter::Emitter, Env};
use hhas_coeffects::HhasCoeffects;
use hhas_constant_rust::{self as hhas_constant, HhasConstant};
use hhas_function_rust::{self as hhas_function, HhasFunction};
use hhas_pos_rust::Span;
use hhbc_id_rust::{r#const, function, Id};
use hhbc_string_utils_rust::strip_global_ns;
use instruction_sequence::{instr, InstrSeq, Result};
use oxidized::ast as tast;

fn emit_constant_cinit<'a>(
    e: &mut Emitter,
    env: &mut Env<'a>,
    constant: &'a tast::Gconst,
    c: &HhasConstant<'a>,
) -> Result<Option<HhasFunction<'a>>> {
    let const_id = r#const::Type::from_ast_name(&constant.name.1);
    let (ns, name) = utils::split_ns_from_name(const_id.to_raw_string());
    let name = String::new() + strip_global_ns(ns) + "86cinit_" + name;
    let original_id: function::Type = name.into();
    let ret = constant.type_.as_ref();
    let return_type_info = ret
        .map(|h| {
            emit_type_hint::hint_to_type_info(
                &Kind::Return,
                false, /* skipawaitable */
                false, /* nullable */
                &[],   /* tparams */
                h,
            )
        })
        .transpose()?;
    c.initializer_instrs
        .as_ref()
        .map(|instrs| {
            let verify_instr = match return_type_info {
                None => instr::empty(),
                Some(_) => instr::verify_ret_type_c(),
            };
            let instrs = InstrSeq::gather(vec![instrs.clone(), verify_instr, instr::retc()]);
            let body = emit_body::make_body(
                e,
                instrs,
                vec![],
                false,  /* is_memoize_wrapper */
                false,  /* is_memoize_wrapper_lsb */
                vec![], /* upper_bounds */
                vec![], /* shadowed_params */
                vec![], /* params */
                return_type_info,
                None, /* doc_comment */
                Some(&env),
            )?;
            Ok(HhasFunction {
                attributes: vec![],
                name: original_id.into(),
                body,
                span: Span::from_pos(&constant.span),
                coeffects: HhasCoeffects::default(),
                flags: hhas_function::Flags::NO_INJECTION,
            })
        })
        .transpose()
}

fn emit_constant<'a>(
    e: &mut Emitter,
    env: &mut Env<'a>,
    constant: &'a tast::Gconst,
) -> Result<(HhasConstant<'a>, Option<HhasFunction<'a>>)> {
    let c = hhas_constant::from_ast(e, env, &constant.name, Some(&constant.value))?;
    let f = emit_constant_cinit(e, env, constant, &c)?;
    Ok((c, f))
}

pub fn emit_constants_from_program<'a>(
    e: &mut Emitter,
    env: &mut Env<'a>,
    defs: &'a [tast::Def],
) -> Result<(Vec<HhasConstant<'a>>, Vec<HhasFunction<'a>>)> {
    let const_tuples = defs
        .iter()
        .filter_map(|d| d.as_constant().map(|c| emit_constant(e, env, c)))
        .collect::<Result<Vec<_>>>()?;
    let (contants, inits): (Vec<_>, Vec<_>) = const_tuples.into_iter().unzip();
    Ok((contants, inits.into_iter().filter_map(|x| x).collect()))
}
