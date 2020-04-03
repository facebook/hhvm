// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod emit_memoize_function;

use ast_scope_rust::{Scope, ScopeItem};
use closure_convert_rust::HoistKind;
use emit_attribute_rust as emit_attribute;
use emit_body_rust::{self as emit_body};
use emit_memoize_helpers_rust as emit_memoize_helpers;
use env::emitter::Emitter;
use hhas_attribute_rust::{self as hhas_attribute, HhasAttribute};
use hhas_function_rust::{self as hhas_function, HhasFunction};
use hhas_pos_rust::Span;
use hhbc_id_rust::{self as hhbc_id, Id};
use instruction_sequence_rust::{instr, Result};
use naming_special_names_rust::user_attributes as ua;
use options::HhvmFlags;
use oxidized::{aast as a, ast as tast, ast_defs, pos::Pos};
use rx_rust as rx;

use std::borrow::Cow;

pub fn emit_function<'a>(
    e: &mut Emitter,
    f: &'a tast::Fun_,
    hoisted: HoistKind,
) -> Result<Vec<HhasFunction<'a>>> {
    use ast_defs::FunKind;
    use hhas_function::Flags;
    let original_id = hhbc_id::function::Type::from_ast_name(&f.name.1);
    let mut flags = Flags::empty();
    flags.set(
        hhas_function::Flags::ASYNC,
        match f.fun_kind {
            FunKind::FAsync | FunKind::FAsyncGenerator => true,
            _ => false,
        },
    );

    let mut attrs: Vec<HhasAttribute> =
        emit_attribute::from_asts(e, &f.namespace, &f.user_attributes)?;
    attrs.extend(emit_attribute::add_reified_attribute(&f.tparams));
    let memoized = attrs.iter().any(|a| ua::is_memoized(&a.name));
    flags.set(Flags::MEMOIZE_IMPL, memoized);
    flags.set(Flags::NO_INJECTION, hhas_attribute::is_no_injection(&attrs));

    let renamed_id = {
        let mut id = original_id.clone();
        if memoized {
            id.add_suffix(emit_memoize_helpers::MEMOIZE_SUFFIX);
        }
        id
    };
    flags.set(
        Flags::INTERCEPTABLE,
        emit_memoize_function::is_interceptable(original_id.clone(), e.options()),
    );
    let is_meth_caller = f.name.1.starts_with("\\MethCaller$");
    let call_context = if is_meth_caller {
        match &f.user_attributes[..] {
            [tast::UserAttribute {
                name: ast_defs::Id(_, ref s),
                params,
            }] if s == "__MethCaller" => match &params[..] {
                [tast::Expr(_, tast::Expr_::String(ref ctx))] if !ctx.is_empty() => Some(
                    hhbc_id::class::Type::from_ast_name(ctx)
                        .to_raw_string()
                        .into(),
                ),
                _ => None,
            },
            _ => None,
        }
    } else {
        None
    };
    let mut scope = Scope::toplevel();
    scope.push_item(ScopeItem::Function(Cow::Borrowed(&f)));

    let rx_level = rx::Level::from_ast(&f.user_attributes).unwrap_or(rx::Level::NonRx);
    let (ast_body, rx_body) = {
        if rx_level != rx::Level::NonRx {
            match rx::halves_of_is_enabled_body(&f.body) {
                Some((enabled_body, disabled_body)) => {
                    if e.options().hhvm.flags.contains(HhvmFlags::RX_IS_ENABLED) {
                        (enabled_body, true)
                    } else {
                        flags.insert(hhas_function::Flags::RX_DISABLED);
                        (disabled_body, false)
                    }
                }
                None => (&f.body.ast, true),
            }
        } else {
            (&f.body.ast, false)
        }
    };

    let deprecation_info = hhas_attribute::deprecation_info(attrs.iter());
    let (body, is_gen, is_pair_gen) = {
        let deprecation_info = if memoized { None } else { deprecation_info };
        let native = attrs.iter().any(|a| ua::is_native(&a.name));
        use emit_body::{Args as EmitBodyArgs, Flags as EmitBodyFlags};
        let mut body_flags = EmitBodyFlags::empty();
        body_flags.set(
            EmitBodyFlags::ASYNC,
            flags.contains(hhas_function::Flags::ASYNC),
        );
        body_flags.set(EmitBodyFlags::RX_BODY, rx_body);
        body_flags.set(EmitBodyFlags::NATIVE, native);
        body_flags.set(EmitBodyFlags::MEMOIZE, memoized);
        body_flags.set(
            EmitBodyFlags::SKIP_AWAITABLE,
            f.fun_kind == ast_defs::FunKind::FAsync,
        );
        emit_body::emit_body(
            e,
            &f.namespace,
            &vec![a::Def::mk_stmt(a::Stmt(
                Pos::make_none(),
                // TODO(shiqicao): T62049979 we should avoid cloning
                a::Stmt_::Block(ast_body.clone()),
            ))],
            instr::null(),
            EmitBodyArgs {
                flags: body_flags,
                deprecation_info: &deprecation_info,
                default_dropthrough: None,
                doc_comment: f.doc_comment.clone(),
                pos: &f.span,
                scope: &scope,
                ret: f.ret.1.as_ref(),
                ast_params: &f.params,
                call_context,
                immediate_tparams: &f.tparams,
            },
        )?
    };
    flags.set(hhas_function::Flags::GENERATOR, is_gen);
    flags.set(hhas_function::Flags::PAIR_GENERATOR, is_pair_gen);
    let memoize_wrapper = if memoized {
        Some(emit_memoize_function::emit_wrapper_function(
            original_id,
            &renamed_id,
            &deprecation_info,
            &f,
        ))
    } else {
        None
    };
    let name: String = renamed_id.into();
    let normal_function = HhasFunction {
        attributes: attrs,
        name: name.into(),
        span: Span::from_pos(&f.span),
        rx_level,
        body,
        hoisted,
        flags,
    };

    Ok(if let Some(memoize_wrapper) = memoize_wrapper {
        vec![normal_function, memoize_wrapper]
    } else {
        vec![normal_function]
    })
}

pub fn emit_functions_from_program<'a>(
    e: &mut Emitter,
    hoist_kinds: &[HoistKind],
    prog: &'a tast::Program,
) -> Result<Vec<HhasFunction<'a>>> {
    Ok(hoist_kinds
        .into_iter()
        .zip(prog)
        .filter_map(|(hoist_kind, d)| d.as_fun().map(|f| emit_function(e, f, *hoist_kind)))
        .collect::<Result<Vec<Vec<_>>>>()?
        .into_iter()
        .flatten()
        .collect::<Vec<_>>())
}
