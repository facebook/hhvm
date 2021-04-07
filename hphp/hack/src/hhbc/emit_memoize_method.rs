// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_scope_rust::{self as ast_scope, Scope, ScopeItem};
use emit_attribute_rust as emit_attribute;
use emit_body_rust as emit_body;
use emit_fatal_rust as emit_fatal;
use emit_memoize_helpers_rust as emit_memoize_helpers;
use emit_param_rust as emit_param;
use emit_pos_rust::emit_pos_then;
use env::{emitter::Emitter, Env};
use hhas_attribute_rust::deprecation_info;
use hhas_body_rust::HhasBody;
use hhas_coeffects::HhasCoeffects;
use hhas_method_rust::{HhasMethod, HhasMethodFlags};
use hhas_param_rust::HhasParam;
use hhas_pos_rust::Span;
use hhas_type::Info as HhasTypeInfo;
use hhbc_ast_rust::{FcallArgs, FcallFlags, SpecialClsRef};
use hhbc_id_rust::{class, method, Id};
use hhbc_string_utils_rust::{coeffects, reified};
use instruction_sequence::{instr, InstrSeq, Result};
use naming_special_names_rust::{members, user_attributes as ua};
use options::{HhvmFlags, Options};
use oxidized::{ast as T, pos::Pos};
use runtime::TypedValue;

use std::convert::TryInto;

extern crate bitflags;
use bitflags::bitflags;

/// Precomputed information required for generation of memoized methods
pub struct MemoizeInfo<'a> {
    /// True if the enclosing class is a trait
    is_trait: bool,
    /// Enclosing class ID
    class_id: class::Type<'a>,
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
        .any(|a| ua::MEMOIZE_LSB == &a.name.1)
}

pub fn make_info<'a>(
    class: &T::Class_,
    class_id: class::Type<'a>,
    methods: &[T::Method_],
) -> Result<MemoizeInfo<'a>> {
    for m in methods.iter() {
        // check methods
        if is_memoize(m) {
            emit_memoize_helpers::check_memoize_possible(&m.name.0, &m.params[..], true)?;
            let pos = &m.name.0;
            if class.kind.is_cinterface() {
                return Err(emit_fatal::raise_fatal_runtime(
                    pos,
                    "<<__Memoize>> cannot be used in interfaces",
                ));
            } else if m.abstract_ {
                return Err(emit_fatal::raise_fatal_parse(
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

pub fn emit_wrapper_methods<'a>(
    emitter: &mut Emitter,
    env: &mut Env<'a>,
    info: &MemoizeInfo,
    class: &T::Class_,
    methods: &'a [T::Method_],
) -> Result<Vec<HhasMethod<'a>>> {
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
fn make_memoize_wrapper_method<'a>(
    emitter: &mut Emitter,
    env: &mut Env<'a>,
    info: &MemoizeInfo,
    class: &T::Class_,
    method: &'a T::Method_,
) -> Result<HhasMethod<'a>> {
    let ret = if method.name.1 == members::__CONSTRUCT {
        None
    } else {
        method.ret.1.as_ref()
    };
    let name = method::Type::from_ast_name(&method.name.1);
    let scope = &Scope {
        items: vec![
            ScopeItem::Class(ast_scope::Class::new_ref(class)),
            ScopeItem::Method(ast_scope::Method::new_ref(method)),
        ],
    };
    let mut attributes =
        emit_attribute::from_asts(emitter, &class.namespace, &method.user_attributes)?;
    attributes.extend(emit_attribute::add_reified_attribute(&method.tparams));
    let is_async = method.fun_kind.is_fasync();
    // __Memoize is not allowed on lambdas, so we never need to inherit the rx
    // level from the declaring scope when we're in a Memoize wrapper
    let coeffects = HhasCoeffects::from_ast(&method.ctxs, &method.params);
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
        params: &method.params,
        ret,
        method_id: &name,
        flags: arg_flags,
    };
    env.with_rx_body(coeffects.is_any_rx_or_pure());
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

fn emit_memoize_wrapper_body<'a>(
    emitter: &mut Emitter,
    env: &mut Env<'a>,
    args: &mut Args,
) -> Result<HhasBody> {
    let mut tparams: Vec<&str> = args
        .scope
        .get_tparams()
        .iter()
        .map(|tp| tp.name.1.as_str())
        .collect();
    let return_type_info = emit_body::emit_return_type_info(
        &tparams[..],
        args.flags.contains(Flags::IS_ASYNC),
        args.ret,
    )?;
    let hhas_params = emit_param::from_asts(
        emitter,
        &mut tparams,
        args.namespace,
        true,
        args.scope,
        args.params,
    )?;
    args.flags.set(Flags::WITH_LSB, is_memoize_lsb(args.method));
    args.flags.set(Flags::IS_STATIC, args.method.static_);
    emit(emitter, env, hhas_params, return_type_info, args)
}

fn emit<'a>(
    emitter: &mut Emitter,
    env: &mut Env<'a>,
    hhas_params: Vec<HhasParam>,
    return_type_info: HhasTypeInfo,
    args: &Args,
) -> Result<HhasBody> {
    let pos = &args.method.span;
    let instrs = make_memoize_method_code(emitter, env, pos, &hhas_params[..], args)?;
    let instrs = emit_pos_then(pos, instrs);
    make_wrapper(emitter, env, instrs, hhas_params, return_type_info, args)
}

fn make_memoize_method_code(
    emitter: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    hhas_params: &[HhasParam],
    args: &Args,
) -> Result<InstrSeq> {
    if args.params.is_empty() && !args.flags.contains(Flags::IS_REFIED) {
        make_memoize_method_no_params_code(emitter, args)
    } else {
        make_memoize_method_with_params_code(emitter, env, pos, hhas_params, args)
    }
}

// method is the already-renamed memoize method that must be wrapped
fn make_memoize_method_with_params_code(
    emitter: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    hhas_params: &[HhasParam],
    args: &Args,
) -> Result<InstrSeq> {
    let mut param_count = hhas_params.len();
    let notfound = emitter.label_gen_mut().next_regular();
    let suspended_get = emitter.label_gen_mut().next_regular();
    let eager_set = emitter.label_gen_mut().next_regular();
    // The local that contains the reified generics is the first non parameter local,
    // so the first local is parameter count + 1 when there are reified = generics
    let add_refied = usize::from(args.flags.contains(Flags::IS_REFIED));
    let first_local = local::Type::Unnamed(param_count + add_refied);
    let deprecation_body =
        emit_body::emit_deprecation_info(args.scope, args.deprecation_info, emitter.systemlib())?;
    let (begin_label, default_value_setters) =
        // Default value setters belong in the wrapper method not in the original method
        emit_param::emit_param_default_value_setter(emitter, env, pos, hhas_params)?;
    let fcall_args = {
        let mut fcall_flags = FcallFlags::default();
        if args.flags.contains(Flags::IS_REFIED) {
            fcall_flags |= FcallFlags::HAS_GENERICS;
        };
        if args.flags.contains(Flags::IS_ASYNC) {
            FcallArgs::new(
                fcall_flags,
                1,
                vec![],
                Some(eager_set.clone()),
                param_count,
                None,
            )
        } else {
            FcallArgs::new(fcall_flags, 1, vec![], None, param_count, None)
        }
    };
    let (reified_get, reified_memokeym) = if !args.flags.contains(Flags::IS_REFIED) {
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
    param_count += add_refied;
    Ok(InstrSeq::gather(vec![
        begin_label,
        emit_body::emit_method_prolog(emitter, env, pos, hhas_params, args.params, &[])?,
        deprecation_body,
        if args.method.static_ {
            instr::empty()
        } else {
            instr::checkthis()
        },
        emit_memoize_helpers::param_code_sets(hhas_params, param_count),
        reified_memokeym,
        if args.flags.contains(Flags::IS_ASYNC) {
            InstrSeq::gather(vec![
                instr::memoget_eager(
                    notfound.clone(),
                    suspended_get.clone(),
                    Some((first_local.clone(), param_count.try_into().unwrap())),
                ),
                instr::retc(),
                instr::label(suspended_get),
                instr::retc_suspended(),
            ])
        } else {
            InstrSeq::gather(vec![
                instr::memoget(
                    notfound.clone(),
                    Some((first_local.clone(), param_count.try_into().unwrap())),
                ),
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
        emit_memoize_helpers::param_code_gets(hhas_params),
        reified_get,
        if args.method.static_ {
            call_cls_method(fcall_args, args)
        } else {
            let mut renamed_method_id: method::Type<'static> =
                args.method_id.to_raw_string().to_owned().into();
            renamed_method_id.add_suffix(emit_memoize_helpers::MEMOIZE_SUFFIX);
            instr::fcallobjmethodd_nullthrows(fcall_args, renamed_method_id)
        },
        instr::memoset(Some((first_local.clone(), param_count.try_into().unwrap()))),
        if args.flags.contains(Flags::IS_ASYNC) {
            InstrSeq::gather(vec![
                instr::retc_suspended(),
                instr::label(eager_set),
                instr::memoset_eager(Some((first_local, param_count.try_into().unwrap()))),
                instr::retc(),
            ])
        } else {
            InstrSeq::gather(vec![instr::retc()])
        },
        default_value_setters,
    ]))
}

fn make_memoize_method_no_params_code(emitter: &mut Emitter, args: &Args) -> Result {
    let notfound = emitter.label_gen_mut().next_regular();
    let suspended_get = emitter.label_gen_mut().next_regular();
    let eager_set = emitter.label_gen_mut().next_regular();
    let deprecation_body =
        emit_body::emit_deprecation_info(args.scope, args.deprecation_info, emitter.systemlib())?;

    let fcall_args = FcallArgs::new(
        FcallFlags::default(),
        1,
        vec![],
        if args.flags.contains(Flags::IS_ASYNC) {
            Some(eager_set.clone())
        } else {
            None
        },
        0,
        None,
    );
    Ok(InstrSeq::gather(vec![
        deprecation_body,
        if args.method.static_ {
            instr::empty()
        } else {
            instr::checkthis()
        },
        if args.flags.contains(Flags::IS_ASYNC) {
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
        if args.method.static_ {
            instr::nulluninit()
        } else {
            instr::this()
        },
        instr::nulluninit(),
        if args.method.static_ {
            call_cls_method(fcall_args, args)
        } else {
            let mut renamed_method_id: method::Type<'static> =
                args.method_id.to_raw_string().to_owned().into();
            renamed_method_id.add_suffix(emit_memoize_helpers::MEMOIZE_SUFFIX);
            instr::fcallobjmethodd_nullthrows(fcall_args, renamed_method_id)
        },
        instr::memoset(None),
        if args.flags.contains(Flags::IS_ASYNC) {
            InstrSeq::gather(vec![
                instr::retc_suspended(),
                instr::label(eager_set),
                instr::memoset_eager(None),
                instr::retc(),
            ])
        } else {
            InstrSeq::gather(vec![instr::retc()])
        },
    ]))
}

// Construct the wrapper function
fn make_wrapper<'a>(
    emitter: &mut Emitter,
    env: &Env<'a>,
    instrs: InstrSeq,
    params: Vec<HhasParam>,
    return_type_info: HhasTypeInfo,
    args: &Args,
) -> Result<HhasBody> {
    let mut decl_vars = vec![];
    if args.flags.contains(Flags::IS_REFIED) {
        decl_vars.push(reified::GENERICS_LOCAL_NAME.into());
    }
    if args.flags.contains(Flags::HAS_COEFFECTS_LOCAL) {
        decl_vars.push(coeffects::LOCAL_NAME.into());
    }
    emit_body::make_body(
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
        Some(&env),
    )
}

fn is_method_interceptable(opts: &Options) -> bool {
    opts.hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION)
}

fn call_cls_method(fcall_args: FcallArgs, args: &Args) -> InstrSeq {
    let mut method_id: method::Type<'static> = args.method_id.to_raw_string().to_owned().into();
    method_id.add_suffix(emit_memoize_helpers::MEMOIZE_SUFFIX);
    if args.info.is_trait || args.flags.contains(Flags::WITH_LSB) {
        instr::fcallclsmethodsd(fcall_args, SpecialClsRef::Self_, method_id)
    } else {
        let class_id: class::Type<'static> = args.info.class_id.to_raw_string().to_owned().into();
        instr::fcallclsmethodd(fcall_args, method_id, class_id)
    }
}

struct Args<'a> {
    pub info: &'a MemoizeInfo<'a>,
    pub method: &'a T::Method_,
    pub namespace: &'a T::Nsenv,
    pub scope: &'a Scope<'a>,
    pub deprecation_info: Option<&'a [TypedValue]>,
    pub params: &'a [T::FunParam],
    pub ret: Option<&'a T::Hint>,
    pub method_id: &'a method::Type<'a>,
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
