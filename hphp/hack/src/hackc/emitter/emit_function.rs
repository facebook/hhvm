// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod emit_memoize_function;

use ast_scope::{self as ast_scope, Scope, ScopeItem};
use emit_body::{self as emit_body};
use env::emitter::Emitter;
use ffi::{Slice, Str};
use hhas_attribute::{self as hhas_attribute, HhasAttribute};
use hhas_coeffects::HhasCoeffects;
use hhas_function::{self as hhas_function, HhasFunction};
use hhas_pos::HhasSpan;
use hhbc_id::{class::ClassType, function::FunctionType, Id};
use instruction_sequence::{instr, Result};
use naming_special_names_rust::user_attributes as ua;
use ocamlrep::rc::RcOc;
use oxidized::{ast, ast_defs};

use itertools::Either;

pub fn emit_function<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    fd: &'a ast::FunDef,
) -> Result<Vec<HhasFunction<'arena>>> {
    use ast_defs::FunKind;
    use hhas_function::HhasFunctionFlags;

    let alloc = e.alloc;
    let f = &fd.fun;
    let original_id = FunctionType::from_ast_name(alloc, &f.name.1);
    let mut flags = HhasFunctionFlags::empty();
    flags.set(
        HhasFunctionFlags::ASYNC,
        matches!(f.fun_kind, FunKind::FAsync | FunKind::FAsyncGenerator),
    );

    let mut attrs: Vec<HhasAttribute<'arena>> = emit_attribute::from_asts(e, &f.user_attributes)?;
    attrs.extend(emit_attribute::add_reified_attribute(alloc, &f.tparams));
    let memoized = attrs
        .iter()
        .any(|a| ua::is_memoized(a.name.unsafe_as_str()));
    flags.set(HhasFunctionFlags::MEMOIZE_IMPL, memoized);
    flags.set(
        HhasFunctionFlags::NO_INJECTION,
        hhas_attribute::is_no_injection(&attrs),
    );

    let renamed_id = {
        if memoized {
            FunctionType::add_suffix(alloc, &original_id, emit_memoize_helpers::MEMOIZE_SUFFIX)
        } else {
            original_id
        }
    };
    flags.set(
        HhasFunctionFlags::INTERCEPTABLE,
        emit_memoize_function::is_interceptable(e.options()),
    );
    let is_meth_caller = f.name.1.starts_with("\\MethCaller$");
    let call_context = if is_meth_caller {
        match &f.user_attributes[..] {
            [ast::UserAttribute {
                name: ast_defs::Id(_, ref s),
                params,
            }] if s == "__MethCaller" => match &params[..] {
                [ast::Expr(_, _, ast::Expr_::String(ref ctx))] if !ctx.is_empty() => Some(
                    ClassType::from_ast_name(
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
        [ast::UserAttribute { name, params }]
            if name.1 == "__DebuggerMain" && params.is_empty() =>
        {
            true
        }
        _ => false,
    };
    let mut scope = Scope::toplevel();
    if !is_debug_main {
        scope.push_item(ScopeItem::Function(ast_scope::Fun::new_ref(fd)));
    }

    let mut coeffects =
        HhasCoeffects::from_ast(alloc, f.ctxs.as_ref(), &f.params, &f.tparams, vec![]);
    if is_meth_caller {
        coeffects = coeffects.with_caller(alloc)
    }
    if e.systemlib()
        && (f.name.1 == "\\HH\\Coeffects\\backdoor"
            || f.name.1 == "\\HH\\Coeffects\\backdoor_async")
    {
        coeffects = coeffects.with_backdoor(alloc)
    }
    let ast_body = &f.body.fb_ast;
    let deprecation_info = hhas_attribute::deprecation_info(attrs.iter());
    let (body, is_gen, is_pair_gen) = {
        let deprecation_info = if memoized { None } else { deprecation_info };
        let native = attrs.iter().any(|a| ua::is_native(a.name.unsafe_as_str()));
        use emit_body::{Args as EmitBodyArgs, Flags as EmitBodyFlags};
        let mut body_flags = EmitBodyFlags::empty();
        body_flags.set(
            EmitBodyFlags::ASYNC,
            flags.contains(HhasFunctionFlags::ASYNC),
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
    let is_readonly_return = f.readonly_ret.is_some();
    flags.set(HhasFunctionFlags::GENERATOR, is_gen);
    flags.set(HhasFunctionFlags::PAIR_GENERATOR, is_pair_gen);
    flags.set(HhasFunctionFlags::READONLY_RETURN, is_readonly_return);
    let memoize_wrapper = if memoized {
        Some(emit_memoize_function::emit_wrapper_function(
            e,
            original_id,
            &renamed_id,
            deprecation_info,
            fd,
        )?)
    } else {
        None
    };
    let normal_function = HhasFunction {
        attributes: Slice::fill_iter(alloc, attrs.into_iter()),
        name: FunctionType(Str::new_str(alloc, renamed_id.to_raw_string())),
        span: HhasSpan::from_pos(&f.span),
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

pub fn emit_functions_from_program<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    ast: &'a [ast::Def],
) -> Result<Vec<HhasFunction<'arena>>> {
    Ok(ast
        .iter()
        .filter_map(|d| d.as_fun().map(|f| emit_function(e, f)))
        .collect::<Result<Vec<Vec<_>>>>()?
        .into_iter()
        .flatten()
        .collect::<Vec<_>>())
}
