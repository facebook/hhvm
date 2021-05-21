// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod emit_memoize_function;

use hhbc_by_ref_ast_scope::{self as ast_scope, Scope, ScopeItem};
use hhbc_by_ref_emit_attribute as emit_attribute;
use hhbc_by_ref_emit_body::{self as emit_body};
use hhbc_by_ref_emit_memoize_helpers as emit_memoize_helpers;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_attribute::{self as hhas_attribute, HhasAttribute};
use hhbc_by_ref_hhas_coeffects::HhasCoeffects;
use hhbc_by_ref_hhas_function::{self as hhas_function, HhasFunction};
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhbc_id::{self as hhbc_id, Id};
use hhbc_by_ref_instruction_sequence::{instr, Result};
use naming_special_names_rust::user_attributes as ua;
use ocamlrep::rc::RcOc;
use oxidized::{ast as tast, ast_defs};

use itertools::Either;

pub fn emit_function<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    fd: &'a tast::FunDef,
) -> Result<Vec<HhasFunction<'arena>>> {
    use ast_defs::FunKind;
    use hhas_function::Flags;

    let f = &fd.fun;
    let original_id = hhbc_id::function::Type::from_ast_name(alloc, &f.name.1);
    let mut flags = Flags::empty();
    flags.set(
        hhas_function::Flags::ASYNC,
        matches!(f.fun_kind, FunKind::FAsync | FunKind::FAsyncGenerator),
    );

    let mut attrs: Vec<HhasAttribute<'arena>> =
        emit_attribute::from_asts(alloc, e, &f.user_attributes)?;
    attrs.extend(emit_attribute::add_reified_attribute(&f.tparams));
    let memoized = attrs.iter().any(|a| ua::is_memoized(&a.name));
    flags.set(Flags::MEMOIZE_IMPL, memoized);
    flags.set(Flags::NO_INJECTION, hhas_attribute::is_no_injection(&attrs));

    let renamed_id = {
        if memoized {
            hhbc_id::function::Type::add_suffix(
                alloc,
                &original_id,
                emit_memoize_helpers::MEMOIZE_SUFFIX,
            )
        } else {
            original_id
        }
    };
    flags.set(
        Flags::INTERCEPTABLE,
        emit_memoize_function::is_interceptable(e.options()),
    );
    let is_meth_caller = f.name.1.starts_with("\\MethCaller$");
    let call_context = if is_meth_caller {
        match &f.user_attributes[..] {
            [tast::UserAttribute {
                name: ast_defs::Id(_, ref s),
                params,
            }] if s == "__MethCaller" => match &params[..] {
                [tast::Expr(_, tast::Expr_::String(ref ctx))] if !ctx.is_empty() => Some(
                    hhbc_id::class::Type::from_ast_name(
                        alloc,
                        // FIXME: This is not safe--string literals are binary strings.
                        // There's no guarantee that they're valid UTF-8.
                        unsafe { std::str::from_utf8_unchecked(ctx.as_slice()) },
                    )
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
    let is_debug_main = match f.user_attributes.as_slice() {
        [tast::UserAttribute { name, params }]
            if name.1 == "__DebuggerMain" && params.is_empty() =>
        {
            true
        }
        _ => false,
    };
    let mut scope = Scope::toplevel();
    if !is_debug_main {
        scope.push_item(ScopeItem::Function(ast_scope::Fun::new_ref(&fd)));
    }

    let mut coeffects = HhasCoeffects::from_ast(&f.ctxs, &f.params, &f.tparams, vec![]);
    if is_meth_caller {
        coeffects = coeffects.with_caller()
    }
    let ast_body = &f.body.ast;
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
        body_flags.set(EmitBodyFlags::NATIVE, native);
        body_flags.set(EmitBodyFlags::MEMOIZE, memoized);
        body_flags.set(
            EmitBodyFlags::SKIP_AWAITABLE,
            f.fun_kind == ast_defs::FunKind::FAsync,
        );
        body_flags.set(
            EmitBodyFlags::HAS_COEFFECTS_LOCAL,
            coeffects.has_coeffects_local(),
        );

        emit_body::emit_body(
            alloc,
            e,
            RcOc::clone(&fd.namespace),
            Either::Right(ast_body),
            instr::null(alloc),
            scope,
            EmitBodyArgs {
                flags: body_flags,
                deprecation_info: &deprecation_info,
                default_dropthrough: None,
                doc_comment: f.doc_comment.clone(),
                pos: &f.span,
                ret: f.ret.1.as_ref(),
                ast_params: &f.params,
                call_context,
                immediate_tparams: &f.tparams,
                class_tparam_names: &[],
            },
        )?
    };
    flags.set(hhas_function::Flags::GENERATOR, is_gen);
    flags.set(hhas_function::Flags::PAIR_GENERATOR, is_pair_gen);
    let memoize_wrapper = if memoized {
        Some(emit_memoize_function::emit_wrapper_function(
            alloc,
            e,
            original_id,
            &renamed_id,
            &deprecation_info,
            &fd,
        )?)
    } else {
        None
    };
    let normal_function = HhasFunction {
        attributes: attrs,
        name: (alloc, renamed_id.to_raw_string()).into(),
        span: Span::from_pos(&f.span),
        coeffects,
        body,
        flags,
    };

    Ok(if let Some(memoize_wrapper) = memoize_wrapper {
        vec![normal_function, memoize_wrapper]
    } else {
        vec![normal_function]
    })
}

pub fn emit_functions_from_program<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    tast: &'a [tast::Def],
) -> Result<Vec<HhasFunction<'arena>>> {
    Ok(tast
        .iter()
        .filter_map(|d| d.as_fun().map(|f| emit_function(alloc, e, f)))
        .collect::<Result<Vec<Vec<_>>>>()?
        .into_iter()
        .flatten()
        .collect::<Vec<_>>())
}
