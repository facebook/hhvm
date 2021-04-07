// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_scope_rust::{self as ast_scope, Scope, ScopeItem};
use emit_attribute_rust as emit_attribute;
use emit_body_rust as emit_body;
use emit_memoize_helpers_rust as emit_memoize_helpers;
use emit_param_rust as emit_param;
use emit_pos_rust::emit_pos_then;
use env::{emitter::Emitter, Env};
use hhas_body_rust::HhasBody;
use hhas_coeffects::HhasCoeffects;
use hhas_function_rust::{Flags as HhasFunctionFlags, HhasFunction};
use hhas_param_rust::HhasParam;
use hhas_pos_rust::Span;
use hhas_type::Info as HhasTypeInfo;
use hhbc_ast_rust::{FcallArgs, FcallFlags};
use hhbc_id_rust::{function::Type as FunId, Id};
use hhbc_string_utils_rust::{coeffects, reified};
use instruction_sequence::{instr, InstrSeq, Result};
use ocamlrep::rc::RcOc;
use options::{HhvmFlags, Options, RepoFlags};
use oxidized::{ast as T, pos::Pos};
use runtime::TypedValue;

pub(crate) fn is_interceptable(opts: &Options) -> bool {
    opts.hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION)
        && !opts.repo_flags.contains(RepoFlags::AUTHORITATIVE)
}

pub(crate) fn emit_wrapper_function<'a>(
    emitter: &mut Emitter,
    original_id: FunId<'a>,
    renamed_id: &FunId,
    deprecation_info: &Option<&[TypedValue]>,
    f: &'a T::Fun_,
) -> Result<HhasFunction<'a>> {
    emit_memoize_helpers::check_memoize_possible(&(f.name).0, &f.params, false)?;
    let scope = Scope {
        items: vec![ScopeItem::Function(ast_scope::Fun::new_ref(f))],
    };
    let mut tparams = scope
        .get_tparams()
        .iter()
        .map(|tp| tp.name.1.as_str())
        .collect::<Vec<_>>();
    let params =
        emit_param::from_asts(emitter, &mut tparams, &f.namespace, true, &scope, &f.params)?;
    let mut attributes = emit_attribute::from_asts(emitter, &f.namespace, &f.user_attributes)?;
    attributes.extend(emit_attribute::add_reified_attribute(&f.tparams));
    let return_type_info = emit_body::emit_return_type_info(
        &tparams,
        f.fun_kind.is_fasync(), /* skip_awaitable */
        f.ret.1.as_ref(),
    )?;
    let is_reified = f
        .tparams
        .iter()
        .any(|tp| tp.reified.is_reified() || tp.reified.is_soft_reified());
    let mut env = Env::default(RcOc::clone(&f.namespace)).with_scope(scope);
    // TODO(hrust): avoid cloning
    let renamed_id_owned: FunId<'static> = renamed_id.to_raw_string().to_string().into();
    let body_instrs = make_memoize_function_code(
        emitter,
        &mut env,
        &f.span,
        *deprecation_info,
        &params,
        &f.params,
        renamed_id_owned,
        f.fun_kind.is_fasync(),
        is_reified,
    )?;
    let coeffects = HhasCoeffects::from_ast(&f.ctxs, &f.params);
    let has_coeffects_local = coeffects.has_coeffects_local();
    env.with_rx_body(coeffects.is_any_rx_or_pure());
    let body = make_wrapper_body(
        emitter,
        env,
        return_type_info,
        params,
        body_instrs,
        is_reified,
        has_coeffects_local,
    )?;

    let mut flags = HhasFunctionFlags::empty();
    flags.set(HhasFunctionFlags::ASYNC, f.fun_kind.is_fasync());
    let is_interceptable = is_interceptable(emitter.options());
    flags.set(HhasFunctionFlags::INTERCEPTABLE, is_interceptable);

    Ok(HhasFunction {
        attributes,
        name: original_id,
        body,
        span: Span::from_pos(&f.span),
        coeffects,
        flags,
    })
}

fn make_memoize_function_code(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    deprecation_info: Option<&[TypedValue]>,
    hhas_params: &[HhasParam],
    ast_params: &[T::FunParam],
    renamed_id: FunId<'static>,
    is_async: bool,
    is_reified: bool,
) -> Result {
    let fun = if hhas_params.is_empty() && !is_reified {
        make_memoize_function_no_params_code(e, env, deprecation_info, renamed_id, is_async)
    } else {
        make_memoize_function_with_params_code(
            e,
            env,
            pos,
            deprecation_info,
            hhas_params,
            ast_params,
            renamed_id,
            is_async,
            is_reified,
        )
    }?;
    Ok(emit_pos_then(pos, fun))
}

fn make_memoize_function_with_params_code(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    deprecation_info: Option<&[TypedValue]>,
    hhas_params: &[HhasParam],
    ast_params: &[T::FunParam],
    renamed_id: FunId<'static>,
    is_async: bool,
    is_reified: bool,
) -> Result {
    let param_count = hhas_params.len();
    let notfound = e.label_gen_mut().next_regular();
    let suspended_get = e.label_gen_mut().next_regular();
    let eager_set = e.label_gen_mut().next_regular();
    // The local that contains the reified generics is the first non parameter local,
    // so the first local is parameter count + 1 when there are reified = generics
    let add_refied = usize::from(is_reified);
    let first_local = local::Type::Unnamed(param_count + add_refied);
    let deprecation_body =
        emit_body::emit_deprecation_info(&env.scope, deprecation_info, e.systemlib())?;
    let (begin_label, default_value_setters) =
        // Default value setters belong in the wrapper method not in the original method
        emit_param::emit_param_default_value_setter(e, env, pos, hhas_params)?;
    let fcall_args = {
        let mut fcall_flags = FcallFlags::default();
        fcall_flags.set(FcallFlags::HAS_GENERICS, is_reified);
        FcallArgs::new(
            fcall_flags,
            1,
            vec![],
            if is_async {
                Some(eager_set.clone())
            } else {
                None
            },
            param_count,
            None,
        )
    };
    let (reified_get, reified_memokeym) = if !is_reified {
        (instr::empty(), instr::empty())
    } else {
        (
            instr::cgetl(local::Type::Named(reified::GENERICS_LOCAL_NAME.into())),
            InstrSeq::gather(emit_memoize_helpers::get_memo_key_list(
                param_count,
                param_count + add_refied,
                reified::GENERICS_LOCAL_NAME.into(),
            )),
        )
    };
    let param_count = (param_count + add_refied) as isize;
    Ok(InstrSeq::gather(vec![
        begin_label,
        emit_body::emit_method_prolog(e, env, pos, hhas_params, ast_params, &[])?,
        deprecation_body,
        emit_memoize_helpers::param_code_sets(hhas_params, param_count as usize),
        reified_memokeym,
        if is_async {
            InstrSeq::gather(vec![
                instr::memoget_eager(
                    notfound.clone(),
                    suspended_get.clone(),
                    Some((first_local.clone(), param_count)),
                ),
                instr::retc(),
                instr::label(suspended_get),
                instr::retc_suspended(),
            ])
        } else {
            InstrSeq::gather(vec![
                instr::memoget(notfound.clone(), Some((first_local.clone(), param_count))),
                instr::retc(),
            ])
        },
        instr::label(notfound),
        instr::nulluninit(),
        instr::nulluninit(),
        emit_memoize_helpers::param_code_gets(hhas_params),
        reified_get,
        instr::fcallfuncd(fcall_args, renamed_id),
        instr::memoset(Some((first_local.clone(), param_count))),
        if is_async {
            InstrSeq::gather(vec![
                instr::retc_suspended(),
                instr::label(eager_set),
                instr::memoset_eager(Some((first_local, param_count))),
                instr::retc(),
            ])
        } else {
            InstrSeq::gather(vec![instr::retc()])
        },
        default_value_setters,
    ]))
}

fn make_memoize_function_no_params_code(
    e: &mut Emitter,
    env: &mut Env,
    deprecation_info: Option<&[TypedValue]>,
    renamed_id: FunId<'static>,
    is_async: bool,
) -> Result {
    let notfound = e.label_gen_mut().next_regular();
    let suspended_get = e.label_gen_mut().next_regular();
    let eager_set = e.label_gen_mut().next_regular();
    let deprecation_body =
        emit_body::emit_deprecation_info(&env.scope, deprecation_info, e.systemlib())?;
    let fcall_args = FcallArgs::new(
        FcallFlags::default(),
        1,
        vec![],
        if is_async {
            Some(eager_set.clone())
        } else {
            None
        },
        0,
        None,
    );
    Ok(InstrSeq::gather(vec![
        deprecation_body,
        if is_async {
            InstrSeq::gather(vec![
                instr::memoget_eager(notfound.clone(), suspended_get.clone(), None),
                instr::retc(),
                instr::label(suspended_get),
                instr::retc_suspended(),
            ])
        } else {
            InstrSeq::gather(vec![instr::memoget(notfound.clone(), None), instr::retc()])
        },
        instr::label(notfound),
        instr::nulluninit(),
        instr::nulluninit(),
        instr::fcallfuncd(fcall_args, renamed_id),
        instr::memoset(None),
        if is_async {
            InstrSeq::gather(vec![
                instr::retc_suspended(),
                instr::label(eager_set),
                instr::memoset_eager(None),
                instr::retc(),
            ])
        } else {
            instr::retc()
        },
    ]))
}

fn make_wrapper_body<'a>(
    emitter: &mut Emitter,
    env: Env<'a>,
    return_type_info: HhasTypeInfo,
    params: Vec<HhasParam>,
    body_instrs: InstrSeq,
    is_reified: bool,
    has_coeffects_local: bool,
) -> Result<HhasBody> {
    let mut decl_vars = vec![];
    if is_reified {
        decl_vars.push(reified::GENERICS_LOCAL_NAME.into());
    }
    if has_coeffects_local {
        decl_vars.push(coeffects::LOCAL_NAME.into());
    }
    emit_body::make_body(
        emitter,
        body_instrs,
        decl_vars,
        true,   /* is_memoize_wrapper */
        false,  /* is_memoize_wrapper_lsb */
        vec![], /* upper_bounds */
        vec![], /* shadowed_tparams */
        params,
        Some(return_type_info),
        None, /* doc comment */
        Some(&env),
    )
}
