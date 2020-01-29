// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(slice_patterns)]

mod emit_statement;
mod reified_generics_helpers;
mod try_finally_rewriter;

use ast_scope_rust::{Scope, ScopeItem};
use decl_vars_rust as decl_vars;
use emit_adata_rust as emit_adata;
use emit_param_rust as emit_param;
use emit_type_hint_rust as emit_type_hint;
use env::{emitter::Emitter, Env};
use generator_rust as generator;
use hhas_body_rust::HhasBody;
use hhas_param_rust::HhasParam;
use hhas_type::Info as HhasTypeInfo;
use hhbc_ast_rust::Instruct;
use hhbc_string_utils_rust as string_utils;
use instruction_sequence_rust::{InstrSeq, Result};
use label_rewriter_rust as label_rewriter;
use naming_special_names_rust::classes;
use options::CompilerFlags;
use oxidized::{aast, ast as tast, ast_defs, namespace_env, pos::Pos};
use runtime::TypedValue;

static THIS: &'static str = "$this";

extern crate bitflags;

use bitflags::bitflags;

/// Optional arguments for emit_body; use Args::default() for defaults
pub struct Args<'a> {
    pub immediate_tparams: &'a Vec<tast::Tparam>,
    pub ast_params: &'a Vec<tast::FunParam>,
    pub ret: Option<aast::Hint>,
    pub scope: &'a Scope<'a>,
    pub pos: &'a Pos,
    pub deprecation_info: &'a Option<&'a [TypedValue]>,
    pub doc_comment: Option<&'a str>,
    pub default_dropthrough: Option<InstrSeq>,
    pub flags: Flags,
}
impl Args<'_> {
    pub fn with_default<F, T>(f: F) -> T
    where
        F: FnOnce(Args) -> T,
    {
        let args = Args {
            immediate_tparams: &vec![],
            ast_params: &vec![],
            ret: None,
            scope: &Scope::toplevel(),
            pos: &Pos::make_none(),
            deprecation_info: &None,
            doc_comment: None,
            default_dropthrough: None,
            flags: Flags::empty(),
        };
        f(args)
    }
}

bitflags! {
    pub struct Flags: u8 {
        const SKIP_AWAITABLE = 1 << 1;
        const MEMOIZE = 1 << 2;
        const CLOSURE_BODY = 1 << 3;
        const NATIVE = 1 << 4;
        const RX_BODY = 1 << 5;
        const ASYNC = 1 << 6;
        const DEBUGGER_MODIFY_PROGRAM = 1 << 7;
    }
}

pub fn emit_body_with_default_args(
    emitter: &mut Emitter,
    namespace: &namespace_env::Env,
    body: &tast::Program,
    return_value: InstrSeq,
) -> Result<HhasBody> {
    let (res, _, _) =
        Args::with_default(|mut args| emit_body(emitter, namespace, body, return_value, &mut args));
    res
}

pub fn emit_body(
    emitter: &mut Emitter,
    namespace: &namespace_env::Env,
    body: &tast::Program,
    return_value: InstrSeq,
    args: &mut Args,
) -> (Result<HhasBody>, bool, bool) {
    if args.flags.contains(Flags::ASYNC)
        && args.flags.contains(Flags::SKIP_AWAITABLE)
        && args.ret.as_ref().map_or(false, |hint| !is_awaitable(&hint))
    {
        report_error(
            args.flags.contains(Flags::CLOSURE_BODY),
            args.scope,
            args.pos,
        );
    };

    let tparams = args
        .scope
        .get_tparams()
        .into_iter()
        .map(|tp| tp.clone())
        .collect::<Vec<tast::Tparam>>();
    let mut tp_names = get_tp_names(&tparams);
    let (is_generator, is_pair_generator) = generator::is_function_generator(body);

    emitter.label_gen_mut().reset();
    emitter.iterator_mut().reset();

    let return_type_info = match make_return_type_info(
        args.flags.contains(Flags::SKIP_AWAITABLE),
        args.flags.contains(Flags::NATIVE),
        &args.ret,
        &tp_names,
    ) {
        Err(x) => return (Err(x), is_generator, is_pair_generator),
        Ok(x) => x,
    };
    let params = match make_params(
        emitter,
        namespace,
        &mut tp_names,
        args,
        !args.flags.contains(Flags::MEMOIZE),
    ) {
        Err(x) => return (Err(x), is_generator, is_pair_generator),
        Ok(x) => x,
    };
    let upper_bounds = make_upper_bounds(
        emitter,
        args.immediate_tparams,
        args.flags.contains(Flags::SKIP_AWAITABLE),
    );
    let (need_local_this, decl_vars) = make_decl_vars(
        emitter,
        args.scope,
        args.immediate_tparams,
        &params,
        body,
        args.flags.contains(Flags::CLOSURE_BODY),
    );
    let mut env = make_env(
        namespace,
        need_local_this,
        args.scope,
        args.flags.contains(Flags::RX_BODY),
    );

    emit_statements(
        &mut env,
        args.ret.as_ref(),
        return_value,
        &params,
        &mut args.default_dropthrough,
        &return_type_info,
        args.flags.contains(Flags::ASYNC),
        is_generator,
    );
    env.jump_targets_gen.reset();

    let should_reserve_locals = set_function_jmp_targets(emitter, &mut env, args.scope);
    let local_gen = emitter.local_gen_mut();
    local_gen.reset(params.len() + decl_vars.len());
    if should_reserve_locals {
        local_gen.reserve_retval_and_label_id_locals();
    };

    let body_instrs = match make_body_instrs(
        emitter,
        &mut env,
        args,
        &params,
        &tparams,
        &decl_vars,
        body,
        need_local_this,
        is_generator,
        args.flags.contains(Flags::NATIVE),
        args.flags.contains(Flags::DEBUGGER_MODIFY_PROGRAM),
    ) {
        Err(x) => return (Err(x), is_generator, is_pair_generator),
        Ok(x) => x,
    };

    (
        Ok(make_body(
            emitter,
            body_instrs,
            decl_vars,
            false, // is_memoize_wrapper
            false, // is_memoize_wrapper_lsb
            upper_bounds,
            params,
            Some(return_type_info),
            args.doc_comment.map(|dc| dc.to_string()),
            Some(env),
        )),
        is_generator,
        is_pair_generator,
    )
}

fn make_body_instrs(
    emitter: &mut Emitter,
    env: &mut Env,
    args: &Args,
    params: &[HhasParam],
    tparams: &[tast::Tparam],
    decl_vars: &[String],
    body: &tast::Program,
    need_local_this: bool,
    is_native: bool,
    is_generator: bool,
    debugger_modify_program: bool,
) -> Result {
    let stmt_instrs = if is_native {
        InstrSeq::make_nativeimpl()
    } else {
        env.do_function(body, emit_defs)
    };

    let (begin_label, default_value_setters) =
        emit_param::emit_param_default_value_setter(emitter, env, is_native, args.pos, params)?;

    let header_content = make_header_content(
        env,
        args,
        params,
        tparams,
        decl_vars,
        need_local_this,
        is_native,
        is_generator,
    );
    let first_instr_is_label = match InstrSeq::first(&stmt_instrs) {
        Some(Instruct::ILabel(_)) => true,
        _ => false,
    };
    let header = if first_instr_is_label && InstrSeq::is_empty(&header_content) {
        InstrSeq::gather(vec![InstrSeq::make_entrynop(), begin_label])
    } else {
        InstrSeq::gather(vec![header_content, begin_label])
    };

    let mut body_instrs = InstrSeq::gather(vec![default_value_setters, stmt_instrs, header]);
    if debugger_modify_program {
        modify_prog_for_debugger_eval(&mut body_instrs);
    };
    Ok(body_instrs)
}

fn make_header_content(
    env: &mut Env,
    args: &Args,
    params: &[HhasParam],
    tparams: &[tast::Tparam],
    decl_vars: &[String],
    need_local_this: bool,
    is_native: bool,
    is_generator: bool,
) -> InstrSeq {
    let method_prolog = if is_native {
        InstrSeq::Empty
    } else {
        let should_emit_init_this = !args.scope.is_in_static_method()
            && (need_local_this
                || (args.scope.is_toplevel() && decl_vars.contains(&THIS.to_string())));
        emit_method_prolog(
            env,
            args.pos,
            params,
            args.ast_params,
            tparams,
            should_emit_init_this,
        )
    };

    let deprecation_warning = emit_deprecation_info(args.scope, args.deprecation_info);

    let generator_info = if is_generator {
        InstrSeq::gather(vec![InstrSeq::make_createcont(), InstrSeq::make_popc()])
    } else {
        InstrSeq::Empty
    };

    InstrSeq::gather(vec![generator_info, deprecation_warning, method_prolog])
}

fn make_decl_vars(
    emitter: &mut Emitter,
    scope: &Scope,
    immediate_params: &[tast::Tparam],
    params: &[HhasParam],
    body: &tast::Program,
    is_closure_body: bool,
) -> (bool, Vec<String>) {
    let has_this = scope.has_this();
    let is_toplevel = scope.is_toplevel();
    let is_in_static_method = scope.is_in_static_method();

    let explicit_use_set = &emitter.state().explicit_use_set;

    let (need_local_this, mut decl_vars) = decl_vars::from_ast(
        is_closure_body,
        has_this,
        is_toplevel,
        is_in_static_method,
        explicit_use_set,
        params,
        body,
    );

    if is_closure_body {
        let captured_vars = scope.get_captured_vars();
        move_this(&mut decl_vars);
        decl_vars.retain(|v| !captured_vars.contains(v));
        decl_vars.extend_from_slice(&captured_vars.as_slice());
    } else if has_reified(immediate_params) {
        decl_vars.push(String::from(string_utils::reified::GENERICS_LOCAL_NAME));
    }

    (need_local_this, decl_vars)
}

fn make_return_type_info(
    skip_awaitable: bool,
    is_native: bool,
    ret: &Option<aast::Hint>,
    tp_names: &[&str],
) -> Result<HhasTypeInfo> {
    let return_type_info = match ret {
        None => Ok(HhasTypeInfo::make(
            Some("".to_string()),
            hhas_type::constraint::Type::default(),
        )),
        Some(hint) => emit_type_hint::hint_to_type_info(
            &emit_type_hint::Kind::Return,
            skip_awaitable,
            false, // nullable
            tp_names,
            &hint,
        ),
    };
    if is_native {
        return return_type_info.map(|rti| {
            emit_type_hint::emit_type_constraint_for_native_function(tp_names, ret, rti)
        });
    };
    return_type_info
}

#[allow(unused_variables)]
fn make_env(
    _namespace: &namespace_env::Env,
    need_local_this: bool,
    _scope: &Scope,
    is_rx_body: bool,
) -> Env {
    let mut env = Env::default();
    env.with_need_local_this(need_local_this);
    env.with_rx_body(is_rx_body);
    //TODO(hrust) add scope
    //TODO(hrust) add namespace
    env
}

fn make_upper_bounds(
    emitter: &mut Emitter,
    immediate_tparams: &[tast::Tparam],
    skip_awaitable: bool,
) -> Vec<(String, Vec<HhasTypeInfo>)> {
    if emitter
        .options()
        .hack_compiler_flags
        .contains(CompilerFlags::ENFORCE_GENERICS_UB)
    {
        emit_generics_upper_bounds(immediate_tparams, skip_awaitable)
    } else {
        vec![]
    }
}

fn make_params(
    emitter: &mut Emitter,
    namespace: &namespace_env::Env,
    tp_names: &mut Vec<&str>,
    args: &Args,
    generate_defaults: bool,
) -> Result<Vec<HhasParam>> {
    emit_param::from_asts(
        emitter,
        tp_names,
        namespace,
        generate_defaults,
        args.scope,
        args.ast_params,
    )
}

pub fn make_body(
    emitter: &mut Emitter,
    mut body_instrs: InstrSeq,
    decl_vars: Vec<String>,
    is_memoize_wrapper: bool,
    is_memoize_wrapper_lsb: bool,
    upper_bounds: Vec<(String, Vec<HhasTypeInfo>)>,
    mut params: Vec<HhasParam>,
    return_type_info: Option<HhasTypeInfo>,
    doc_comment: Option<String>,
    env: Option<Env>,
) -> HhasBody {
    body_instrs.rewrite_user_labels(emitter.label_gen_mut());
    emit_adata::rewrite_typed_value(&mut body_instrs);
    if emitter
        .options()
        .hack_compiler_flags
        .contains(CompilerFlags::RELABEL)
    {
        label_rewriter::relabel_function(&mut params, &mut body_instrs);
    };
    let num_iters = if is_memoize_wrapper {
        0
    } else {
        emitter.iterator().count()
    };
    HhasBody {
        body_instrs,
        decl_vars,
        num_iters,
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        upper_bounds,
        params,
        return_type_info,
        doc_comment,
        env,
    }
}

fn emit_defs(_env: &mut Env, _defs: &tast::Program) -> InstrSeq {
    //TODO(hrust) implement
    InstrSeq::Empty
}

fn emit_method_prolog(
    _env: &mut Env,
    _pos: &Pos,
    _params: &[HhasParam],
    _ast_params: &[tast::FunParam],
    _tparams: &[tast::Tparam],
    _should_emit_init_this: bool,
) -> InstrSeq {
    //TODO(hrust) implement
    InstrSeq::Empty
}

fn emit_deprecation_info(_scope: &Scope, _deprecation_info: &Option<&[TypedValue]>) -> InstrSeq {
    //TODO(hrust) implement
    InstrSeq::Empty
}

#[allow(unused_variables)]
fn emit_statements(
    env: &mut Env,
    ret: Option<&aast::Hint>,
    return_value: InstrSeq,
    params: &[HhasParam],
    default_dropthrough: &mut Option<InstrSeq>,
    return_type_info: &HhasTypeInfo,
    is_async: bool,
    is_generator: bool,
) {
    let verify_return = match &return_type_info.user_type {
        Some(s) if s == "" => None,
        _ if return_type_info.has_type_constraint() && !is_generator => ret,
        _ => None,
    };
    if default_dropthrough.is_none() && is_async && verify_return.is_some() {
        *default_dropthrough = Some(InstrSeq::gather(vec![
            InstrSeq::make_null(),
            InstrSeq::make_verify_ret_type_c(),
            InstrSeq::make_retc(),
        ]));
    };
    let (num_out, verify_out) = emit_verify_out(params);

    //TODO(hrust) uncomment after porting emit_statement
    // emit_statement::set_verify_return(verify_return);
    // emit_statement::set_verify_out(verify_out);
    // emit_statement::set_num_out(num_out);
    // emit_statement::set_default_dropthrough(default_dropthrough);
    // emit_statement::set_default_return_value(return_value);
    // emit_statement::set_function_pos(pos);
}

fn emit_verify_out(_params: &[HhasParam]) -> (usize, InstrSeq) {
    //TODO(hrust) implement
    (0, InstrSeq::Empty)
}

pub fn emit_generics_upper_bounds(
    tparams: &[tast::Tparam],
    skip_awaitable: bool,
) -> Vec<(String, Vec<HhasTypeInfo>)> {
    let constraint_filter = |(kind, hint): &(ast_defs::ConstraintKind, tast::Hint)| {
        if let ast_defs::ConstraintKind::ConstraintAs = &kind {
            emit_type_hint::hint_to_type_info(
                &emit_type_hint::Kind::UpperBound,
                skip_awaitable,
                false, // nullable
                get_tp_names(tparams).as_slice(),
                &hint,
            )
            .ok() //TODO(hrust) propagate Err result
        } else {
            None
        }
    };
    let tparam_filter = |tparam: &tast::Tparam| {
        let ubs = tparam
            .constraints
            .iter()
            .filter_map(constraint_filter)
            .collect::<Vec<_>>();
        match &ubs[..] {
            [] => None,
            _ => Some((get_tp_name(tparam).to_owned(), ubs)),
        }
    };
    tparams.iter().filter_map(tparam_filter).collect::<Vec<_>>()
}

fn move_this(vars: &mut Vec<String>) {
    let this_keyword = String::from(THIS);
    if vars.contains(&this_keyword) {
        vars.retain(|s| s != THIS);
        vars.insert(0, this_keyword);
    }
}

fn has_reified(tparams: &[tast::Tparam]) -> bool {
    use aast::ReifyKind;
    for tparam in tparams.iter() {
        match tparam.reified {
            ReifyKind::SoftReified | ReifyKind::Reified => return true,
            _ => (),
        }
    }
    false
}

fn get_tp_name(tparam: &tast::Tparam) -> &str {
    let ast_defs::Id(_, name) = &tparam.name;
    name
}

fn get_tp_names(tparams: &[tast::Tparam]) -> Vec<&str> {
    tparams.iter().map(get_tp_name).collect()
}

fn modify_prog_for_debugger_eval(_body_instrs: &mut InstrSeq) {
    //TODO(hrust) implement
}

fn set_function_jmp_targets(emitter: &mut Emitter, env: &mut Env, scope: &Scope) -> bool {
    use ScopeItem::*;
    let function_state_key = match scope.items.as_slice() {
        [] => env::get_unique_id_for_main(),
        [.., Class(cls), Method(md)] | [.., Class(cls), Method(md), Lambda(_)] => {
            env::get_unique_id_for_method(cls, md)
        }
        [.., Function(fun)] => env::get_unique_id_for_function(fun),
        _ => panic!("unexpected scope shape"),
    };
    let global_state = emitter.state();
    match global_state.function_to_labels_map.get(&function_state_key) {
        Some(labels) => {
            env.jump_targets_gen.set_function_has_goto(true);
            env.jump_targets_gen.set_labels_in_function(labels.clone());
        }
        None => {
            env.jump_targets_gen.set_function_has_goto(false);
            env.jump_targets_gen
                .set_labels_in_function(<env::SMap<bool>>::new());
        }
    };
    global_state
        .functions_with_finally
        .contains(&function_state_key)
}

fn is_awaitable(hint: &tast::Hint) -> bool {
    use tast::{Hint, Hint_::*};
    let Hint(_, h) = hint;
    match &**h {
        Happly(ast_defs::Id(_, s), hs) if s == classes::AWAITABLE && hs.len() <= 1 => true,
        Hsoft(h) | Hoption(h) => is_awaitable(h),
        _ => false,
    }
}

fn report_error(_is_closure_body: bool, _scope: &Scope, _pos: &Pos) {
    //TODO(hrust) implement
}
