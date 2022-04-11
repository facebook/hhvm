// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use core_utils_rust as utils;
use emit_type_hint::Kind;
use env::{emitter::Emitter, Env};
use error::Result;
use ffi::{Maybe, Slice, Str};
use hhbc::{
    hhas_coeffects::HhasCoeffects,
    hhas_constant::HhasConstant,
    hhas_function::{HhasFunction, HhasFunctionFlags},
    hhas_pos::HhasSpan,
    TypedValue,
};
use hhbc_string_utils::strip_global_ns;
use hhvm_types_ffi::ffi::Attr;
use instruction_sequence::{instr, InstrSeq};
use oxidized::ast;

fn emit_constant_cinit<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    constant: &'a ast::Gconst,
    init: Option<InstrSeq<'arena>>,
) -> Result<Option<HhasFunction<'arena>>> {
    let alloc = env.arena;
    let const_name = hhbc::ConstName::from_ast_name(alloc, &constant.name.1);
    let (ns, name) = utils::split_ns_from_name(const_name.unsafe_as_str());
    let name = String::new() + strip_global_ns(ns) + "86cinit_" + name;
    let original_name = hhbc::FunctionName::new(Str::new_str(alloc, &name));
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
    init.map(|instrs| {
        let verify_instr = match return_type_info {
            None => instr::empty(),
            Some(_) => instr::verify_ret_type_c(),
        };
        let instrs = InstrSeq::gather(vec![instrs, verify_instr, instr::retc()]);
        let body = emit_body::make_body(
            alloc,
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
            Some(env),
        )?;
        Ok(HhasFunction {
            attributes: Slice::empty(),
            name: original_name,
            body,
            span: HhasSpan::from_pos(&constant.span),
            coeffects: HhasCoeffects::default(),
            flags: HhasFunctionFlags::empty(),
            attrs: Attr::AttrNoInjection,
        })
    })
    .transpose()
}

fn emit_constant<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    constant: &'a ast::Gconst,
) -> Result<(HhasConstant<'arena>, Option<HhasFunction<'arena>>)> {
    let (c, init) = from_ast(e, env, &constant.name, false, Some(&constant.value))?;
    let f = emit_constant_cinit(e, env, constant, init)?;
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
    Ok((contants, inits.into_iter().flatten().collect()))
}

pub fn from_ast<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    id: &'a ast::Id,
    is_abstract: bool,
    expr: Option<&ast::Expr>,
) -> Result<(HhasConstant<'arena>, Option<InstrSeq<'arena>>)> {
    let alloc = env.arena;
    let (value, initializer_instrs) = match expr {
        None => (None, None),
        Some(init) => match constant_folder::expr_to_typed_value(emitter, init) {
            Ok(v) => (Some(v), None),
            Err(_) => (
                Some(TypedValue::Uninit),
                Some(emit_expression::emit_expr(emitter, env, init)?),
            ),
        },
    };
    let constant = HhasConstant {
        name: hhbc::ConstName::from_ast_name(alloc, id.name()),
        value: Maybe::from(value),
        is_abstract,
    };
    Ok((constant, initializer_instrs))
}
