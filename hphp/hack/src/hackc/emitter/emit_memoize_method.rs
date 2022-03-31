// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_scope::{self as ast_scope, Scope, ScopeItem};
use emit_method::get_attrs_for_method;
use emit_pos::emit_pos_then;
use env::{emitter::Emitter, Env};
use ffi::{Slice, Str};
use hhas_attribute::deprecation_info;
use hhas_body::HhasBody;
use hhas_coeffects::HhasCoeffects;
use hhas_method::{HhasMethod, HhasMethodFlags};
use hhas_param::HhasParam;
use hhas_pos::HhasSpan;
use hhas_type::HhasTypeInfo;
use hhbc_ast::{FCallArgs, FCallArgsFlags, LocalRange, SpecialClsRef, Visibility};
use hhbc_id::{class, method};
use hhbc_string_utils::reified;
use instruction_sequence::{instr, Error, InstrSeq, Result};
use label::Label;
use local::Local;
use naming_special_names_rust::{members, user_attributes as ua};
use options::HhvmFlags;
use oxidized::{ast as T, pos::Pos};
use runtime::TypedValue;

use bitflags::bitflags;

/// Precomputed information required for generation of memoized methods
pub struct MemoizeInfo<'arena> {
    /// True if the enclosing class is a trait
    is_trait: bool,
    /// Enclosing class ID
    class_id: class::ClassType<'arena>,
}

fn is_memoize(method: &T::Method_) -> bool {
    method
        .user_attributes
        .iter()
        .any(|a| ua::is_memoized(&a.name.1))
}

fn is_memoize_lsb(method: &T::Method_) -> bool {
    method
        .user_attributes
        .iter()
        .any(|a| ua::MEMOIZE_LSB == a.name.1 || ua::POLICY_SHARDED_MEMOIZE_LSB == a.name.1)
}

pub fn make_info<'arena>(
    class: &T::Class_,
    class_id: class::ClassType<'arena>,
    methods: &[T::Method_],
) -> Result<MemoizeInfo<'arena>> {
    for m in methods.iter() {
        // check methods
        if is_memoize(m) {
            emit_memoize_helpers::check_memoize_possible(&m.name.0, &m.params[..], true)?;
            let pos = &m.name.0;
            if class.kind.is_cinterface() {
                return Err(Error::fatal_runtime(
                    pos,
                    "<<__Memoize>> cannot be used in interfaces",
                ));
            } else if m.abstract_ {
                return Err(Error::fatal_parse(
                    pos,
                    format!(
                        "Abstract method {}::{} cannot be memoized",
                        class_id.unsafe_as_str(),
                        &m.name.1,
                    ),
                ));
            };
        };
    }

    let is_trait = class.kind.is_ctrait();
    Ok(MemoizeInfo { is_trait, class_id })
}

pub fn emit_wrapper_methods<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    info: &MemoizeInfo<'arena>,
    class: &'a T::Class_,
    methods: &'a [T::Method_],
) -> Result<Vec<HhasMethod<'arena>>> {
    // Wrapper methods may not have iterators
    emitter.iterator_mut().reset();

    let mut hhas_methods = vec![];
    for m in methods.iter() {
        if is_memoize(m) {
            env.scope
                .push_item(ScopeItem::Method(ast_scope::Method::new_ref(m)));
            hhas_methods.push(make_memoize_wrapper_method(emitter, env, info, class, m)?);
        };
    }
    Ok(hhas_methods)
}

// This is cut-and-paste from emit_method, with special casing for wrappers
fn make_memoize_wrapper_method<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    info: &MemoizeInfo<'arena>,
    class: &'a T::Class_,
    method: &'a T::Method_,
) -> Result<HhasMethod<'arena>> {
    let alloc = env.arena;
    let ret = if method.name.1 == members::__CONSTRUCT {
        None
    } else {
        method.ret.1.as_ref()
    };
    let name = method::MethodType::from_ast_name(alloc, &method.name.1);
    let scope = &Scope {
        items: vec![
            ScopeItem::Class(ast_scope::Class::new_ref(class)),
            ScopeItem::Method(ast_scope::Method::new_ref(method)),
        ],
    };
    let mut attributes = emit_attribute::from_asts(emitter, &method.user_attributes)?;
    attributes.extend(emit_attribute::add_reified_attribute(
        alloc,
        &method.tparams,
    ));
    let is_async = method.fun_kind.is_fasync();
    // __Memoize is not allowed on lambdas, so we never need to inherit the rx
    // level from the declaring scope when we're in a Memoize wrapper
    let coeffects = HhasCoeffects::from_ast(
        alloc,
        method.ctxs.as_ref(),
        &method.params,
        &method.tparams,
        &class.tparams,
    );
    let is_reified = method
        .tparams
        .iter()
        .any(|tp| tp.reified.is_reified() || tp.reified.is_soft_reified());
    let should_emit_implicit_context = emitter
        .options()
        .hhvm
        .flags
        .contains(HhvmFlags::ENABLE_IMPLICIT_CONTEXT)
        && attributes.iter().any(|a| {
            naming_special_names_rust::user_attributes::is_memoized_policy_sharded(
                a.name.unsafe_as_str(),
            )
        });
    let mut arg_flags = Flags::empty();
    arg_flags.set(Flags::IS_ASYNC, is_async);
    arg_flags.set(Flags::IS_REIFIED, is_reified);
    arg_flags.set(
        Flags::SHOULD_EMIT_IMPLICIT_CONTEXT,
        should_emit_implicit_context,
    );
    let mut args = Args {
        info,
        method,
        scope,
        deprecation_info: deprecation_info(attributes.iter()),
        params: &method.params,
        ret,
        method_id: &name,
        flags: arg_flags,
    };
    let body = emit_memoize_wrapper_body(emitter, env, &mut args)?;
    let mut flags = HhasMethodFlags::empty();
    flags.set(HhasMethodFlags::IS_ASYNC, is_async);

    let attrs = get_attrs_for_method(
        emitter,
        method,
        &attributes,
        &method.visibility,
        class,
        false,
    );

    Ok(HhasMethod {
        attributes: Slice::fill_iter(alloc, attributes.into_iter()),
        visibility: Visibility::from(method.visibility),
        name,
        body,
        span: HhasSpan::from_pos(&method.span),
        coeffects,
        flags,
        attrs,
    })
}

fn emit_memoize_wrapper_body<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    args: &mut Args<'_, 'a, 'arena>,
) -> Result<HhasBody<'arena>> {
    let alloc = env.arena;
    let mut tparams: Vec<&str> = args
        .scope
        .get_tparams()
        .iter()
        .map(|tp| tp.name.1.as_str())
        .collect();
    let return_type_info = emit_body::emit_return_type_info(
        alloc,
        &tparams[..],
        args.flags.contains(Flags::IS_ASYNC),
        args.ret,
    )?;
    let hhas_params = emit_param::from_asts(emitter, &mut tparams, true, args.scope, args.params)?;
    args.flags.set(Flags::WITH_LSB, is_memoize_lsb(args.method));
    args.flags.set(Flags::IS_STATIC, args.method.static_);
    emit(emitter, env, hhas_params, return_type_info, args)
}

fn emit<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    hhas_params: Vec<(HhasParam<'arena>, Option<(Label, T::Expr)>)>,
    return_type_info: HhasTypeInfo<'arena>,
    args: &Args<'_, 'a, 'arena>,
) -> Result<HhasBody<'arena>> {
    let pos = &args.method.span;
    let (instrs, decl_vars) = make_memoize_method_code(emitter, env, pos, &hhas_params, args)?;
    let instrs = emit_pos_then(pos, instrs);
    make_wrapper(
        emitter,
        env,
        instrs,
        hhas_params,
        decl_vars,
        return_type_info,
        args,
    )
}

fn make_memoize_method_code<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    hhas_params: &[(HhasParam<'arena>, Option<(Label, T::Expr)>)],
    args: &Args<'_, 'a, 'arena>,
) -> Result<(InstrSeq<'arena>, Vec<Str<'arena>>)> {
    if args.params.is_empty()
        && !args.flags.contains(Flags::IS_REIFIED)
        && !args.flags.contains(Flags::SHOULD_EMIT_IMPLICIT_CONTEXT)
    {
        make_memoize_method_no_params_code(emitter, args)
    } else {
        make_memoize_method_with_params_code(emitter, env, pos, hhas_params, args)
    }
}

// method is the already-renamed memoize method that must be wrapped
fn make_memoize_method_with_params_code<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    hhas_params: &[(HhasParam<'arena>, Option<(Label, T::Expr)>)],
    args: &Args<'_, 'a, 'arena>,
) -> Result<(InstrSeq<'arena>, Vec<Str<'arena>>)> {
    let alloc = env.arena;
    let param_count = hhas_params.len();
    let notfound = emitter.label_gen_mut().next_regular();
    let suspended_get = emitter.label_gen_mut().next_regular();
    let eager_set = emitter.label_gen_mut().next_regular();
    // The local that contains the reified generics is the first non parameter local,
    // so the first unnamed local is parameter count + 1 when there are reified generics.
    let is_reified = args.flags.contains(Flags::IS_REIFIED);
    let add_reified = usize::from(is_reified);
    let should_emit_implicit_context = args.flags.contains(Flags::SHOULD_EMIT_IMPLICIT_CONTEXT);
    let add_implicit_context = usize::from(should_emit_implicit_context);
    let first_unnamed_idx = param_count + add_reified;
    let generics_local = Local::new(param_count); // only used if is_reified == true.
    let decl_vars = match is_reified {
        true => vec![reified::GENERICS_LOCAL_NAME.into()],
        false => Vec::new(),
    };
    emitter.init_named_locals(
        hhas_params
            .iter()
            .map(|(param, _)| param.name)
            .chain(decl_vars.iter().copied()),
    );
    let deprecation_body = emit_body::emit_deprecation_info(
        alloc,
        args.scope,
        args.deprecation_info,
        emitter.systemlib(),
    )?;
    let (begin_label, default_value_setters) =
        // Default value setters belong in the wrapper method not in the original method
        emit_param::emit_param_default_value_setter(emitter, env, pos, hhas_params)?;
    let fcall_args = {
        let mut fcall_flags = FCallArgsFlags::default();
        if is_reified {
            fcall_flags |= FCallArgsFlags::HasGenerics;
        };
        let async_eager_target = if args.flags.contains(Flags::IS_ASYNC) {
            Some(eager_set)
        } else {
            None
        };
        FCallArgs::new(
            fcall_flags,
            1,
            param_count as u32,
            Slice::empty(),
            Slice::empty(),
            async_eager_target,
            None,
        )
    };
    let (reified_get, reified_memokeym) = if !is_reified {
        (instr::empty(), instr::empty())
    } else {
        (
            instr::cgetl(generics_local),
            InstrSeq::gather(emit_memoize_helpers::get_memo_key_list(
                Local::new(param_count + first_unnamed_idx),
                generics_local,
            )),
        )
    };
    let ic_memokey = if !should_emit_implicit_context {
        instr::empty()
    } else {
        // Last unnamed local slot
        let local = Local::new(first_unnamed_idx + param_count + add_reified);
        emit_memoize_helpers::get_implicit_context_memo_key(alloc, local)
    };
    let first_unnamed_local = Local::new(first_unnamed_idx);
    let key_count = (param_count + add_reified + add_implicit_context) as isize;
    let local_range = LocalRange {
        start: first_unnamed_local,
        len: key_count.try_into().unwrap(),
    };
    let instrs = InstrSeq::gather(vec![
        begin_label,
        emit_body::emit_method_prolog(emitter, env, pos, hhas_params, args.params, &[])?,
        deprecation_body,
        if args.method.static_ {
            instr::empty()
        } else {
            instr::checkthis()
        },
        emit_memoize_helpers::param_code_sets(hhas_params.len(), Local::new(first_unnamed_idx)),
        reified_memokeym,
        ic_memokey,
        if args.flags.contains(Flags::IS_ASYNC) {
            InstrSeq::gather(vec![
                instr::memoget_eager(notfound, suspended_get, local_range),
                instr::retc(),
                instr::label(suspended_get),
                instr::retc_suspended(),
            ])
        } else {
            InstrSeq::gather(vec![instr::memoget(notfound, local_range), instr::retc()])
        },
        instr::label(notfound),
        if args.method.static_ {
            instr::nulluninit()
        } else {
            instr::this()
        },
        instr::nulluninit(),
        emit_memoize_helpers::param_code_gets(hhas_params.len()),
        reified_get,
        if args.method.static_ {
            call_cls_method(alloc, fcall_args, args)
        } else {
            let renamed_method_id = method::MethodType::add_suffix(
                alloc,
                args.method_id,
                emit_memoize_helpers::MEMOIZE_SUFFIX,
            );
            instr::fcallobjmethodd_nullthrows(fcall_args, renamed_method_id)
        },
        instr::memoset(local_range),
        if args.flags.contains(Flags::IS_ASYNC) {
            InstrSeq::gather(vec![
                instr::retc_suspended(),
                instr::label(eager_set),
                instr::memoset_eager(local_range),
                instr::retc(),
            ])
        } else {
            instr::retc()
        },
        default_value_setters,
    ]);
    Ok((instrs, decl_vars))
}

fn make_memoize_method_no_params_code<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    args: &Args<'_, 'a, 'arena>,
) -> Result<(InstrSeq<'arena>, Vec<Str<'arena>>)> {
    let notfound = emitter.label_gen_mut().next_regular();
    let suspended_get = emitter.label_gen_mut().next_regular();
    let eager_set = emitter.label_gen_mut().next_regular();
    let alloc = emitter.alloc;
    let deprecation_body = emit_body::emit_deprecation_info(
        alloc,
        args.scope,
        args.deprecation_info,
        emitter.systemlib(),
    )?;

    let fcall_args = FCallArgs::new(
        FCallArgsFlags::default(),
        1,
        0,
        Slice::empty(),
        Slice::empty(),
        if args.flags.contains(Flags::IS_ASYNC) {
            Some(eager_set)
        } else {
            None
        },
        None,
    );
    let instrs = InstrSeq::gather(vec![
        deprecation_body,
        if args.method.static_ {
            instr::empty()
        } else {
            instr::checkthis()
        },
        if args.flags.contains(Flags::IS_ASYNC) {
            InstrSeq::gather(vec![
                instr::memoget_eager(notfound, suspended_get, LocalRange::default()),
                instr::retc(),
                instr::label(suspended_get),
                instr::retc_suspended(),
            ])
        } else {
            InstrSeq::gather(vec![
                instr::memoget(notfound, LocalRange::default()),
                instr::retc(),
            ])
        },
        instr::label(notfound),
        if args.method.static_ {
            instr::nulluninit()
        } else {
            instr::this()
        },
        instr::nulluninit(),
        if args.method.static_ {
            call_cls_method(alloc, fcall_args, args)
        } else {
            let renamed_method_id = method::MethodType::add_suffix(
                alloc,
                args.method_id,
                emit_memoize_helpers::MEMOIZE_SUFFIX,
            );
            instr::fcallobjmethodd_nullthrows(fcall_args, renamed_method_id)
        },
        instr::memoset(LocalRange::default()),
        if args.flags.contains(Flags::IS_ASYNC) {
            InstrSeq::gather(vec![
                instr::retc_suspended(),
                instr::label(eager_set),
                instr::memoset_eager(LocalRange::default()),
                instr::retc(),
            ])
        } else {
            instr::retc()
        },
    ]);
    Ok((instrs, Vec::new()))
}

// Construct the wrapper function
fn make_wrapper<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a, 'arena>,
    instrs: InstrSeq<'arena>,
    params: Vec<(HhasParam<'arena>, Option<(Label, T::Expr)>)>,
    decl_vars: Vec<Str<'arena>>,
    return_type_info: HhasTypeInfo<'arena>,
    args: &Args<'_, 'a, 'arena>,
) -> Result<HhasBody<'arena>> {
    emit_body::make_body(
        env.arena,
        emitter,
        instrs,
        decl_vars,
        true,
        args.flags.contains(Flags::WITH_LSB),
        vec![], /* upper_bounds */
        vec![], /* shadowed_tparams */
        params,
        Some(return_type_info),
        None,
        Some(env),
    )
}

fn call_cls_method<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    fcall_args: FCallArgs<'arena>,
    args: &Args<'_, 'a, 'arena>,
) -> InstrSeq<'arena> {
    let method_id =
        method::MethodType::add_suffix(alloc, args.method_id, emit_memoize_helpers::MEMOIZE_SUFFIX);
    if args.info.is_trait || args.flags.contains(Flags::WITH_LSB) {
        instr::fcallclsmethodsd(fcall_args, SpecialClsRef::SelfCls, method_id)
    } else {
        instr::fcallclsmethodd(fcall_args, method_id, args.info.class_id)
    }
}

struct Args<'r, 'ast, 'arena> {
    pub info: &'r MemoizeInfo<'arena>,
    pub method: &'r T::Method_,
    pub scope: &'r Scope<'ast, 'arena>,
    pub deprecation_info: Option<&'r [TypedValue<'arena>]>,
    pub params: &'r [T::FunParam],
    pub ret: Option<&'r T::Hint>,
    pub method_id: &'r method::MethodType<'arena>,
    pub flags: Flags,
}

bitflags! {
    pub struct Flags: u8 {
        const IS_STATIC = 1 << 1;
        const IS_REIFIED = 1 << 2;
        const WITH_LSB = 1 << 3;
        const IS_ASYNC = 1 << 4;
        const SHOULD_EMIT_IMPLICIT_CONTEXT = 1 << 5;
    }
}
