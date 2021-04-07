// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod emit_statement;
mod reified_generics_helpers;
mod try_finally_rewriter;

use aast::TypeHint;
use aast_defs::{Hint, Hint_::*};
use ast_body::AstBody;
use ast_class_expr_rust::ClassExpr;
use ast_scope_rust::{Scope, ScopeItem};
use decl_vars_rust as decl_vars;
use emit_adata_rust as emit_adata;
use emit_expression_rust as emit_expression;
use emit_fatal_rust::{emit_fatal_runtime, raise_fatal_parse};
use emit_param_rust as emit_param;
use emit_pos_rust::emit_pos;
use emit_statement::emit_final_stmts;
use emit_type_hint_rust as emit_type_hint;
use env::{emitter::Emitter, Env};
use generator_rust as generator;
use hhas_body_rust::HhasBody;
use hhas_body_rust::HhasBodyEnv;
use hhas_param_rust::HhasParam;
use hhas_type::Info as HhasTypeInfo;
use hhbc_ast_rust::{
    ClassKind, FcallArgs, FcallFlags, Instruct, IstypeOp, MemberKey, MemberOpMode, ParamId,
    QueryOp, ReadOnlyOp,
};
use hhbc_id_rust::function;
use hhbc_string_utils_rust as string_utils;
use instruction_sequence::{flatten, instr, unrecoverable, Error, InstrSeq, Result};
use label_rewriter_rust as label_rewriter;
use label_rust::Label;
use ocamlrep::rc::RcOc;
use options::{CompilerFlags, LangFlags};
use oxidized::{
    aast, aast_defs, ast as tast, ast_defs, doc_comment::DocComment, namespace_env, pos::Pos,
};
use reified_generics_helpers as RGH;
use runtime::TypedValue;
use statement_state::StatementState;
use unique_id_builder::*;

use bitflags::bitflags;
use indexmap::IndexSet;
use itertools::Either;
use std::collections::HashSet;

static THIS: &'static str = "$this";

/// Optional arguments for emit_body; use Args::default() for defaults
pub struct Args<'a> {
    pub immediate_tparams: &'a Vec<tast::Tparam>,
    pub class_tparam_names: &'a [&'a str],
    pub ast_params: &'a Vec<tast::FunParam>,
    pub ret: Option<&'a tast::Hint>,
    pub pos: &'a Pos,
    pub deprecation_info: &'a Option<&'a [TypedValue]>,
    pub doc_comment: Option<DocComment>,
    pub default_dropthrough: Option<InstrSeq>,
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
        const RX_BODY = 1 << 5;
        const ASYNC = 1 << 6;
        const DEBUGGER_MODIFY_PROGRAM = 1 << 7;
    }
}

pub fn emit_body_with_default_args<'b>(
    emitter: &mut Emitter,
    namespace: RcOc<namespace_env::Env>,
    body: &'b tast::Program,
    return_value: InstrSeq,
) -> Result<HhasBody> {
    let args = Args {
        immediate_tparams: &vec![],
        class_tparam_names: &vec![],
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
        emitter,
        namespace,
        Either::Left(body),
        return_value,
        Scope::toplevel(),
        args,
    )
    .map(|r| r.0)
}

pub fn emit_body<'b>(
    emitter: &mut Emitter,
    namespace: RcOc<namespace_env::Env>,
    body: AstBody<'b>,
    return_value: InstrSeq,
    scope: Scope<'_>,
    args: Args<'_>,
) -> Result<(HhasBody, bool, bool)> {
    let tparams = scope
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
        namespace.as_ref(),
        &mut tp_names,
        args.ast_params,
        &scope,
        args.flags,
    )?;

    let upper_bounds = emit_generics_upper_bounds(
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
    let mut env = make_env(
        namespace,
        scope,
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
        body,
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

fn make_body_instrs(
    emitter: &mut Emitter,
    env: &mut Env,
    params: &[HhasParam],
    tparams: &[tast::Tparam],
    decl_vars: &[String],
    body: AstBody,
    is_generator: bool,
    deprecation_info: Option<&[TypedValue]>,
    pos: &Pos,
    ast_params: &[tast::FunParam],
    flags: Flags,
) -> Result {
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
    _decl_vars: &[String],
    is_generator: bool,
    deprecation_info: Option<&[TypedValue]>,
    pos: &Pos,
    ast_params: &[tast::FunParam],
    flags: Flags,
) -> Result {
    let method_prolog = if flags.contains(Flags::NATIVE) {
        instr::empty()
    } else {
        emit_method_prolog(emitter, env, pos, params, ast_params, tparams)?
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
    body: &AstBody,
    arg_flags: Flags,
) -> Result<Vec<String>> {
    let explicit_use_set = &emitter.emit_global_state().explicit_use_set;

    let mut decl_vars =
        decl_vars::from_ast(params, body, explicit_use_set).map_err(unrecoverable)?;

    let mut decl_vars = if arg_flags.contains(Flags::CLOSURE_BODY) {
        let mut captured_vars = scope.get_captured_vars();
        move_this(&mut decl_vars);
        decl_vars.retain(|v| !captured_vars.contains(v));
        captured_vars.extend_from_slice(&decl_vars.as_slice());
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
            .any(|t| t.reified != tast::ReifyKind::Erased)
    {
        decl_vars.insert(0, string_utils::reified::GENERICS_LOCAL_NAME.into());
    }
    Ok(decl_vars)
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

pub fn make_env<'a>(
    namespace: RcOc<namespace_env::Env>,
    scope: Scope<'a>,
    call_context: Option<String>,
    is_rx_body: bool,
) -> Env<'a> {
    let mut env = Env::default(namespace);
    env.call_context = call_context;
    env.scope = scope;
    env.with_rx_body(is_rx_body);
    env
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
    shadowed_tparams: Vec<String>,
    mut params: Vec<HhasParam>,
    return_type_info: Option<HhasTypeInfo>,
    doc_comment: Option<DocComment>,
    opt_env: Option<&Env<'a>>,
) -> Result<HhasBody> {
    body_instrs = flatten(body_instrs);
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
    let body_env = if let Some(env) = opt_env {
        let is_namespaced = env.namespace.name.is_none();
        if let Some(cd) = env.scope.get_class() {
            Some(HhasBodyEnv {
                is_namespaced,
                class_info: Some((cd.get_kind(), cd.get_name_str().to_string())),
                parent_name: ClassExpr::get_parent_class_name(cd),
            })
        } else {
            Some(HhasBodyEnv {
                is_namespaced,
                class_info: None,
                parent_name: None,
            })
        }
    } else {
        None
    };
    Ok(HhasBody {
        body_instrs,
        decl_vars,
        num_iters,
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        upper_bounds,
        shadowed_tparams,
        params,
        return_type_info,
        doc_comment,
        env: body_env,
    })
}

fn emit_ast_body(env: &mut Env, e: &mut Emitter, body: &AstBody) -> Result {
    match body {
        Either::Left(p) => emit_defs(env, e, p),
        Either::Right(b) => emit_final_stmts(e, env, b),
    }
}

fn emit_defs(env: &mut Env, emitter: &mut Emitter, prog: &[tast::Def]) -> Result {
    use tast::Def;
    fn emit_def(env: &mut Env, emitter: &mut Emitter, def: &tast::Def) -> Result {
        match def {
            Def::Stmt(s) => emit_statement::emit_stmt(emitter, env, s),
            Def::Namespace(ns) => emit_defs(env, emitter, &ns.1),
            _ => Ok(instr::empty()),
        }
    }
    fn aux(env: &mut Env, emitter: &mut Emitter, defs: &[tast::Def]) -> Result {
        match defs {
            [Def::SetNamespaceEnv(ns), ..] => {
                env.namespace = RcOc::clone(&ns);
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
    }
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

////////////////////////////////////////////////////////////////////////////////
// atom_helpers

mod atom_helpers {
    use crate::*;
    use aast_defs::ReifyKind::Erased;
    use ast_defs::Id;
    use instruction_sequence::{instr, unrecoverable, InstrSeq, Result};
    use local::Type::Named;
    use tast::Tparam;

    fn strip_ns(name: &str) -> &str {
        match name.chars().next() {
            Some('\\') => &name[1..],
            _ => name,
        }
    }

    pub fn is_generic(name: &str, tparams: &[Tparam]) -> bool {
        tparams.iter().any(|t| match t.name {
            Id(_, ref s) => s == self::strip_ns(name),
        })
    }

    pub fn is_erased_generic(name: &str, tparams: &[Tparam]) -> bool {
        tparams.iter().any(|t| match t.name {
            Id(_, ref s) if s == self::strip_ns(name) => t.reified == Erased,
            _ => false,
        })
    }

    pub fn index_of_generic(tparams: &[Tparam], name: &str) -> Result<usize> {
        match tparams.iter().enumerate().find_map(|(i, t)| match t.name {
            Id(_, ref s) if s == self::strip_ns(name) => Some(i),
            _ => None,
        }) {
            Some(u) => Ok(u),
            None => Err(unrecoverable("Expected generic")),
        }
    }

    pub fn emit_clscnsl(
        param: &HhasParam,
        pos: &Pos,
        cls_instrs: InstrSeq,
        msg: &str,
        label_not_a_class: Label,
        label_done: Label,
    ) -> Result {
        let param_name = &param.name;
        let loc = Named(param_name.into());
        Ok(InstrSeq::gather(vec![
            cls_instrs,
            instr::classgetc(),
            instr::clscnsl(loc.clone()),
            instr::popl(loc),
            instr::jmp(label_done.clone()),
            instr::label(label_not_a_class),
            emit_fatal_runtime(&pos, msg),
            instr::label(label_done),
        ]))
    }
} //mod atom_helpers

////////////////////////////////////////////////////////////////////////////////
// atom_instrs

fn atom_instrs(
    emitter: &mut Emitter,
    env: &mut Env,
    param: &HhasParam,
    ast_param: &tast::FunParam,
    tparams: &[tast::Tparam],
) -> Result<Option<InstrSeq>> {
    if !param
        .user_attributes
        .iter()
        .any(|a| a.is(|x| x == "__Atom"))
    {
        return Ok(None); // Not an atom. Nothing to do.
    }
    match &ast_param.type_hint {
        TypeHint(_, None) => Err(raise_fatal_parse(
            &ast_param.pos,
            "__Atom param type hint unavailable",
        )),
        TypeHint(_, Some(Hint(_, h))) => {
            let label_done = emitter.label_gen_mut().next_regular();
            let label_not_a_class = emitter.label_gen_mut().next_regular();
            match &**h {
                Happly(ast_defs::Id(_, ref ctor), vec) if ctor == "\\HH\\MemberOf" => {
                    match &vec[..] {
                        [hint, _] => {
                            let Hint(_, e) = hint;
                            match &**e {
                                // Immediate type.
                                Happly(ast_defs::Id(pos, ref tag), _) => {
                                    if atom_helpers::is_erased_generic(tag, tparams) {
                                        Err(raise_fatal_parse(
                                            &pos,
                                            "Erased generic as HH\\MemberOf enum type",
                                        ))
                                    } else {
                                        if !atom_helpers::is_generic(tag, tparams) {
                                            //'tag' is just a name.
                                            Ok(Some(atom_helpers::emit_clscnsl(
                                                param,
                                                &pos,
                                                InstrSeq::gather(vec![
                                                    emit_expression::emit_expr(
                                                        emitter,
                                                        env,
                                                        &(tast::Expr(
                                                            Pos::make_none(),
                                                            aast::Expr_::String(
                                                                bstr::BString::from(tag.to_owned()),
                                                            ),
                                                        )),
                                                    )?,
                                                    emit_expression::emit_expr(
                                                        emitter,
                                                        env,
                                                        &(tast::Expr(
                                                            Pos::make_none(),
                                                            aast::Expr_::True,
                                                        )),
                                                    )?,
                                                    instr::oodeclexists(ClassKind::Class),
                                                    instr::jmpz(label_not_a_class.clone()),
                                                    emit_expression::emit_expr(
                                                        emitter,
                                                        env,
                                                        &(tast::Expr(
                                                            Pos::make_none(),
                                                            aast::Expr_::String(
                                                                bstr::BString::from(tag.to_owned()),
                                                            ),
                                                        )),
                                                    )?,
                                                ]),
                                                "Type is not a class",
                                                label_not_a_class,
                                                label_done,
                                            )?))
                                        } else {
                                            //'tag' is a reified generic.
                                            Ok(Some(atom_helpers::emit_clscnsl(
                                                param,
                                                &pos,
                                                InstrSeq::gather(vec![
                                                    emit_expression::emit_reified_generic_instrs(
                                                        &Pos::make_none(),
                                                        true,
                                                        atom_helpers::index_of_generic(
                                                            tparams, tag,
                                                        )?,
                                                    )?,
                                                    instr::basec(0, MemberOpMode::ModeNone),
                                                    instr::querym(
                                                        1,
                                                        QueryOp::CGetQuiet,
                                                        MemberKey::ET(
                                                            "classname".into(),
                                                            ReadOnlyOp::Any,
                                                        ),
                                                    ),
                                                    instr::dup(),
                                                    instr::istypec(IstypeOp::OpNull),
                                                    instr::jmpnz(label_not_a_class.clone()),
                                                ]),
                                                "Generic type parameter does not resolve to a class",
                                                label_not_a_class,
                                                label_done,
                                            )?))
                                        }
                                    }
                                }
                                // Type constant.
                                Haccess(Hint(_, h), _) => {
                                    match &**h {
                                        Happly(ast_defs::Id(pos, ref tag), _) => {
                                            if atom_helpers::is_erased_generic(tag, tparams) {
                                                Err(raise_fatal_parse(
                                                    &pos,
                                                    "Erased generic as HH\\MemberOf enum type",
                                                ))
                                            } else {
                                                //'tag' is a type constant.
                                                Ok(Some(atom_helpers::emit_clscnsl(
                                                    param,
                                                    &pos,
                                                    InstrSeq::gather(vec![
                                                        emit_expression::get_type_structure_for_hint(
                                                            emitter,
                                                            tparams
                                                                .iter()
                                                                .map(|fp| fp.name.1.as_str())
                                                                .collect::<Vec<_>>()
                                                                .as_slice(),
                                                            &IndexSet::new(),
                                                            hint,
                                                        )?,
                                                        instr::combine_and_resolve_type_struct(1),
                                                        instr::basec(0, MemberOpMode::ModeNone),
                                                        instr::querym(1, QueryOp::CGetQuiet, MemberKey::ET("classname".into(), ReadOnlyOp::Any)),
                                                        instr::dup(),
                                                        instr::istypec(IstypeOp::OpNull),
                                                        instr::jmpnz(label_not_a_class.clone()),
                                                    ]),
                                                    "Type constant does not resolve to a class",
                                                    label_not_a_class,
                                                    label_done,
                                            )?))
                                            }
                                        }
                                        _ => Err(unrecoverable(
                                            "Unexpected case for HH\\MemberOf enum type",
                                        )),
                                    }
                                }
                                _ => {
                                    Err(unrecoverable("Unexpected case for HH\\MemberOf enum type"))
                                }
                            }
                        }
                        _ => Err(unrecoverable(
                            "Wrong number of type arguments to HH\\MemberOf",
                        )),
                    }
                }
                _ => Err(raise_fatal_parse(
                    &ast_param.pos,
                    "'__Atom' applied to a non-HH\\MemberOf parameter",
                )),
            }
        }
    }
}

pub fn emit_method_prolog(
    emitter: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    params: &[HhasParam],
    ast_params: &[tast::FunParam],
    tparams: &[tast::Tparam],
) -> Result {
    let mut make_param_instr =
        |(param, ast_param): (&HhasParam, &tast::FunParam)| -> Result<Option<InstrSeq>> {
            let param_name = &param.name;
            let param_name = || ParamId::ParamNamed(param_name.into());
            if param.is_variadic {
                Ok(None)
            } else {
                use RGH::ReificationLevel as L;
                let param_checks =
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
                    }?;

                let atom_instrs = if emitter
                    .options()
                    .hhvm
                    .hack_lang
                    .flags
                    .contains(LangFlags::ENABLE_ENUM_CLASSES)
                {
                    atom_instrs(emitter, env, param, ast_param, tparams)?
                } else {
                    None
                };

                match (param_checks, atom_instrs) {
                    (None, None) => Ok(None),
                    (Some(is), None) => Ok(Some(is)),
                    (Some(is), Some(js)) => Ok(Some(InstrSeq::gather(vec![is, js]))),
                    (None, Some(js)) => Ok(Some(js)),
                }
            }
        };

    let ast_params = ast_params
        .iter()
        .filter(|p| !(p.is_variadic && p.name == "..."))
        .collect::<Vec<_>>();
    if params.len() != ast_params.len() {
        return Err(Error::Unrecoverable("length mismatch".into()));
    }

    let param_instrs = params
        .iter()
        .zip(ast_params.into_iter())
        .filter_map(|p| make_param_instr(p).transpose())
        .collect::<Result<Vec<_>>>()?;

    let mut instrs = vec![emit_pos(pos)];
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
                    Some(c) if c.get_kind() == tast::ClassKind::Ctrait => (
                        "::".into(),
                        InstrSeq::gather(vec![instr::self_(), instr::classname()]),
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
                    return Err(Error::Unrecoverable(
                        "deprecated functions must have names".into(),
                    ));
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
                    instr::nulluninit(),
                    instr::nulluninit(),
                    trait_instrs,
                    instr::string(deprecation_string),
                    concat_instruction,
                    instr::int64(sampling_rate),
                    instr::int(error_code),
                    instr::fcallfuncd(
                        FcallArgs::new(FcallFlags::default(), 1, vec![], None, 3, None),
                        function::from_raw_string("trigger_sampled_error"),
                    ),
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
    immediate_tparams: &[tast::Tparam],
    class_tparam_names: &[&str],
    skip_awaitable: bool,
) -> Vec<(String, Vec<HhasTypeInfo>)> {
    let constraint_filter = |(kind, hint): &(ast_defs::ConstraintKind, tast::Hint)| {
        if let ast_defs::ConstraintKind::ConstraintAs = &kind {
            let mut tparam_names = get_tp_names(immediate_tparams);
            tparam_names.extend_from_slice(class_tparam_names);
            emit_type_hint::hint_to_type_info(
                &emit_type_hint::Kind::UpperBound,
                skip_awaitable,
                false, // nullable
                &tparam_names,
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
    immediate_tparams
        .iter()
        .filter_map(tparam_filter)
        .collect::<Vec<_>>()
}

fn emit_shadowed_tparams(
    immediate_tparams: &[tast::Tparam],
    class_tparam_names: &[&str],
) -> Vec<String> {
    let s1 = get_tp_names_set(immediate_tparams);
    let s2: HashSet<&str> = class_tparam_names.into_iter().cloned().collect();
    // TODO(hrust): remove sort after Rust emitter released
    let mut r = s1
        .intersection(&s2)
        .map(|s| (*s).into())
        .collect::<Vec<_>>();
    r.sort();
    r
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

pub fn get_tp_names_set(tparams: &[tast::Tparam]) -> HashSet<&str> {
    tparams.iter().map(get_tp_name).collect()
}

fn modify_prog_for_debugger_eval(_body_instrs: &mut InstrSeq) {
    unimplemented!()
}

fn set_function_jmp_targets(emitter: &mut Emitter, env: &mut Env) -> bool {
    use ScopeItem::*;
    let function_state_key = match env.scope.items.as_slice() {
        [] => get_unique_id_for_main(),
        [.., Class(cls), Method(md)] | [.., Class(cls), Method(md), Lambda(_)] => {
            get_unique_id_for_method(cls.get_name_str(), md.get_name_str())
        }
        [.., Function(fun)] => get_unique_id_for_function(fun.get_name_str()),
        _ => panic!("unexpected scope shape"),
    };
    let global_state = emitter.emit_global_state();
    match global_state.function_to_labels_map.get(&function_state_key) {
        Some(labels) => {
            env.jump_targets_gen.set_labels_in_function(labels.clone());
        }
        None => {
            env.jump_targets_gen
                .set_labels_in_function(<SMap<bool>>::new());
        }
    };
    global_state
        .functions_with_finally
        .contains(&function_state_key)
}
