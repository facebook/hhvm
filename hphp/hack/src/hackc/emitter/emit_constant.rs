// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use core_utils_rust as utils;
use emit_type_hint::{self as emit_type_hint, Kind};
use env::{emitter::Emitter, Env};
use ffi::{Slice, Str};
use hhas_coeffects::HhasCoeffects;
use hhas_constant::{self as hhas_constant, HhasConstant};
use hhas_function::{HhasFunction, HhasFunctionFlags};
use hhas_pos::HhasSpan;
use hhbc_id::{r#const, function, Id};
use hhbc_string_utils::strip_global_ns;
use hhvm_types_ffi::ffi::Attr;
use instruction_sequence::{instr, InstrSeq, Result};
use oxidized::ast;

fn emit_constant_cinit<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    constant: &'a ast::Gconst,
    c: &HhasConstant<'arena>,
) -> Result<Option<HhasFunction<'arena>>> {
    let alloc = env.arena;
    let const_id = r#const::ConstType::from_ast_name(alloc, &constant.name.1);
    let (ns, name) = utils::split_ns_from_name(const_id.to_raw_string());
    let name = String::new() + strip_global_ns(ns) + "86cinit_" + name;
    let original_id = function::FunctionType(Str::new_str(alloc, &name));
    let ret = constant.type_.as_ref();
    let return_type_info = ret
        .map(|h| {
            emit_type_hint::hint_to_type_info(
                alloc,
                &Kind::Return,
                false, /* skipawaitable */
                false, /* nullable */
                &[],   /* tparams */
                h,
            )
        })
        .transpose()?;
    Option::from(c.initializer_instrs.as_ref().map(|instrs| {
        let verify_instr = match return_type_info {
            None => instr::empty(alloc),
            Some(_) => instr::verify_ret_type_c(alloc),
        };
        let instrs = InstrSeq::gather(
            alloc,
            vec![
                InstrSeq::clone(alloc, instrs),
                verify_instr,
                instr::retc(alloc),
            ],
        );
        let body = emit_body::make_body(
            alloc,
            e,
            instrs,
            vec![],
            false,  /* is_memoize_wrapper */
            false,  /* is_memoize_wrapper_lsb */
            0,      /* num closures */
            vec![], /* upper_bounds */
            vec![], /* shadowed_params */
            vec![], /* params */
            return_type_info,
            None, /* doc_comment */
            Some(env),
        )?;
        Ok(HhasFunction {
            attributes: Slice::empty(),
            name: original_id,
            body,
            span: HhasSpan::from_pos(&constant.span),
            coeffects: HhasCoeffects::default(),
            flags: HhasFunctionFlags::empty(),
            attrs: Attr::AttrNoInjection,
        })
    }))
    .transpose()
}

fn emit_constant<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    constant: &'a ast::Gconst,
) -> Result<(HhasConstant<'arena>, Option<HhasFunction<'arena>>)> {
    let c = hhas_constant::from_ast(e, env, &constant.name, false, Some(&constant.value))?;
    let f = emit_constant_cinit(e, env, constant, &c)?;
    Ok((c, f))
}

pub fn emit_constants_from_program<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    defs: &'a [ast::Def],
) -> Result<(Vec<HhasConstant<'arena>>, Vec<HhasFunction<'arena>>)> {
    let const_tuples = defs
        .iter()
        .filter_map(|d| d.as_constant().map(|c| emit_constant(e, env, c)))
        .collect::<Result<Vec<_>>>()?;
    let (contants, inits): (Vec<_>, Vec<_>) = const_tuples.into_iter().unzip();
    Ok((contants, inits.into_iter().filter_map(|x| x).collect()))
}
