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

fn emit_constant_cinit<'a, 'decl>(
    e: &mut Emitter<'decl>,
    env: &mut Env<'a>,
    constant: &'a ast::Gconst,
    init: Option<InstrSeq>,
) -> Result<Option<Function>> {
    let const_name = hhbc::ConstName::from_ast_name(&constant.name.1);
    let (ns, name) = utils::split_ns_from_name(const_name.as_str());
    let name = String::new() + strip_global_ns(ns) + "86cinit_" + name;
    let original_name = hhbc::FunctionName::intern(&name);
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
    init.map(|instrs| {
        let verify_instr = match return_type_info {
            None => instr::empty(),
            Some(_) => instr::verify_ret_type_c(),
        };
        let instrs = InstrSeq::gather(vec![instrs, verify_instr, instr::ret_c()]);
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
            Some(env),
        )?;
        let mut attrs = Attr::AttrNoInjection;
        attrs.set(Attr::AttrPersistent, e.systemlib());
        attrs.set(Attr::AttrBuiltin, e.systemlib());

        Ok(Function {
            attributes: Default::default(),
            name: original_name,
            body,
            span: Span::from_pos(&constant.span),
            coeffects: Coeffects::default(),
            flags: FunctionFlags::empty(),
            attrs,
        })
    })
    .transpose()
}

fn emit_constant<'a, 'decl>(
    e: &mut Emitter<'decl>,
    env: &mut Env<'a>,
    constant: &'a ast::Gconst,
) -> Result<(Constant, Option<Function>)> {
    let (c, init) = from_ast(e, env, &constant.name, false, Some(&constant.value))?;
    let f = emit_constant_cinit(e, env, constant, init)?;
    Ok((c, f))
}

pub fn emit_constants_from_program<'a, 'decl>(
    e: &mut Emitter<'decl>,
    env: &mut Env<'a>,
    defs: &'a [ast::Def],
) -> Result<(Vec<Constant>, Vec<Function>)> {
    let const_tuples = defs
        .iter()
        .filter_map(|d| d.as_constant().map(|c| emit_constant(e, env, c)))
        .collect::<Result<Vec<_>>>()?;
    let (contants, inits): (Vec<_>, Vec<_>) = const_tuples.into_iter().unzip();
    Ok((contants, inits.into_iter().flatten().collect()))
}

pub fn from_ast<'a, 'decl>(
    emitter: &mut Emitter<'decl>,
    env: &Env<'a>,
    id: &'a ast::Id,
    is_abstract: bool,
    expr: Option<&ast::Expr>,
) -> Result<(Constant, Option<InstrSeq>)> {
    let (value, initializer_instrs) = match expr {
        None => (None, None),
        Some(init) => match constant_folder::expr_to_typed_value(emitter, &env.scope, init) {
            Ok(v) => (Some(v), None),
            Err(_) => (
                Some(TypedValue::Uninit),
                Some(emit_expression::emit_expr(emitter, env, init)?),
            ),
        },
    };
    let mut attrs = Attr::AttrNone;
    attrs.set(Attr::AttrPersistent, emitter.systemlib());
    attrs.set(Attr::AttrAbstract, is_abstract);

    let constant = Constant {
        name: hhbc::ConstName::from_ast_name(id.name()),
        value: Maybe::from(value),
        attrs,
    };
    Ok((constant, initializer_instrs))
}
