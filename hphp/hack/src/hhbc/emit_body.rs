// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod emit_statement;
mod reified_generics_helpers;
mod try_finally_rewriter;

use ast_scope_rust::{Scope, ScopeItem};
use decl_vars_rust as decl_vars;
use emit_adata_rust as emit_adata;
use emit_expression_rust as emit_expression;
use emit_fatal_rust::raise_fatal_runtime;
use emit_param_rust as emit_param;
use emit_pos_rust::{emit_pos, emit_pos_then};
use emit_type_hint_rust as emit_type_hint;
use env::{
    emitter::{Context, Emitter},
    local, Env,
};
use generator_rust as generator;
use global_state::LazyState;
use hhas_body_rust::HhasBody;
use hhas_param_rust::HhasParam;
use hhas_type::Info as HhasTypeInfo;
use hhbc_ast_rust::{Instruct, IstypeOp, ParamId};
use hhbc_string_utils_rust as string_utils;
use instruction_sequence_rust::{instr, unrecoverable, Error, InstrSeq, Result};
use label_rewriter_rust as label_rewriter;
use naming_special_names_rust::classes;
use options::CompilerFlags;
use oxidized::{aast, ast as tast, ast_defs, doc_comment::DocComment, namespace_env, pos::Pos};
use reified_generics_helpers as RGH;
use runtime::TypedValue;

use bitflags::bitflags;
use indexmap::IndexSet;

static THIS: &'static str = "$this";

/// Optional arguments for emit_body; use Args::default() for defaults
pub struct Args<'a, 'b> {
    pub immediate_tparams: &'b Vec<tast::Tparam>,
    pub ast_params: &'b Vec<tast::FunParam>,
    pub ret: Option<&'a tast::Hint>,
    pub scope: &'b Scope<'a>,
    pub pos: &'b Pos,
    pub deprecation_info: &'b Option<&'b [TypedValue]>,
    pub doc_comment: Option<DocComment>,
    pub default_dropthrough: Option<InstrSeq>,
    pub call_context: Option<String>,
    pub flags: Flags,
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

pub fn emit_body_with_default_args<'a, 'b>(
    emitter: &mut Emitter,
    namespace: &namespace_env::Env,
    body: &'b tast::Program,
    return_value: InstrSeq,
) -> Result<HhasBody<'a>> {
    let args = Args {
        immediate_tparams: &vec![],
        ast_params: &vec![],
        ret: None,
        scope: &Scope::toplevel(),
        pos: &Pos::make_none(),
        deprecation_info: &None,
        doc_comment: None,
        default_dropthrough: None,
        call_context: None,
        flags: Flags::empty(),
    };
    emit_body(emitter, namespace, body, return_value, args).map(|r| r.0)
}

pub fn emit_body<'a, 'b>(
    emitter: &mut Emitter,
    namespace: &namespace_env::Env,
    body: &'b tast::Program,
    return_value: InstrSeq,
    args: Args<'a, '_>,
) -> Result<(HhasBody<'a>, bool, bool)> {
    if args.flags.contains(Flags::ASYNC)
        && args.flags.contains(Flags::SKIP_AWAITABLE)
        && args.ret.map_or(false, |hint| !is_awaitable(&hint))
    {
        report_error(
            args.flags.contains(Flags::CLOSURE_BODY),
            args.scope,
            args.pos,
        )?
    };

    let tparams = args
        .scope
        .get_tparams()
        .into_iter()
        .map(|tp| tp.clone())
        .collect::<Vec<tast::Tparam>>();
    let mut tp_names = get_tp_names(&tparams);
    let (is_generator, is_pair_generator) = generator::is_function_generator(&body);

    emitter.label_gen_mut().reset();
    emitter.iterator_mut().reset();

    let return_type_info = make_return_type_info(
        args.flags.contains(Flags::SKIP_AWAITABLE),
        args.flags.contains(Flags::NATIVE),
        args.ret,
        &tp_names,
    )?;

    let params = make_params(
        emitter,
        namespace,
        &mut tp_names,
        args.ast_params,
        args.scope,
        args.flags,
    )?;

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
        &body,
        args.flags,
    )?;
    let mut env = make_env(
        namespace,
        need_local_this,
        // TODO(hrust): avoid clone here.
        args.scope.clone(),
        args.call_context,
        args.flags.contains(Flags::RX_BODY),
    );

    set_emit_statement_state(
        emitter,
        return_value,
        &params,
        &return_type_info,
        args.ret,
        args.pos,
        args.default_dropthrough,
        args.flags,
        is_generator,
    );
    env.jump_targets_gen.reset();

    let should_reserve_locals = set_function_jmp_targets(emitter, &mut env);
    let local_gen = emitter.local_gen_mut();
    local_gen.reset(params.len() + decl_vars.len());
    if should_reserve_locals {
        local_gen.reserve_retval_and_label_id_locals();
    };
    let body_instrs = make_body_instrs(
        emitter,
        &mut env,
        &params,
        &tparams,
        &decl_vars,
        &body,
        need_local_this,
        is_generator,
        args.deprecation_info.clone(),
        &args.pos,
        &args.ast_params,
        args.flags,
    )?;
    Ok((
        make_body(
            emitter,
            body_instrs,
            decl_vars,
            false, // is_memoize_wrapper
            false, // is_memoize_wrapper_lsb
            upper_bounds,
            params,
            Some(return_type_info),
            args.doc_comment.to_owned(),
            Some(env),
        )?,
        is_generator,
        is_pair_generator,
    ))
}

fn make_body_instrs(
    emitter: &mut Emitter,
    env: &mut Env,
    params: &[HhasParam],
    tparams: &[tast::Tparam],
    decl_vars: &[String],
    body: &tast::Program,
    need_local_this: bool,
    is_generator: bool,
    deprecation_info: Option<&[TypedValue]>,
    pos: &Pos,
    ast_params: &[tast::FunParam],
    flags: Flags,
) -> Result {
    let stmt_instrs = if flags.contains(Flags::NATIVE) {
        instr::nativeimpl()
    } else {
        env.do_function(emitter, body, emit_defs)?
    };

    let (begin_label, default_value_setters) = emit_param::emit_param_default_value_setter(
        emitter,
        env,
        flags.contains(Flags::NATIVE),
        pos,
        params,
    )?;

    let header_content = make_header_content(
        emitter,
        env,
        params,
        tparams,
        decl_vars,
        need_local_this,
        is_generator,
        deprecation_info,
        pos,
        ast_params,
        flags,
    )?;
    let first_instr_is_label = match InstrSeq::first(&stmt_instrs) {
        Some(Instruct::ILabel(_)) => true,
        _ => false,
    };
    let header = if first_instr_is_label && InstrSeq::is_empty(&header_content) {
        InstrSeq::gather(vec![begin_label, instr::entrynop()])
    } else {
        InstrSeq::gather(vec![begin_label, header_content])
    };

    let mut body_instrs = InstrSeq::gather(vec![header, stmt_instrs, default_value_setters]);
    if flags.contains(Flags::DEBUGGER_MODIFY_PROGRAM) {
        modify_prog_for_debugger_eval(&mut body_instrs);
    };
    Ok(body_instrs)
}

fn make_header_content(
    emitter: &mut Emitter,
    env: &mut Env,
    params: &[HhasParam],
    tparams: &[tast::Tparam],
    decl_vars: &[String],
    need_local_this: bool,
    is_generator: bool,
    deprecation_info: Option<&[TypedValue]>,
    pos: &Pos,
    ast_params: &[tast::FunParam],
    flags: Flags,
) -> Result {
    let method_prolog = if flags.contains(Flags::NATIVE) {
        instr::empty()
    } else {
        let should_emit_init_this = !env.scope.is_in_static_method()
            && (need_local_this
                || (env.scope.is_toplevel() && decl_vars.contains(&THIS.to_string())));
        emit_method_prolog(
            emitter,
            env,
            pos,
            params,
            ast_params,
            tparams,
            should_emit_init_this,
        )?
    };

    let deprecation_warning =
        emit_deprecation_info(&env.scope, deprecation_info, emitter.systemlib())?;

    let generator_info = if is_generator {
        InstrSeq::gather(vec![instr::createcont(), instr::popc()])
    } else {
        instr::empty()
    };

    Ok(InstrSeq::gather(vec![
        method_prolog,
        deprecation_warning,
        generator_info,
    ]))
}

fn make_decl_vars(
    emitter: &mut Emitter,
    scope: &Scope,
    immediate_tparams: &[tast::Tparam],
    params: &[HhasParam],
    body: &tast::Program,
    arg_flags: Flags,
) -> Result<(bool, Vec<String>)> {
    let mut flags = decl_vars::Flags::empty();
    flags.set(decl_vars::Flags::HAS_THIS, scope.has_this());
    flags.set(decl_vars::Flags::IS_TOPLEVEL, scope.is_toplevel());
    flags.set(
        decl_vars::Flags::IS_IN_STATIC_METHOD,
        scope.is_in_static_method(),
    );
    flags.set(
        decl_vars::Flags::IS_CLOSURE_BODY,
        arg_flags.contains(Flags::CLOSURE_BODY),
    );

    let explicit_use_set = &emitter.emit_state().explicit_use_set;

    let (need_local_this, mut decl_vars) =
        decl_vars::from_ast(params, body, flags, explicit_use_set).map_err(unrecoverable)?;

    let mut decl_vars = if arg_flags.contains(Flags::CLOSURE_BODY) {
        let mut captured_vars = scope.get_captured_vars();
        move_this(&mut decl_vars);
        decl_vars.retain(|v| !captured_vars.contains(v));
        captured_vars.extend_from_slice(&decl_vars.as_slice());
        captured_vars
    } else {
        match &scope.items[..] {
            [] | [.., ScopeItem::Class(_), _] => move_this(&mut decl_vars),
            _ => (),
        };
        decl_vars
    };

    if !arg_flags.contains(Flags::CLOSURE_BODY)
        && immediate_tparams
            .iter()
            .any(|t| t.reified != tast::ReifyKind::Erased)
    {
        decl_vars.insert(0, string_utils::reified::GENERICS_LOCAL_NAME.into());
    }
    Ok((need_local_this, decl_vars))
}

pub fn emit_return_type_info(
    tp_names: &[&str],
    skip_awaitable: bool,
    ret: Option<&aast::Hint>,
) -> Result<HhasTypeInfo> {
    match ret {
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
    }
}

fn make_return_type_info(
    skip_awaitable: bool,
    is_native: bool,
    ret: Option<&aast::Hint>,
    tp_names: &[&str],
) -> Result<HhasTypeInfo> {
    let return_type_info = emit_return_type_info(tp_names, skip_awaitable, ret);
    if is_native {
        return return_type_info.map(|rti| {
            emit_type_hint::emit_type_constraint_for_native_function(tp_names, ret, rti)
        });
    };
    return_type_info
}

#[allow(unused_variables)]
pub fn make_env<'a>(
    _namespace: &namespace_env::Env,
    need_local_this: bool,
    scope: Scope<'a>,
    call_context: Option<String>,
    is_rx_body: bool,
) -> Env<'a> {
    let mut env = Env::default();
    env.with_need_local_this(need_local_this);
    env.with_rx_body(is_rx_body);
    env.scope = scope;
    env.call_context = call_context;
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
        .contains(CompilerFlags::EMIT_GENERICS_UB)
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
    ast_params: &[tast::FunParam],
    scope: &Scope,
    flags: Flags,
) -> Result<Vec<HhasParam>> {
    let generate_defaults = !flags.contains(Flags::MEMOIZE);
    emit_param::from_asts(
        emitter,
        tp_names,
        namespace,
        generate_defaults,
        scope,
        ast_params,
    )
}

pub fn make_body<'a>(
    emitter: &mut Emitter,
    mut body_instrs: InstrSeq,
    decl_vars: Vec<String>,
    is_memoize_wrapper: bool,
    is_memoize_wrapper_lsb: bool,
    upper_bounds: Vec<(String, Vec<HhasTypeInfo>)>,
    mut params: Vec<HhasParam>,
    return_type_info: Option<HhasTypeInfo>,
    doc_comment: Option<DocComment>,
    env: Option<Env<'a>>,
) -> Result<HhasBody<'a>> {
    body_instrs.rewrite_user_labels(emitter.label_gen_mut());
    emit_adata::rewrite_typed_values(emitter, &mut body_instrs)?;
    if emitter
        .options()
        .hack_compiler_flags
        .contains(CompilerFlags::RELABEL)
    {
        label_rewriter::relabel_function(&mut params, &mut body_instrs);
    }
    let num_iters = if is_memoize_wrapper {
        0
    } else {
        emitter.iterator().count()
    };
    Ok(HhasBody {
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
    })
}

fn emit_defs(env: &mut Env, emitter: &mut Emitter, prog: &[tast::Def]) -> Result {
    use tast::Def;
    fn emit_def(env: &mut Env, emitter: &mut Emitter, def: &tast::Def) -> Result {
        fn get_order(emit_id: &Option<tast::EmitId>) -> &isize {
            match emit_id {
                None | Some(tast::EmitId::Anonymous) => {
                    panic!("Expected closure_convert to annotate def with order number");
                }
                Some(tast::EmitId::EmitId(n)) => n,
            }
        };
        match def {
            Def::Stmt(s) => emit_statement::emit_stmt(emitter, env, s),
            Def::Namespace(ns) => emit_defs(env, emitter, &ns.1),
            Def::Class(cd) => {
                let make_def_instr = |num| {
                    if emitter.context().systemlib() {
                        instr::defclsnop(num)
                    } else {
                        instr::defcls(num)
                    }
                };
                let tast::Id(pos, _) = &(*cd).name;
                match &(*cd).emit_id {
                    Some(tast::EmitId::EmitId(n)) => Ok(emit_pos_then(&pos, make_def_instr(*n))),
                    Some(tast::EmitId::Anonymous) => Ok(instr::empty()),
                    None => panic!(
                        "Expected toplevel class declaration to be converted in closure_convert"
                    ),
                }
            }
            Def::Typedef(td) => {
                let tast::Id(pos, _) = &(*td).name;
                let num = get_order(&(*td).emit_id);
                Ok(emit_pos_then(&pos, instr::deftypealias(*num)))
            }
            Def::RecordDef(rd) => {
                let tast::Id(pos, _) = &(*rd).name;
                let num = get_order(&(*rd).emit_id);
                Ok(emit_pos_then(&pos, instr::defrecord(*num)))
            }
            Def::Constant(c) => {
                let tast::Id(pos, _) = &(*c).name;
                let num = get_order(&(*c).emit_id);
                Ok(emit_pos_then(&pos, instr::defcns(*num)))
            }
            _ => Ok(instr::empty()),
        }
    };
    fn aux(env: &mut Env, emitter: &mut Emitter, defs: &[tast::Def]) -> Result {
        match defs {
            [Def::SetNamespaceEnv(ns), ..] => {
                env.namespace = (&**ns.as_ref()).clone();
                aux(env, emitter, &defs[1..])
            }
            [] => emit_statement::emit_dropthrough_return(emitter, env),
            [Def::Stmt(s)] => {
                // emit last statement in the list as final statement
                emit_statement::emit_final_stmt(emitter, env, &s)
            }
            [Def::Stmt(s1), Def::Stmt(s2)] => match s2.1.as_markup() {
                Some(id) if id.1.is_empty() => emit_statement::emit_final_stmt(emitter, env, &s1),
                _ => Ok(InstrSeq::gather(vec![
                    emit_statement::emit_stmt(emitter, env, s1)?,
                    emit_statement::emit_final_stmt(emitter, env, &s2)?,
                ])),
            },
            [def, ..] => Ok(InstrSeq::gather(vec![
                emit_def(env, emitter, def)?,
                aux(env, emitter, &defs[1..])?,
            ])),
        }
    };
    match prog {
        [Def::Stmt(s), ..] if s.1.is_markup() => Ok(InstrSeq::gather(vec![
            emit_statement::emit_markup(emitter, env, s.1.as_markup().unwrap(), true)?,
            aux(env, emitter, &prog[1..])?,
        ])),
        [] | [_] => aux(env, emitter, &prog[..]),
        [def, ..] => {
            let i1 = emit_def(env, emitter, def)?;
            if i1.is_empty() {
                emit_defs(env, emitter, &prog[1..])
            } else {
                Ok(InstrSeq::gather(vec![i1, aux(env, emitter, &prog[1..])?]))
            }
        }
    }
}

pub fn has_type_constraint(
    env: &Env,
    ti: Option<&HhasTypeInfo>,
    ast_param: &tast::FunParam,
) -> (RGH::ReificationLevel, Option<tast::Hint>) {
    use RGH::ReificationLevel as L;
    match (ti, &ast_param.type_hint.1) {
        (Some(ti), Some(h)) if ti.has_type_constraint() => {
            // TODO(hrust): how to avoid clone on h
            let h = RGH::remove_erased_generics(env, h.clone());
            (RGH::has_reified_type_constraint(env, &h), Some(h))
        }
        _ => (L::Unconstrained, None),
    }
}

pub fn emit_method_prolog(
    emitter: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    params: &[HhasParam],
    ast_params: &[tast::FunParam],
    tparams: &[tast::Tparam],
    should_emit_init_this: bool,
) -> Result {
    let mut make_param_instr =
        |(param, ast_param): (&HhasParam, &tast::FunParam)| -> Result<Option<InstrSeq>> {
            let param_name = &param.name;
            let param_name = || ParamId::ParamNamed(param_name.into());
            if param.is_variadic {
                Ok(None)
            } else {
                use RGH::ReificationLevel as L;
                match has_type_constraint(env, param.type_info.as_ref(), ast_param) {
                    (L::Unconstrained, _) => Ok(None),
                    (L::Not, _) => Ok(Some(instr::verify_param_type(param_name()))),
                    (L::Maybe, Some(h)) => Ok(Some(InstrSeq::gather(vec![
                        emit_expression::get_type_structure_for_hint(
                            emitter,
                            tparams
                                .iter()
                                .map(|fp| fp.name.1.as_str())
                                .collect::<Vec<_>>()
                                .as_slice(),
                            &IndexSet::new(),
                            &h,
                        )?,
                        instr::verify_param_type_ts(param_name()),
                    ]))),
                    (L::Definitely, Some(h)) => {
                        let check = instr::istypel(
                            local::Type::Named((&param.name).into()),
                            IstypeOp::OpNull,
                        );
                        let verify_instr = instr::verify_param_type_ts(param_name());
                        RGH::simplify_verify_type(emitter, env, pos, check, &h, verify_instr)
                            .map(Some)
                    }
                    _ => Err(unrecoverable("impossible")),
                }
            }
        };

    let ast_params = ast_params
        .iter()
        .filter(|p| !(p.is_variadic && p.name == "..."))
        .collect::<Vec<_>>();
    if params.len() != ast_params.len() {
        return Err(Error::Unrecoverable("lenth mismatch".into()));
    }
    let param_instrs = params
        .iter()
        .zip(ast_params.into_iter())
        .filter_map(|p| make_param_instr(p).transpose())
        .collect::<Result<Vec<_>>>()?;

    let mut instrs = vec![emit_pos(pos)];
    if should_emit_init_this {
        instrs.push(instr::initthisloc(local::Type::Named(THIS.into())))
    }
    instrs.extend_from_slice(param_instrs.as_slice());
    Ok(InstrSeq::gather(instrs))
}

pub fn emit_deprecation_info(
    scope: &Scope,
    deprecation_info: Option<&[TypedValue]>,
    is_systemlib: bool,
) -> Result<InstrSeq> {
    Ok(match deprecation_info {
        None => instr::empty(),
        Some(args) => {
            fn strip_id<'a>(id: &'a tast::Id) -> &'a str {
                string_utils::strip_global_ns(id.1.as_str())
            }
            let (class_name, trait_instrs, concat_instruction): (String, _, _) =
                match scope.get_class() {
                    None => ("".into(), instr::empty(), instr::empty()),
                    Some(c) if c.kind == tast::ClassKind::Ctrait => (
                        "::".into(),
                        InstrSeq::gather(vec![instr::self_(), instr::classname()]),
                        instr::concat(),
                    ),
                    Some(c) => (
                        strip_id(&c.name).to_string() + "::",
                        instr::empty(),
                        instr::empty(),
                    ),
                };

            let fn_name = match scope.items.get(0) {
                Some(ScopeItem::Function(f)) => strip_id(&f.name),
                Some(ScopeItem::Method(m)) => strip_id(&m.name),
                _ => {
                    return Err(Error::Unrecoverable(
                        "deprecated functions must have names".into(),
                    ))
                }
            };
            let deprecation_string = class_name
                + fn_name
                + ": "
                + (if args.is_empty() {
                    "deprecated function"
                } else if let TypedValue::String(s) = &args[0] {
                    s.as_str()
                } else {
                    return Err(Error::Unrecoverable(
                        "deprecated attribute first argument is not a string".into(),
                    ));
                });
            let sampling_rate = if args.len() <= 1 {
                1i64
            } else if let Some(TypedValue::Int(i)) = args.get(1) {
                *i
            } else {
                return Err(Error::Unrecoverable(
                    "deprecated attribute second argument is not an integer".into(),
                ));
            };
            let error_code = if is_systemlib {
                /*E_DEPRECATED*/
                8192
            } else {
                /*E_USER_DEPRECATED*/
                16384
            };

            if sampling_rate <= 0 {
                instr::empty()
            } else {
                InstrSeq::gather(vec![
                    trait_instrs,
                    instr::string(deprecation_string),
                    concat_instruction,
                    instr::int64(sampling_rate),
                    instr::int(error_code),
                    instr::trigger_sampled_error(),
                    instr::popc(),
                ])
            }
        }
    })
}

fn set_emit_statement_state(
    emitter: &mut Emitter,
    default_return_value: InstrSeq,
    params: &[HhasParam],
    return_type_info: &HhasTypeInfo,
    return_type: Option<&tast::Hint>,
    pos: &Pos,
    default_dropthrough: Option<InstrSeq>,
    flags: Flags,
    is_generator: bool,
) {
    let verify_return = match &return_type_info.user_type {
        Some(s) if s == "" => None,
        _ if return_type_info.has_type_constraint() && !is_generator => {
            return_type.map(|h| h.clone())
        }
        _ => None,
    };
    let default_dropthrough = if default_dropthrough.is_some() {
        default_dropthrough
    } else if flags.contains(Flags::ASYNC) && verify_return.is_some() {
        Some(InstrSeq::gather(vec![
            instr::null(),
            instr::verify_ret_type_c(),
            instr::retc(),
        ]))
    } else {
        None
    };
    let (num_out, verify_out) = emit_verify_out(params);

    emit_statement::set_state(
        emitter,
        emit_statement::State {
            verify_return,
            default_return_value,
            default_dropthrough,
            verify_out,
            function_pos: pos.clone(),
            num_out,
        },
    )
}

fn emit_verify_out(params: &[HhasParam]) -> (usize, InstrSeq) {
    let param_instrs: Vec<InstrSeq> = params
        .iter()
        .enumerate()
        .filter_map(|(i, p)| {
            if p.is_inout {
                Some(InstrSeq::gather(vec![
                    instr::cgetl(local::Type::Named(p.name.clone())),
                    match p.type_info.as_ref() {
                        Some(HhasTypeInfo { user_type, .. })
                            if user_type.as_ref().map_or(true, |t| {
                                !(t.ends_with("HH\\mixed") || t.ends_with("HH\\dynamic"))
                            }) =>
                        {
                            instr::verify_out_type(ParamId::ParamUnnamed(i as isize))
                        }
                        _ => instr::empty(),
                    },
                ]))
            } else {
                None
            }
        })
        .collect();
    (
        param_instrs.len(),
        InstrSeq::gather(param_instrs.into_iter().rev().collect()),
    )
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
    if vars.iter().any(|v| v == &THIS) {
        vars.retain(|s| s != THIS);
        vars.push(String::from(THIS));
    }
}

fn get_tp_name(tparam: &tast::Tparam) -> &str {
    let ast_defs::Id(_, name) = &tparam.name;
    name
}

pub fn get_tp_names(tparams: &[tast::Tparam]) -> Vec<&str> {
    tparams.iter().map(get_tp_name).collect()
}

fn modify_prog_for_debugger_eval(_body_instrs: &mut InstrSeq) {
    unimplemented!()
}

fn set_function_jmp_targets(emitter: &mut Emitter, env: &mut Env) -> bool {
    use ScopeItem::*;
    let function_state_key = match env.scope.items.as_slice() {
        [] => env::get_unique_id_for_main(),
        [.., Class(cls), Method(md)] | [.., Class(cls), Method(md), Lambda(_)] => {
            env::get_unique_id_for_method(cls, md)
        }
        [.., Function(fun)] => env::get_unique_id_for_function(fun),
        _ => panic!("unexpected scope shape"),
    };
    let global_state = emitter.emit_state();
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

fn report_error(is_closure_body: bool, scope: &Scope, pos: &Pos) -> Result<()> {
    let msg: String = if is_closure_body {
        "Return type hint for async closure must be awaitable".into()
    } else {
        let mut scope = scope.items.iter();
        let s1 = scope.next();
        let s2 = scope.next();
        use ScopeItem as S;
        let (kind, name) = match (s1, s2) {
            (Some(S::Function(f)), _) => ("function", string_utils::strip_ns(&f.name.1).to_owned()),
            (Some(S::Method(m)), Some(S::Class(c))) => (
                "method",
                string_utils::strip_ns(&c.name.1).to_owned() + "::" + m.name.1.as_str(),
            ),
            _ => return Err(Error::Unrecoverable("Unexpected".into())),
        };
        format!(
            "Return type hint for async {} {}() must be awaitable",
            kind, name
        )
    };
    Err(raise_fatal_runtime(pos, msg))
}
