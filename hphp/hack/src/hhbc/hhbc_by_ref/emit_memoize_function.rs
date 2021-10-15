// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::Slice;
use hhbc_by_ref_ast_scope::{self as ast_scope, Scope, ScopeItem};
use hhbc_by_ref_emit_attribute as emit_attribute;
use hhbc_by_ref_emit_body as emit_body;
use hhbc_by_ref_emit_memoize_helpers as emit_memoize_helpers;
use hhbc_by_ref_emit_param as emit_param;
use hhbc_by_ref_emit_pos::emit_pos_then;
use hhbc_by_ref_env::{emitter::Emitter, Env};
use hhbc_by_ref_hhas_body::HhasBody;
use hhbc_by_ref_hhas_coeffects::HhasCoeffects;
use hhbc_by_ref_hhas_function::{HhasFunction, HhasFunctionFlags};
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_hhas_pos::HhasSpan;
use hhbc_by_ref_hhas_type::HhasTypeInfo;
use hhbc_by_ref_hhbc_ast::{FcallArgs, FcallFlags};
use hhbc_by_ref_hhbc_id::function;
use hhbc_by_ref_hhbc_string_utils::reified;
use hhbc_by_ref_instruction_sequence::{instr, InstrSeq, Result};
use hhbc_by_ref_label::Label;
use hhbc_by_ref_local::Local;
use hhbc_by_ref_options::{HhvmFlags, Options, RepoFlags};
use hhbc_by_ref_runtime::TypedValue;
use ocamlrep::rc::RcOc;
use oxidized::{ast as T, pos::Pos};

pub(crate) fn is_interceptable(opts: &Options) -> bool {
    opts.hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION)
        && !opts.repo_flags.contains(RepoFlags::AUTHORITATIVE)
}

pub(crate) fn emit_wrapper_function<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    original_id: function::FunctionType<'arena>,
    renamed_id: &function::FunctionType<'arena>,
    deprecation_info: &Option<&[TypedValue<'arena>]>,
    fd: &'a T::FunDef,
) -> Result<HhasFunction<'arena>> {
    let f = &fd.fun;
    emit_memoize_helpers::check_memoize_possible(&(f.name).0, &f.params, false)?;
    let scope = Scope {
        items: vec![ScopeItem::Function(ast_scope::Fun::new_ref(fd))],
    };
    let mut tparams = scope
        .get_tparams()
        .iter()
        .map(|tp| tp.name.1.as_str())
        .collect::<Vec<_>>();
    let params = emit_param::from_asts(alloc, emitter, &mut tparams, true, &scope, &f.params)?;
    let mut attributes = emit_attribute::from_asts(alloc, emitter, &f.user_attributes)?;
    attributes.extend(emit_attribute::add_reified_attribute(alloc, &f.tparams));
    let return_type_info = emit_body::emit_return_type_info(
        alloc,
        &tparams,
        f.fun_kind.is_fasync(), /* skip_awaitable */
        f.ret.1.as_ref(),
    )?;
    let is_reified = f
        .tparams
        .iter()
        .any(|tp| tp.reified.is_reified() || tp.reified.is_soft_reified());
    let should_emit_implicit_context = emitter
        .options()
        .hhvm
        .flags
        .contains(HhvmFlags::ENABLE_IMPLICIT_CONTEXT)
        && attributes.iter().any(|a| {
            naming_special_names_rust::user_attributes::is_memoized_policy_sharded(a.name.as_str())
        });
    let mut env = Env::default(alloc, RcOc::clone(&fd.namespace)).with_scope(scope);
    let body_instrs = make_memoize_function_code(
        alloc,
        emitter,
        &mut env,
        &f.span,
        *deprecation_info,
        &params,
        &f.params,
        *renamed_id,
        f.fun_kind.is_fasync(),
        is_reified,
        should_emit_implicit_context,
    )?;
    let coeffects = HhasCoeffects::from_ast(alloc, &f.ctxs, &f.params, &f.tparams, vec![]);
    let body = make_wrapper_body(
        alloc,
        emitter,
        env,
        return_type_info,
        params,
        body_instrs,
        is_reified,
    )?;

    let mut flags = HhasFunctionFlags::empty();
    flags.set(HhasFunctionFlags::ASYNC, f.fun_kind.is_fasync());
    let is_interceptable = is_interceptable(emitter.options());
    flags.set(HhasFunctionFlags::INTERCEPTABLE, is_interceptable);

    Ok(HhasFunction {
        attributes: Slice::fill_iter(alloc, attributes.into_iter()),
        name: original_id,
        body,
        span: HhasSpan::from_pos(&f.span),
        coeffects,
        flags,
    })
}

fn make_memoize_function_code<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    deprecation_info: Option<&[TypedValue<'arena>]>,
    hhas_params: &[(HhasParam<'arena>, Option<(Label, T::Expr)>)],
    ast_params: &[T::FunParam],
    renamed_id: function::FunctionType<'arena>,
    is_async: bool,
    is_reified: bool,
    should_emit_implicit_context: bool,
) -> Result<InstrSeq<'arena>> {
    let fun = if hhas_params.is_empty() && !is_reified && !should_emit_implicit_context {
        make_memoize_function_no_params_code(alloc, e, env, deprecation_info, renamed_id, is_async)
    } else {
        make_memoize_function_with_params_code(
            alloc,
            e,
            env,
            pos,
            deprecation_info,
            hhas_params,
            ast_params,
            renamed_id,
            is_async,
            is_reified,
            should_emit_implicit_context,
        )
    }?;
    Ok(emit_pos_then(alloc, pos, fun))
}

fn make_memoize_function_with_params_code<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    deprecation_info: Option<&[TypedValue<'arena>]>,
    hhas_params: &[(HhasParam<'arena>, Option<(Label, T::Expr)>)],
    ast_params: &[T::FunParam],
    renamed_id: function::FunctionType<'arena>,
    is_async: bool,
    is_reified: bool,
    should_emit_implicit_context: bool,
) -> Result<InstrSeq<'arena>> {
    let param_count = hhas_params.len();
    let notfound = e.label_gen_mut().next_regular();
    let suspended_get = e.label_gen_mut().next_regular();
    let eager_set = e.label_gen_mut().next_regular();
    // The local that contains the reified generics is the first non parameter local,
    // so the first local is parameter count + 1 when there are reified = generics
    let add_reified = usize::from(is_reified);
    let add_implicit_context = usize::from(should_emit_implicit_context);
    let first_local_idx = param_count + add_reified;
    let deprecation_body =
        emit_body::emit_deprecation_info(alloc, &env.scope, deprecation_info, e.systemlib())?;
    let (begin_label, default_value_setters) =
        // Default value setters belong in the wrapper method not in the original method
        emit_param::emit_param_default_value_setter(e, env, pos, hhas_params)?;
    let fcall_args = {
        let mut fcall_flags = FcallFlags::default();
        fcall_flags.set(FcallFlags::HAS_GENERICS, is_reified);
        FcallArgs::new(
            fcall_flags,
            1,
            Slice::empty(),
            Slice::empty(),
            if is_async { Some(eager_set) } else { None },
            param_count,
            None,
        )
    };
    let (reified_get, reified_memokeym) = if !is_reified {
        (instr::empty(alloc), instr::empty(alloc))
    } else {
        (
            instr::cgetl(
                alloc,
                Local::Named(Slice::new(reified::GENERICS_LOCAL_NAME.as_bytes())),
            ),
            InstrSeq::gather(
                alloc,
                emit_memoize_helpers::get_memo_key_list(
                    alloc,
                    param_count,
                    first_local_idx,
                    reified::GENERICS_LOCAL_NAME,
                ),
            ),
        )
    };
    let ic_memokey = if !should_emit_implicit_context {
        instr::empty(alloc)
    } else {
        // Last unnamed local slot
        let local = first_local_idx + param_count + add_reified;
        emit_memoize_helpers::get_implicit_context_memo_key(alloc, local)
    };
    let first_local = Local::Unnamed(first_local_idx);
    let key_count = (param_count + add_reified + add_implicit_context) as isize;
    Ok(InstrSeq::gather(
        alloc,
        vec![
            begin_label,
            emit_body::emit_method_prolog(e, env, pos, hhas_params, ast_params, &[])?,
            deprecation_body,
            emit_memoize_helpers::param_code_sets(alloc, hhas_params, first_local_idx),
            reified_memokeym,
            ic_memokey,
            if is_async {
                InstrSeq::gather(
                    alloc,
                    vec![
                        instr::memoget_eager(
                            alloc,
                            notfound,
                            suspended_get,
                            Some((first_local, key_count)),
                        ),
                        instr::retc(alloc),
                        instr::label(alloc, suspended_get),
                        instr::retc_suspended(alloc),
                    ],
                )
            } else {
                InstrSeq::gather(
                    alloc,
                    vec![
                        instr::memoget(alloc, notfound, Some((first_local, key_count))),
                        instr::retc(alloc),
                    ],
                )
            },
            instr::label(alloc, notfound),
            instr::nulluninit(alloc),
            instr::nulluninit(alloc),
            emit_memoize_helpers::param_code_gets(alloc, hhas_params),
            reified_get,
            instr::fcallfuncd(alloc, fcall_args, renamed_id),
            instr::memoset(alloc, Some((first_local, key_count))),
            if is_async {
                InstrSeq::gather(
                    alloc,
                    vec![
                        instr::retc_suspended(alloc),
                        instr::label(alloc, eager_set),
                        instr::memoset_eager(alloc, Some((first_local, key_count))),
                        instr::retc(alloc),
                    ],
                )
            } else {
                InstrSeq::gather(alloc, vec![instr::retc(alloc)])
            },
            default_value_setters,
        ],
    ))
}

fn make_memoize_function_no_params_code<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    deprecation_info: Option<&[TypedValue<'arena>]>,
    renamed_id: function::FunctionType<'arena>,
    is_async: bool,
) -> Result<InstrSeq<'arena>> {
    let notfound = e.label_gen_mut().next_regular();
    let suspended_get = e.label_gen_mut().next_regular();
    let eager_set = e.label_gen_mut().next_regular();
    let deprecation_body =
        emit_body::emit_deprecation_info(alloc, &env.scope, deprecation_info, e.systemlib())?;
    let fcall_args = FcallArgs::new(
        FcallFlags::default(),
        1,
        Slice::empty(),
        Slice::empty(),
        if is_async { Some(eager_set) } else { None },
        0,
        None,
    );
    Ok(InstrSeq::gather(
        alloc,
        vec![
            deprecation_body,
            if is_async {
                InstrSeq::gather(
                    alloc,
                    vec![
                        instr::memoget_eager(alloc, notfound, suspended_get, None),
                        instr::retc(alloc),
                        instr::label(alloc, suspended_get),
                        instr::retc_suspended(alloc),
                    ],
                )
            } else {
                InstrSeq::gather(
                    alloc,
                    vec![instr::memoget(alloc, notfound, None), instr::retc(alloc)],
                )
            },
            instr::label(alloc, notfound),
            instr::nulluninit(alloc),
            instr::nulluninit(alloc),
            instr::fcallfuncd(alloc, fcall_args, renamed_id),
            instr::memoset(alloc, None),
            if is_async {
                InstrSeq::gather(
                    alloc,
                    vec![
                        instr::retc_suspended(alloc),
                        instr::label(alloc, eager_set),
                        instr::memoset_eager(alloc, None),
                        instr::retc(alloc),
                    ],
                )
            } else {
                instr::retc(alloc)
            },
        ],
    ))
}

fn make_wrapper_body<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    env: Env<'a, 'arena>,
    return_type_info: HhasTypeInfo<'arena>,
    params: Vec<(HhasParam<'arena>, Option<(Label, T::Expr)>)>,
    body_instrs: InstrSeq<'arena>,
    is_reified: bool,
) -> Result<HhasBody<'arena>> {
    let mut decl_vars = vec![];
    if is_reified {
        decl_vars.push(reified::GENERICS_LOCAL_NAME.into());
    }
    emit_body::make_body(
        alloc,
        emitter,
        body_instrs,
        decl_vars,
        true,   /* is_memoize_wrapper */
        false,  /* is_memoize_wrapper_lsb */
        0,      /* num closures */
        vec![], /* upper_bounds */
        vec![], /* shadowed_tparams */
        params,
        Some(return_type_info),
        None, /* doc comment */
        Some(&env),
    )
}
