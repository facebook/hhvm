// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod emit_statement;
mod reified_generics_helpers;
mod try_finally_rewriter;

use emit_statement::emit_final_stmts;
use reified_generics_helpers as RGH;

use ast_body::AstBody;
use ast_class_expr::ClassExpr;
use ast_scope::{Scope, ScopeItem};
use emit_pos::emit_pos;
use env::{emitter::Emitter, Env};
use hash::HashSet;
use hhas_body::{HhasBody, HhasBodyEnv};
use hhas_param::HhasParam;
use hhas_type::HhasTypeInfo;
use hhbc_ast::{FCallArgs, FCallArgsFlags, Instruct, IsTypeOp, ParamId, Pseudo};
use hhbc_id::function;
use hhbc_string_utils as string_utils;
use instruction_sequence::{instr, Error, InstrSeq, Result};
use label::Label;
use local::{Local, LocalId};
use options::CompilerFlags;
use runtime::TypedValue;
use statement_state::StatementState;
use unique_id_builder::*;

use ocamlrep::rc::RcOc;
use oxidized::{aast, ast, ast_defs, doc_comment::DocComment, namespace_env, pos::Pos};

use ffi::{Maybe, Maybe::*, Pair, Slice, Str};

use bitflags::bitflags;
use indexmap::IndexSet;
use itertools::Either;

static THIS: &str = "$this";

/// Optional arguments for emit_body; use Args::default() for defaults
pub struct Args<'a, 'arena> {
    pub immediate_tparams: &'a Vec<ast::Tparam>,
    pub class_tparam_names: &'a [&'a str],
    pub ast_params: &'a Vec<ast::FunParam>,
    pub ret: Option<&'a ast::Hint>,
    pub pos: &'a Pos,
    pub deprecation_info: &'a Option<&'a [TypedValue<'arena>]>,
    pub doc_comment: Option<DocComment>,
    pub default_dropthrough: Option<InstrSeq<'arena>>,
    pub call_context: Option<String>,
    pub flags: Flags,
}

bitflags! {
    pub struct Flags: u8 {
        const HAS_COEFFECTS_LOCAL = 1 << 0;
        const SKIP_AWAITABLE = 1 << 1;
        const MEMOIZE = 1 << 2;
        const CLOSURE_BODY = 1 << 3;
        const NATIVE = 1 << 4;
        const ASYNC = 1 << 6;
        const DEBUGGER_MODIFY_PROGRAM = 1 << 7;
    }
}

pub fn emit_body_with_default_args<'b, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    namespace: RcOc<namespace_env::Env>,
    body: &'b ast::Program,
    return_value: InstrSeq<'arena>,
) -> Result<HhasBody<'arena>> {
    let args = Args {
        immediate_tparams: &vec![],
        class_tparam_names: &[],
        ast_params: &vec![],
        ret: None,
        pos: &Pos::make_none(),
        deprecation_info: &None,
        doc_comment: None,
        default_dropthrough: None,
        call_context: None,
        flags: Flags::empty(),
    };
    emit_body(
        alloc,
        emitter,
        namespace,
        Either::Left(body.as_slice()),
        return_value,
        Scope::toplevel(),
        args,
    )
    .map(|r| r.0)
}

pub fn emit_body<'b, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    namespace: RcOc<namespace_env::Env>,
    body: AstBody<'b>,
    return_value: InstrSeq<'arena>,
    scope: Scope<'_, 'arena>,
    args: Args<'_, 'arena>,
) -> Result<(HhasBody<'arena>, bool, bool)> {
    let tparams: Vec<ast::Tparam> = scope.get_tparams().into_iter().cloned().collect();
    let mut tp_names = get_tp_names(&tparams);
    let (is_generator, is_pair_generator) = generator::is_function_generator(&body);

    emitter.label_gen_mut().reset();
    emitter.iterator_mut().reset();

    let return_type_info = make_return_type_info(
        alloc,
        args.flags.contains(Flags::SKIP_AWAITABLE),
        args.flags.contains(Flags::NATIVE),
        args.ret,
        &tp_names,
    )?;

    let params = make_params(emitter, &mut tp_names, args.ast_params, &scope, args.flags)?;

    let upper_bounds = emit_generics_upper_bounds(
        alloc,
        args.immediate_tparams,
        args.class_tparam_names,
        args.flags.contains(Flags::SKIP_AWAITABLE),
    );
    let shadowed_tparams = emit_shadowed_tparams(args.immediate_tparams, args.class_tparam_names);
    let decl_vars = make_decl_vars(
        emitter,
        &scope,
        args.immediate_tparams,
        &params,
        &body,
        args.flags,
    )?;
    let mut env = make_env(alloc, namespace, scope, args.call_context);

    set_emit_statement_state(
        alloc,
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
    let num_locals = params.len() + decl_vars.len();
    local_gen.reset(LocalId::from_usize(num_locals));
    if should_reserve_locals {
        local_gen.reserve_retval_and_label_id_locals();
    };
    let body_instrs = make_body_instrs(
        emitter,
        &mut env,
        &params,
        &tparams,
        &decl_vars,
        body,
        is_generator,
        args.deprecation_info.clone(),
        args.pos,
        args.ast_params,
        args.flags,
    )?;
    Ok((
        make_body(
            alloc,
            emitter,
            body_instrs,
            decl_vars,
            false, // is_memoize_wrapper
            false, // is_memoize_wrapper_lsb
            upper_bounds,
            shadowed_tparams,
            params,
            Some(return_type_info),
            args.doc_comment.to_owned(),
            Some(&env),
        )?,
        is_generator,
        is_pair_generator,
    ))
}

fn make_body_instrs<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    tparams: &[ast::Tparam],
    decl_vars: &[String],
    body: AstBody<'_>,
    is_generator: bool,
    deprecation_info: Option<&[TypedValue<'arena>]>,
    pos: &Pos,
    ast_params: &[ast::FunParam],
    flags: Flags,
) -> Result<InstrSeq<'arena>> {
    let stmt_instrs = if flags.contains(Flags::NATIVE) {
        instr::nativeimpl()
    } else {
        env.do_function(emitter, &body, emit_ast_body)?
    };

    let (begin_label, default_value_setters) =
        emit_param::emit_param_default_value_setter(emitter, env, pos, params)?;

    let header_content = make_header_content(
        emitter,
        env,
        params,
        tparams,
        decl_vars,
        is_generator,
        deprecation_info,
        pos,
        ast_params,
        flags,
    )?;
    let first_instr_is_label = match InstrSeq::first(&stmt_instrs) {
        Some(Instruct::Pseudo(Pseudo::Label(_))) => true,
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

fn make_header_content<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    tparams: &[ast::Tparam],
    _decl_vars: &[String],
    is_generator: bool,
    deprecation_info: Option<&[TypedValue<'arena>]>,
    pos: &Pos,
    ast_params: &[ast::FunParam],
    flags: Flags,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let method_prolog = if flags.contains(Flags::NATIVE) {
        instr::empty()
    } else {
        emit_method_prolog(emitter, env, pos, params, ast_params, tparams)?
    };

    let deprecation_warning =
        emit_deprecation_info(alloc, &env.scope, deprecation_info, emitter.systemlib())?;

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

fn make_decl_vars<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    scope: &Scope<'a, 'arena>,
    immediate_tparams: &[ast::Tparam],
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    body: &AstBody<'_>,
    arg_flags: Flags,
) -> Result<Vec<String>> {
    let explicit_use_set = &emitter.emit_global_state().explicit_use_set;

    let mut decl_vars =
        decl_vars::from_ast(params, body, explicit_use_set).map_err(Error::unrecoverable)?;

    let mut decl_vars = if arg_flags.contains(Flags::CLOSURE_BODY) {
        let mut captured_vars = scope.get_captured_vars();
        move_this(&mut decl_vars);
        decl_vars.retain(|v| !captured_vars.contains(v));
        captured_vars.extend_from_slice(decl_vars.as_slice());
        captured_vars
    } else {
        match &scope.items[..] {
            [] | [.., ScopeItem::Class(_), _] => move_this(&mut decl_vars),
            _ => {}
        };
        decl_vars
    };

    if arg_flags.contains(Flags::HAS_COEFFECTS_LOCAL) {
        decl_vars.insert(0, string_utils::coeffects::LOCAL_NAME.into());
    }

    if !arg_flags.contains(Flags::CLOSURE_BODY)
        && immediate_tparams
            .iter()
            .any(|t| t.reified != ast::ReifyKind::Erased)
    {
        decl_vars.insert(0, string_utils::reified::GENERICS_LOCAL_NAME.into());
    }
    Ok(decl_vars)
}

pub fn emit_return_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    tp_names: &[&str],
    skip_awaitable: bool,
    ret: Option<&aast::Hint>,
) -> Result<HhasTypeInfo<'arena>> {
    match ret {
        None => Ok(HhasTypeInfo::make(
            Just(Str::new_str(alloc, "")),
            hhas_type::constraint::Constraint::default(),
        )),
        Some(hint) => emit_type_hint::hint_to_type_info(
            alloc,
            &emit_type_hint::Kind::Return,
            skip_awaitable,
            false, // nullable
            tp_names,
            hint,
        ),
    }
}

fn make_return_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    skip_awaitable: bool,
    is_native: bool,
    ret: Option<&aast::Hint>,
    tp_names: &[&str],
) -> Result<HhasTypeInfo<'arena>> {
    let return_type_info = emit_return_type_info(alloc, tp_names, skip_awaitable, ret);
    if is_native {
        return return_type_info.map(|rti| {
            emit_type_hint::emit_type_constraint_for_native_function(alloc, tp_names, ret, rti)
        });
    };
    return_type_info
}

pub fn make_env<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    namespace: RcOc<namespace_env::Env>,
    scope: Scope<'a, 'arena>,
    call_context: Option<String>,
) -> Env<'a, 'arena> {
    let mut env = Env::default(alloc, namespace);
    env.call_context = call_context;
    env.scope = scope;
    env
}

fn make_params<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    tp_names: &mut Vec<&str>,
    ast_params: &[ast::FunParam],
    scope: &Scope<'a, 'arena>,
    flags: Flags,
) -> Result<Vec<(HhasParam<'arena>, Option<(Label, ast::Expr)>)>> {
    let generate_defaults = !flags.contains(Flags::MEMOIZE);
    emit_param::from_asts(emitter, tp_names, generate_defaults, scope, ast_params)
}

pub fn make_body<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    mut body_instrs: InstrSeq<'arena>,
    decl_vars: Vec<String>,
    is_memoize_wrapper: bool,
    is_memoize_wrapper_lsb: bool,
    upper_bounds: Vec<Pair<Str<'arena>, Slice<'arena, HhasTypeInfo<'arena>>>>,
    shadowed_tparams: Vec<String>,
    mut params: Vec<(HhasParam<'arena>, Option<(Label, ast::Expr)>)>,
    return_type_info: Option<HhasTypeInfo<'arena>>,
    doc_comment: Option<DocComment>,
    opt_env: Option<&Env<'a, 'arena>>,
) -> Result<HhasBody<'arena>> {
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
    let body_env = if let Some(env) = opt_env {
        let is_namespaced = env.namespace.name.is_none();
        if let Some(cd) = env.scope.get_class() {
            Some(HhasBodyEnv {
                is_namespaced,
                class_info: Just(
                    (cd.get_kind().into(), Str::new_str(alloc, cd.get_name_str())).into(),
                ),
                parent_name: ClassExpr::get_parent_class_name(cd)
                    .as_ref()
                    .map(|s| Str::new_str(alloc, s))
                    .into(),
            })
        } else {
            Some(HhasBodyEnv {
                is_namespaced,
                class_info: Nothing,
                parent_name: Nothing,
            })
        }
    } else {
        None
    };

    // Pretty-print the DV initializer expression as a Hack source code string,
    // to make it available for reflection.
    params.iter_mut().for_each(|(p, default_value)| {
        p.default_value = Maybe::from(
            default_value
                .as_ref()
                .map(|(l, expr)| {
                    use print_expr::{Context, ExprEnv};
                    let ctx = Context::new(emitter);
                    let expr_env = ExprEnv {
                        codegen_env: body_env.as_ref(),
                    };
                    let mut buf = Vec::new();
                    print_expr::print_expr(&ctx, &mut buf, &expr_env, expr)
                        .map(|_| Pair(l.clone(), Str::from_vec(alloc, buf)))
                        .ok()
                })
                .flatten(),
        );
    });

    Ok(HhasBody {
        body_instrs: body_instrs.compact(alloc),
        decl_vars: Slice::fill_iter(
            alloc,
            decl_vars.into_iter().map(|s| Str::new_str(alloc, &s)),
        ),
        num_iters,
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        upper_bounds: Slice::fill_iter(alloc, upper_bounds.into_iter()),
        shadowed_tparams: Slice::fill_iter(
            alloc,
            shadowed_tparams
                .into_iter()
                .map(|s| Str::new_str(alloc, &s)),
        ),
        params: Slice::fill_iter(alloc, params.into_iter().map(|(p, _)| p)),
        return_type_info: return_type_info.into(),
        doc_comment: doc_comment.map(|c| Str::new_str(alloc, &(c.0).1)).into(),
        env: body_env.into(),
    })
}

fn emit_ast_body<'a, 'arena, 'decl>(
    env: &mut Env<'a, 'arena>,
    e: &mut Emitter<'arena, 'decl>,
    body: &AstBody<'_>,
) -> Result<InstrSeq<'arena>> {
    match body {
        Either::Left(p) => emit_defs(env, e, p),
        Either::Right(b) => emit_final_stmts(e, env, b),
    }
}

fn emit_defs<'a, 'arena, 'decl>(
    env: &mut Env<'a, 'arena>,
    emitter: &mut Emitter<'arena, 'decl>,
    prog: &[ast::Def],
) -> Result<InstrSeq<'arena>> {
    use ast::Def;
    fn emit_def<'a, 'arena, 'decl>(
        env: &mut Env<'a, 'arena>,
        emitter: &mut Emitter<'arena, 'decl>,
        def: &ast::Def,
    ) -> Result<InstrSeq<'arena>> {
        match def {
            Def::Stmt(s) => emit_statement::emit_stmt(emitter, env, s),
            Def::Namespace(ns) => emit_defs(env, emitter, &ns.1),
            _ => Ok(instr::empty()),
        }
    }
    fn aux<'a, 'arena, 'decl>(
        env: &mut Env<'a, 'arena>,
        emitter: &mut Emitter<'arena, 'decl>,
        defs: &[ast::Def],
    ) -> Result<InstrSeq<'arena>> {
        match defs {
            [Def::SetNamespaceEnv(ns), ..] => {
                env.namespace = RcOc::clone(ns);
                aux(env, emitter, &defs[1..])
            }
            [] => emit_statement::emit_dropthrough_return(emitter, env),
            [Def::Stmt(s)] => {
                // emit last statement in the list as final statement
                emit_statement::emit_final_stmt(emitter, env, s)
            }
            [Def::Stmt(s1), Def::Stmt(s2)] => match s2.1.as_markup() {
                Some(id) if id.1.is_empty() => emit_statement::emit_final_stmt(emitter, env, s1),
                _ => Ok(InstrSeq::gather(vec![
                    emit_statement::emit_stmt(emitter, env, s1)?,
                    emit_statement::emit_final_stmt(emitter, env, s2)?,
                ])),
            },
            [def, ..] => Ok(InstrSeq::gather(vec![
                emit_def(env, emitter, def)?,
                aux(env, emitter, &defs[1..])?,
            ])),
        }
    }

    match prog {
        [Def::Stmt(s), ..] if s.1.is_markup() => Ok(InstrSeq::gather(vec![
            emit_statement::emit_markup(emitter, env, s.1.as_markup().unwrap(), true)?,
            aux(env, emitter, &prog[1..])?,
        ])),
        [] | [_] => aux(env, emitter, prog),
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

pub fn has_type_constraint<'a, 'arena>(
    env: &Env<'a, 'arena>,
    ti: Option<&HhasTypeInfo<'_>>,
    ast_param: &ast::FunParam,
) -> (RGH::ReificationLevel, Option<ast::Hint>) {
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

pub fn emit_method_prolog<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    ast_params: &[ast::FunParam],
    tparams: &[ast::Tparam],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let mut make_param_instr =
        |param: &HhasParam<'arena>, ast_param: &ast::FunParam| -> Result<InstrSeq<'arena>> {
            let param_name =
                || ParamId::ParamNamed(Str::new_str(alloc, param.name.unsafe_as_str()));
            if param.is_variadic {
                Ok(instr::empty())
            } else {
                use RGH::ReificationLevel as L;
                match has_type_constraint(env, Option::from(param.type_info.as_ref()), ast_param) {
                    (L::Unconstrained, _) => Ok(instr::empty()),
                    (L::Not, _) => Ok(instr::verify_param_type(param_name())),
                    (L::Maybe, Some(h)) => {
                        if !RGH::happly_decl_has_reified_generics(emitter, &h) {
                            Ok(instr::verify_param_type(param_name()))
                        } else {
                            Ok(InstrSeq::gather(vec![
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
                            ]))
                        }
                    }
                    (L::Definitely, Some(h)) => {
                        if !RGH::happly_decl_has_reified_generics(emitter, &h) {
                            Ok(instr::verify_param_type(param_name()))
                        } else {
                            let check = instr::istypel(
                                Local::Named(Str::new_str(alloc, param.name.unsafe_as_str())),
                                IsTypeOp::Null,
                            );
                            let verify_instr = instr::verify_param_type_ts(param_name());
                            RGH::simplify_verify_type(emitter, env, pos, check, &h, verify_instr)
                        }
                    }
                    _ => Err(Error::unrecoverable("impossible")),
                }
            }
        };

    let ast_params = ast_params
        .iter()
        .filter(|p| !(p.is_variadic && p.name == "..."))
        .collect::<Vec<_>>();
    if params.len() != ast_params.len() {
        return Err(Error::unrecoverable("length mismatch"));
    }

    let mut instrs = Vec::with_capacity(1 + params.len());
    instrs.push(emit_pos(pos));
    for ((param, _), ast_param) in params.iter().zip(ast_params.into_iter()) {
        instrs.push(make_param_instr(param, ast_param)?);
    }
    Ok(InstrSeq::gather(instrs))
}

pub fn emit_deprecation_info<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    scope: &Scope<'a, 'arena>,
    deprecation_info: Option<&[TypedValue<'arena>]>,
    is_systemlib: bool,
) -> Result<InstrSeq<'arena>> {
    Ok(match deprecation_info {
        None => instr::empty(),
        Some(args) => {
            fn strip_id<'a>(id: &'a ast::Id) -> &'a str {
                string_utils::strip_global_ns(id.1.as_str())
            }
            let (class_name, trait_instrs, concat_instruction): (String, _, _) =
                match scope.get_class() {
                    None => ("".into(), instr::empty(), instr::empty()),
                    Some(c) if c.get_kind() == ast::ClassishKind::Ctrait => (
                        "::".into(),
                        InstrSeq::gather(vec![instr::selfcls(), instr::classname()]),
                        instr::concat(),
                    ),
                    Some(c) => (
                        strip_id(c.get_name()).to_string() + "::",
                        instr::empty(),
                        instr::empty(),
                    ),
                };

            let fn_name = match scope.items.last() {
                Some(ScopeItem::Function(f)) => strip_id(f.get_name()),
                Some(ScopeItem::Method(m)) => strip_id(m.get_name()),
                _ => {
                    return Err(Error::unrecoverable("deprecated functions must have names"));
                }
            };
            let deprecation_string = class_name
                + fn_name
                + ": "
                + (if args.is_empty() {
                    "deprecated function"
                } else if let TypedValue::String(s) = &args[0] {
                    s.unsafe_as_str()
                } else {
                    return Err(Error::unrecoverable(
                        "deprecated attribute first argument is not a string",
                    ));
                });
            let sampling_rate = if args.len() <= 1 {
                1i64
            } else if let Some(TypedValue::Int(i)) = args.get(1) {
                *i
            } else {
                return Err(Error::unrecoverable(
                    "deprecated attribute second argument is not an integer",
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
                    instr::nulluninit(),
                    instr::nulluninit(),
                    trait_instrs,
                    instr::string(alloc, deprecation_string),
                    concat_instruction,
                    instr::int(sampling_rate),
                    instr::int(error_code),
                    instr::fcallfuncd(
                        FCallArgs::new(
                            FCallArgsFlags::default(),
                            1,
                            3,
                            Slice::empty(),
                            Slice::empty(),
                            None,
                            None,
                        ),
                        function::from_raw_string(alloc, "trigger_sampled_error"),
                    ),
                    instr::popc(),
                ])
            }
        }
    })
}

fn set_emit_statement_state<'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    default_return_value: InstrSeq<'arena>,
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
    return_type_info: &HhasTypeInfo<'_>,
    return_type: Option<&ast::Hint>,
    pos: &Pos,
    default_dropthrough: Option<InstrSeq<'arena>>,
    flags: Flags,
    is_generator: bool,
) {
    let verify_return = match &return_type_info.user_type {
        Just(s) if s.unsafe_as_str() == "" => None,
        _ if return_type_info.has_type_constraint() && !is_generator => return_type.cloned(),
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
    let (num_out, verify_out) = emit_verify_out(alloc, params);

    emit_statement::set_state(
        emitter,
        StatementState {
            verify_return,
            default_return_value,
            default_dropthrough,
            verify_out,
            function_pos: pos.clone(),
            num_out,
        },
    )
}

fn emit_verify_out<'arena>(
    alloc: &'arena bumpalo::Bump,
    params: &[(HhasParam<'arena>, Option<(Label, ast::Expr)>)],
) -> (usize, InstrSeq<'arena>) {
    let param_instrs: Vec<InstrSeq<'arena>> = params
        .iter()
        .enumerate()
        .filter_map(|(i, (p, _))| {
            if p.is_inout {
                Some(InstrSeq::gather(vec![
                    instr::cgetl(Local::Named(Str::new_str(alloc, p.name.unsafe_as_str()))),
                    match p.type_info.as_ref() {
                        Just(HhasTypeInfo { user_type, .. })
                            if user_type.as_ref().map_or(true, |t| {
                                !(t.unsafe_as_str().ends_with("HH\\mixed")
                                    || t.unsafe_as_str().ends_with("HH\\dynamic"))
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

pub fn emit_generics_upper_bounds<'arena>(
    alloc: &'arena bumpalo::Bump,
    immediate_tparams: &[ast::Tparam],
    class_tparam_names: &[&str],
    skip_awaitable: bool,
) -> Vec<Pair<Str<'arena>, Slice<'arena, HhasTypeInfo<'arena>>>> {
    let constraint_filter = |(kind, hint): &(ast_defs::ConstraintKind, ast::Hint)| {
        if let ast_defs::ConstraintKind::ConstraintAs = &kind {
            let mut tparam_names = get_tp_names(immediate_tparams);
            tparam_names.extend_from_slice(class_tparam_names);
            emit_type_hint::hint_to_type_info(
                alloc,
                &emit_type_hint::Kind::UpperBound,
                skip_awaitable,
                false, // nullable
                &tparam_names,
                hint,
            )
            .ok() //TODO(hrust) propagate Err result
        } else {
            None
        }
    };
    let tparam_filter = |tparam: &ast::Tparam| {
        let ubs = tparam
            .constraints
            .iter()
            .filter_map(constraint_filter)
            .collect::<Vec<_>>();
        match &ubs[..] {
            [] => None,
            _ => Some(
                (
                    Str::new_str(alloc, get_tp_name(tparam)),
                    Slice::fill_iter(alloc, ubs.into_iter()),
                )
                    .into(),
            ),
        }
    };
    immediate_tparams
        .iter()
        .filter_map(tparam_filter)
        .collect::<Vec<_>>()
}

fn emit_shadowed_tparams(
    immediate_tparams: &[ast::Tparam],
    class_tparam_names: &[&str],
) -> Vec<String> {
    let s1 = get_tp_names_set(immediate_tparams);
    let s2: HashSet<&str> = class_tparam_names.iter().cloned().collect();
    // TODO(hrust): remove sort after Rust emitter released
    let mut r = s1
        .intersection(&s2)
        .map(|s| (*s).into())
        .collect::<Vec<_>>();
    r.sort();
    r
}

fn move_this(vars: &mut Vec<String>) {
    if vars.iter().any(|v| v == THIS) {
        vars.retain(|s| s != THIS);
        vars.push(String::from(THIS));
    }
}

fn get_tp_name(tparam: &ast::Tparam) -> &str {
    let ast_defs::Id(_, name) = &tparam.name;
    name
}

pub fn get_tp_names(tparams: &[ast::Tparam]) -> Vec<&str> {
    tparams.iter().map(get_tp_name).collect()
}

pub fn get_tp_names_set(tparams: &[ast::Tparam]) -> HashSet<&str> {
    tparams.iter().map(get_tp_name).collect()
}

fn modify_prog_for_debugger_eval<'arena>(_body_instrs: &mut InstrSeq<'arena>) {
    unimplemented!() // SF(2021-03-17): I found it like this.
}

fn set_function_jmp_targets<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
) -> bool {
    let function_state_key = get_unique_id_for_scope(&env.scope);
    emitter
        .emit_global_state()
        .functions_with_finally
        .contains(&function_state_key)
}
