// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::sync::Arc;

use ast_scope::Scope;
use ast_scope::ScopeItem;
use env::emitter::Emitter;
use error::Result;
use ffi::Str;
use hhbc::ClassName;
use hhbc::Coeffects;
use hhbc::Function;
use hhbc::FunctionName;
use hhbc::Span;
use instruction_sequence::instr;
use naming_special_names_rust::user_attributes as ua;
use oxidized::ast;
use oxidized::ast_defs;

use crate::emit_attribute;
use crate::emit_body;
use crate::emit_memoize_function;
use crate::emit_memoize_helpers;
use crate::emit_param;

pub fn emit_function<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    fd: &'a ast::FunDef,
) -> Result<Vec<Function<'arena>>> {
    use ast_defs::FunKind;
    use hhbc::FunctionFlags;

    let alloc = e.alloc;
    let f = &fd.fun;
    let original_id = FunctionName::from_ast_name(alloc, &fd.name.1);
    let mut flags = FunctionFlags::empty();
    flags.set(
        FunctionFlags::ASYNC,
        matches!(f.fun_kind, FunKind::FAsync | FunKind::FAsyncGenerator),
    );
    let mut user_attrs = emit_attribute::from_asts(e, &f.user_attributes)?;
    user_attrs.extend(emit_attribute::add_reified_attribute(&fd.tparams));
    let memoized = user_attrs.iter().any(|a| ua::is_memoized(a.name.as_str()));
    flags.set(FunctionFlags::MEMOIZE_IMPL, memoized);

    let renamed_id = {
        if memoized {
            FunctionName::add_suffix(alloc, &original_id, emit_memoize_helpers::MEMOIZE_SUFFIX)
        } else {
            original_id
        }
    };
    let is_meth_caller = fd.name.1.starts_with("\\MethCaller$");
    let call_context = if is_meth_caller {
        match &f.user_attributes[..] {
            [
                ast::UserAttribute {
                    name: ast_defs::Id(_, ref s),
                    params,
                },
            ] if s == "__MethCaller" => match &params[..] {
                [ast::Expr(_, _, ast::Expr_::String(ref ctx))] if !ctx.is_empty() => Some(
                    ClassName::from_ast_name_and_mangle(
                        alloc,
                        // FIXME: This is not safe--string literals are binary strings.
                        // There's no guarantee that they're valid UTF-8.
                        unsafe { std::str::from_utf8_unchecked(ctx.as_slice()) },
                    )
                    .unsafe_as_str()
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
    let mut scope = Scope::default();
    if !is_debug_main {
        scope.push_item(ScopeItem::Function(ast_scope::Fun::new_ref(fd)));
    }

    let mut coeffects = Coeffects::from_ast(alloc, f.ctxs.as_ref(), &f.params, &fd.tparams, vec![]);
    if is_meth_caller {
        coeffects = coeffects.with_caller()
    }
    if e.systemlib()
        && (fd.name.1 == "\\HH\\Coeffects\\backdoor"
            || fd.name.1 == "\\HH\\Coeffects\\backdoor_async")
    {
        coeffects = coeffects.with_backdoor()
    }
    if e.systemlib()
        && (fd.name.1 == "\\HH\\Coeffects\\fb\\backdoor_to_globals_leak_safe__DO_NOT_USE")
    {
        coeffects = coeffects.with_backdoor_globals_leak_safe()
    }
    let ast_body = &f.body.fb_ast;
    let deprecation_info = hhbc::deprecation_info(user_attrs.iter());
    let (body, is_gen, is_pair_gen) = {
        let deprecation_info = if memoized { None } else { deprecation_info };
        let native = user_attrs.iter().any(|a| ua::is_native(a.name.as_str()));
        use emit_body::Args as EmitBodyArgs;
        use emit_body::Flags as EmitBodyFlags;
        let mut body_flags = EmitBodyFlags::empty();
        body_flags.set(EmitBodyFlags::ASYNC, flags.contains(FunctionFlags::ASYNC));
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
            Arc::clone(&fd.namespace),
            ast_body,
            instr::null(),
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
                immediate_tparams: &fd.tparams,
                class_tparam_names: &[],
            },
        )?
    };
    flags.set(FunctionFlags::GENERATOR, is_gen);
    flags.set(FunctionFlags::PAIR_GENERATOR, is_pair_gen);
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
    let has_variadic = emit_param::has_variadic(&body.params);
    let attrs =
        emit_memoize_function::get_attrs_for_fun(e, fd, &user_attrs, memoized, has_variadic);
    let normal_function = Function {
        attributes: user_attrs.into(),
        name: FunctionName::new(Str::new_str(alloc, renamed_id.unsafe_as_str())),
        span: Span::from_pos(&f.span),
        coeffects,
        body,
        flags,
        attrs,
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
) -> Result<Vec<Function<'arena>>> {
    Ok(ast
        .iter()
        .filter_map(|d| d.as_fun().map(|f| emit_function(e, f)))
        .collect::<Result<Vec<Vec<_>>>>()?
        .into_iter()
        .flatten()
        .collect::<Vec<_>>())
}
