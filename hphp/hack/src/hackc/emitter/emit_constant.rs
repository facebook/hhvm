// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use core_utils_rust as utils;
use emit_type_hint::Kind;
use env::emitter::Emitter;
use env::Env;
use error::Result;
use ffi::Maybe;
use ffi::Slice;
use ffi::Str;
use hhbc::Coeffects;
use hhbc::Constant;
use hhbc::Function;
use hhbc::FunctionFlags;
use hhbc::Span;
use hhbc::TypedValue;
use hhbc_string_utils::strip_global_ns;
use hhvm_types_ffi::ffi::Attr;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::ast;

use crate::emit_body;
use crate::emit_expression;

fn emit_constant_cinit<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    constant: &'a ast::Gconst,
    init: Option<InstrSeq<'arena>>,
) -> Result<Option<Function<'arena>>> {
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
        let instrs = InstrSeq::gather(vec![instrs, verify_instr, instr::ret_c()]);
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
        Ok(Function {
            attributes: Slice::empty(),
            name: original_name,
            body,
            span: Span::from_pos(&constant.span),
            coeffects: Coeffects::default(),
            flags: FunctionFlags::empty(),
            attrs: Attr::AttrNoInjection,
        })
    })
    .transpose()
}

fn emit_constant<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    constant: &'a ast::Gconst,
) -> Result<(Constant<'arena>, Option<Function<'arena>>)> {
    let (c, init) = from_ast(e, env, &constant.name, false, Some(&constant.value))?;
    let f = emit_constant_cinit(e, env, constant, init)?;
    Ok((c, f))
}

pub fn emit_constants_from_program<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    defs: &'a [ast::Def],
) -> Result<(Vec<Constant<'arena>>, Vec<Function<'arena>>)> {
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
) -> Result<(Constant<'arena>, Option<InstrSeq<'arena>>)> {
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
    let constant = Constant {
        name: hhbc::ConstName::from_ast_name(alloc, id.name()),
        value: Maybe::from(value),
        is_abstract,
    };
    Ok((constant, initializer_instrs))
}
