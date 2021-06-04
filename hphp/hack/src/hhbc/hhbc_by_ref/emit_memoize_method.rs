// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc_by_ref_ast_scope::{self as ast_scope, Scope, ScopeItem};
use hhbc_by_ref_emit_attribute as emit_attribute;
use hhbc_by_ref_emit_body as emit_body;
use hhbc_by_ref_emit_fatal::{raise_fatal_parse, raise_fatal_runtime};
use hhbc_by_ref_emit_memoize_helpers as emit_memoize_helpers;
use hhbc_by_ref_emit_param as emit_param;
use hhbc_by_ref_emit_pos::emit_pos_then;
use hhbc_by_ref_env::{emitter::Emitter, Env};
use hhbc_by_ref_hhas_attribute::deprecation_info;
use hhbc_by_ref_hhas_body::HhasBody;
use hhbc_by_ref_hhas_coeffects::HhasCoeffects;
use hhbc_by_ref_hhas_method::{HhasMethod, HhasMethodFlags};
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_type::Info as HhasTypeInfo;
use hhbc_by_ref_hhbc_ast::{FcallArgs, FcallFlags, SpecialClsRef};
use hhbc_by_ref_hhbc_id::{class, method, Id};
use hhbc_by_ref_hhbc_string_utils::{coeffects, reified};
use hhbc_by_ref_instruction_sequence::{instr, InstrSeq, Result};
use hhbc_by_ref_options::{HhvmFlags, Options};
use hhbc_by_ref_runtime::TypedValue;
use naming_special_names_rust::{members, user_attributes as ua};
use oxidized::{ast as T, pos::Pos};

use std::convert::TryInto;

use bitflags::bitflags;

/// Precomputed information required for generation of memoized methods
pub struct MemoizeInfo<'arena> {
    /// True if the enclosing class is a trait
    is_trait: bool,
    /// Enclosing class ID
    class_id: class::Type<'arena>,
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
        .any(|a| ua::MEMOIZE_LSB == a.name.1)
}

pub fn make_info<'arena>(
    class: &T::Class_,
    class_id: class::Type<'arena>,
    methods: &[T::Method_],
) -> Result<MemoizeInfo<'arena>> {
    for m in methods.iter() {
        // check methods
        if is_memoize(m) {
            emit_memoize_helpers::check_memoize_possible(&m.name.0, &m.params[..], true)?;
            let pos = &m.name.0;
            if class.kind.is_cinterface() {
                return Err(raise_fatal_runtime(
                    pos,
                    "<<__Memoize>> cannot be used in interfaces",
                ));
            } else if m.abstract_ {
                return Err(raise_fatal_parse(
                    pos,
                    format!(
                        "Abstract method {}::{} cannot be memoized",
                        class_id.to_raw_string(),
                        &m.name.1,
                    ),
                ));
            };
        };
    }

    let is_trait = class.kind.is_ctrait();
    Ok(MemoizeInfo { is_trait, class_id })
}

pub fn emit_wrapper_methods<'a, 'arena>(
    emitter: &mut Emitter<'arena>,
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
fn make_memoize_wrapper_method<'a, 'arena>(
    emitter: &mut Emitter<'arena>,
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
    let name = method::Type::from_ast_name(alloc, &method.name.1);
    let scope = &Scope {
        items: vec![
            ScopeItem::Class(ast_scope::Class::new_ref(class)),
            ScopeItem::Method(ast_scope::Method::new_ref(method)),
        ],
    };
    let mut attributes = emit_attribute::from_asts(alloc, emitter, &method.user_attributes)?;
    attributes.extend(emit_attribute::add_reified_attribute(&method.tparams));
    let is_async = method.fun_kind.is_fasync();
    // __Memoize is not allowed on lambdas, so we never need to inherit the rx
    // level from the declaring scope when we're in a Memoize wrapper
    let coeffects = HhasCoeffects::from_ast(
        &method.ctxs,
        &method.params,
        &method.tparams,
        &class.tparams,
    );
    let is_reified = method
        .tparams
        .iter()
        .any(|tp| tp.reified.is_reified() || tp.reified.is_soft_reified());
    let is_interceptable = is_method_interceptable(emitter.options());
    let mut arg_flags = Flags::empty();
    arg_flags.set(Flags::IS_ASYNC, is_async);
    arg_flags.set(Flags::IS_REFIED, is_reified);
    arg_flags.set(Flags::HAS_COEFFECTS_LOCAL, coeffects.has_coeffects_local());
    let mut args = Args {
        info,
        method,
        namespace: &class.namespace,
        scope,
        deprecation_info: deprecation_info(attributes.iter()),
        //deprecation_info: None,
        params: &method.params,
        ret,
        method_id: &name,
        flags: arg_flags,
    };
    let body = emit_memoize_wrapper_body(emitter, env, &mut args)?;
    let mut flags = HhasMethodFlags::empty();
    flags.set(HhasMethodFlags::IS_STATIC, method.static_);
    flags.set(HhasMethodFlags::IS_ABSTRACT, method.abstract_);
    flags.set(HhasMethodFlags::IS_FINAL, method.final_);
    flags.set(HhasMethodFlags::IS_ASYNC, is_async);
    flags.set(HhasMethodFlags::IS_INTERCEPTABLE, is_interceptable);
    Ok(HhasMethod {
        attributes,
        visibility: method.visibility,
        name,
        body,
        span: Span::from_pos(&method.span),
        coeffects,
        flags,
    })
}

fn emit_memoize_wrapper_body<'a, 'arena>(
    emitter: &mut Emitter<'arena>,
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
    let hhas_params =
        emit_param::from_asts(alloc, emitter, &mut tparams, true, args.scope, args.params)?;
    args.flags.set(Flags::WITH_LSB, is_memoize_lsb(args.method));
    args.flags.set(Flags::IS_STATIC, args.method.static_);
    emit(emitter, env, hhas_params, return_type_info, args)
}

fn emit<'a, 'arena>(
    emitter: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    hhas_params: Vec<HhasParam<'arena>>,
    return_type_info: HhasTypeInfo,
    args: &Args<'_, 'a, 'arena>,
) -> Result<HhasBody<'arena>> {
    let alloc = env.arena;
    let pos = &args.method.span;
    let instrs = make_memoize_method_code(emitter, env, pos, &hhas_params[..], args)?;
    let instrs = emit_pos_then(alloc, pos, instrs);
    make_wrapper(emitter, env, instrs, hhas_params, return_type_info, args)
}

fn make_memoize_method_code<'a, 'arena>(
    emitter: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    hhas_params: &[HhasParam<'arena>],
    args: &Args<'_, 'a, 'arena>,
) -> Result<InstrSeq<'arena>> {
    if args.params.is_empty() && !args.flags.contains(Flags::IS_REFIED) {
        make_memoize_method_no_params_code(env.arena, emitter, args)
    } else {
        make_memoize_method_with_params_code(emitter, env, pos, hhas_params, args)
    }
}

// method is the already-renamed memoize method that must be wrapped
fn make_memoize_method_with_params_code<'a, 'arena>(
    emitter: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    hhas_params: &[HhasParam<'arena>],
    args: &Args<'_, 'a, 'arena>,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let mut param_count = hhas_params.len();
    let notfound = emitter.label_gen_mut().next_regular();
    let suspended_get = emitter.label_gen_mut().next_regular();
    let eager_set = emitter.label_gen_mut().next_regular();
    // The local that contains the reified generics is the first non parameter local,
    // so the first local is parameter count + 1 when there are reified = generics
    let add_refied = usize::from(args.flags.contains(Flags::IS_REFIED));
    let first_local = hhbc_by_ref_local::Type::Unnamed(param_count + add_refied);
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
        let mut fcall_flags = FcallFlags::default();
        if args.flags.contains(Flags::IS_REFIED) {
            fcall_flags |= FcallFlags::HAS_GENERICS;
        };
        if args.flags.contains(Flags::IS_ASYNC) {
            FcallArgs::new(fcall_flags, 1, &[], Some(eager_set), param_count, None)
        } else {
            FcallArgs::new(fcall_flags, 1, &[], None, param_count, None)
        }
    };
    let (reified_get, reified_memokeym) = if !args.flags.contains(Flags::IS_REFIED) {
        (instr::empty(alloc), instr::empty(alloc))
    } else {
        (
            instr::cgetl(
                alloc,
                hhbc_by_ref_local::Type::Named(reified::GENERICS_LOCAL_NAME),
            ),
            InstrSeq::gather(
                alloc,
                emit_memoize_helpers::get_memo_key_list(
                    alloc,
                    param_count,
                    param_count + add_refied,
                    reified::GENERICS_LOCAL_NAME,
                ),
            ),
        )
    };
    param_count += add_refied;
    Ok(InstrSeq::gather(
        alloc,
        vec![
            begin_label,
            emit_body::emit_method_prolog(emitter, env, pos, hhas_params, args.params, &[])?,
            deprecation_body,
            if args.method.static_ {
                instr::empty(alloc)
            } else {
                instr::checkthis(alloc)
            },
            emit_memoize_helpers::param_code_sets(alloc, hhas_params, param_count),
            reified_memokeym,
            if args.flags.contains(Flags::IS_ASYNC) {
                InstrSeq::gather(
                    alloc,
                    vec![
                        instr::memoget_eager(
                            alloc,
                            notfound,
                            suspended_get,
                            Some((first_local, param_count.try_into().unwrap())),
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
                        instr::memoget(
                            alloc,
                            notfound,
                            Some((first_local, param_count.try_into().unwrap())),
                        ),
                        instr::retc(alloc),
                    ],
                )
            },
            instr::label(alloc, notfound),
            if args.method.static_ {
                instr::nulluninit(alloc)
            } else {
                instr::this(alloc)
            },
            instr::nulluninit(alloc),
            emit_memoize_helpers::param_code_gets(alloc, hhas_params),
            reified_get,
            if args.method.static_ {
                call_cls_method(alloc, fcall_args, args)
            } else {
                let renamed_method_id = method::Type::add_suffix(
                    alloc,
                    args.method_id,
                    emit_memoize_helpers::MEMOIZE_SUFFIX,
                );
                instr::fcallobjmethodd_nullthrows(alloc, fcall_args, renamed_method_id)
            },
            instr::memoset(alloc, Some((first_local, param_count.try_into().unwrap()))),
            if args.flags.contains(Flags::IS_ASYNC) {
                InstrSeq::gather(
                    alloc,
                    vec![
                        instr::retc_suspended(alloc),
                        instr::label(alloc, eager_set),
                        instr::memoset_eager(
                            alloc,
                            Some((first_local, param_count.try_into().unwrap())),
                        ),
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

fn make_memoize_method_no_params_code<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    args: &Args<'_, 'a, 'arena>,
) -> Result<InstrSeq<'arena>> {
    let notfound = emitter.label_gen_mut().next_regular();
    let suspended_get = emitter.label_gen_mut().next_regular();
    let eager_set = emitter.label_gen_mut().next_regular();
    let deprecation_body = emit_body::emit_deprecation_info(
        alloc,
        args.scope,
        args.deprecation_info,
        emitter.systemlib(),
    )?;

    let fcall_args = FcallArgs::new(
        FcallFlags::default(),
        1,
        &[],
        if args.flags.contains(Flags::IS_ASYNC) {
            Some(eager_set)
        } else {
            None
        },
        0,
        None,
    );
    Ok(InstrSeq::gather(
        alloc,
        vec![
            deprecation_body,
            if args.method.static_ {
                instr::empty(alloc)
            } else {
                instr::checkthis(alloc)
            },
            if args.flags.contains(Flags::IS_ASYNC) {
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
            if args.method.static_ {
                instr::nulluninit(alloc)
            } else {
                instr::this(alloc)
            },
            instr::nulluninit(alloc),
            if args.method.static_ {
                call_cls_method(alloc, fcall_args, args)
            } else {
                let renamed_method_id = method::Type::add_suffix(
                    alloc,
                    args.method_id,
                    emit_memoize_helpers::MEMOIZE_SUFFIX,
                );
                instr::fcallobjmethodd_nullthrows(alloc, fcall_args, renamed_method_id)
            },
            instr::memoset(alloc, None),
            if args.flags.contains(Flags::IS_ASYNC) {
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
                InstrSeq::gather(alloc, vec![instr::retc(alloc)])
            },
        ],
    ))
}

// Construct the wrapper function
fn make_wrapper<'a, 'arena>(
    emitter: &mut Emitter<'arena>,
    env: &Env<'a, 'arena>,
    instrs: InstrSeq<'arena>,
    params: Vec<HhasParam<'arena>>,
    return_type_info: HhasTypeInfo,
    args: &Args<'_, 'a, 'arena>,
) -> Result<HhasBody<'arena>> {
    let alloc = env.arena;
    let mut decl_vars = vec![];
    if args.flags.contains(Flags::IS_REFIED) {
        decl_vars.push(reified::GENERICS_LOCAL_NAME.into());
    }
    if args.flags.contains(Flags::HAS_COEFFECTS_LOCAL) {
        decl_vars.push(coeffects::LOCAL_NAME.into());
    }
    emit_body::make_body(
        alloc,
        emitter,
        instrs,
        decl_vars,
        true,
        args.flags.contains(Flags::WITH_LSB),
        0,
        vec![], /* upper_bounds */
        vec![], /* shadowed_tparams */
        params,
        Some(return_type_info),
        None,
        Some(env),
    )
}

fn is_method_interceptable(opts: &Options) -> bool {
    opts.hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION)
}

fn call_cls_method<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    fcall_args: FcallArgs<'arena>,
    args: &Args<'_, 'a, 'arena>,
) -> InstrSeq<'arena> {
    let method_id =
        method::Type::add_suffix(alloc, args.method_id, emit_memoize_helpers::MEMOIZE_SUFFIX);
    if args.info.is_trait || args.flags.contains(Flags::WITH_LSB) {
        instr::fcallclsmethodsd(alloc, fcall_args, SpecialClsRef::Self_, method_id)
    } else {
        instr::fcallclsmethodd(alloc, fcall_args, method_id, args.info.class_id)
    }
}

struct Args<'r, 'ast, 'arena> {
    pub info: &'r MemoizeInfo<'arena>,
    pub method: &'r T::Method_,
    pub namespace: &'r T::Nsenv,
    pub scope: &'r Scope<'ast>,
    pub deprecation_info: Option<&'r [TypedValue<'arena>]>,
    pub params: &'r [T::FunParam],
    pub ret: Option<&'r T::Hint>,
    pub method_id: &'r method::Type<'arena>,
    pub flags: Flags,
}

bitflags! {
    pub struct Flags: u8 {
        const HAS_COEFFECTS_LOCAL = 1 << 0;
        const IS_STATIC = 1 << 1;
        const IS_REFIED = 1 << 2;
        const WITH_LSB = 1 << 3;
        const IS_ASYNC = 1 << 4;
    }
}
