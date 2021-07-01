// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod emit_statement;
mod reified_generics_helpers;
mod try_finally_rewriter;

use emit_statement::emit_final_stmts;
use reified_generics_helpers as RGH;

use aast::TypeHint;
use aast_defs::{Hint, Hint_::*};
use decl_provider::DeclProvider;
use hash::HashSet;
use hhbc_by_ref_ast_body::AstBody;
use hhbc_by_ref_ast_class_expr::ClassExpr;
use hhbc_by_ref_ast_scope::{Scope, ScopeItem};
use hhbc_by_ref_decl_vars as decl_vars;
use hhbc_by_ref_emit_adata as emit_adata;
use hhbc_by_ref_emit_expression as emit_expression;
use hhbc_by_ref_emit_fatal::{emit_fatal_runtime, raise_fatal_parse};
use hhbc_by_ref_emit_param as emit_param;
use hhbc_by_ref_emit_pos::emit_pos;
use hhbc_by_ref_emit_type_hint as emit_type_hint;
use hhbc_by_ref_env::{emitter::Emitter, Env};
use hhbc_by_ref_generator as generator;
use hhbc_by_ref_hhas_body::{HhasBody, HhasBodyEnv};
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_hhas_type::Info as HhasTypeInfo;
use hhbc_by_ref_hhbc_ast::{
    ClassKind, FcallArgs, FcallFlags, Instruct, IstypeOp, MemberKey, MemberOpMode, ParamId,
    QueryOp, ReadOnlyOp,
};
use hhbc_by_ref_hhbc_id::function;
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::{instr, unrecoverable, Error, InstrSeq, Result};
use hhbc_by_ref_label::Label;
use hhbc_by_ref_label_rewriter as label_rewriter;
use hhbc_by_ref_options::{CompilerFlags, LangFlags};
use hhbc_by_ref_runtime::TypedValue;
use hhbc_by_ref_statement_state::StatementState;
use hhbc_by_ref_unique_id_builder::*;

use naming_special_names_rust::user_attributes as ua;

use ocamlrep::rc::RcOc;
use oxidized::{
    aast, aast_defs, ast as tast, ast_defs, doc_comment::DocComment, namespace_env, pos::Pos,
};

use ffi::Slice;

use bitflags::bitflags;
use indexmap::IndexSet;
use itertools::Either;

static THIS: &'static str = "$this";

/// Optional arguments for emit_body; use Args::default() for defaults
pub struct Args<'a, 'arena> {
    pub immediate_tparams: &'a Vec<tast::Tparam>,
    pub class_tparam_names: &'a [&'a str],
    pub ast_params: &'a Vec<tast::FunParam>,
    pub ret: Option<&'a tast::Hint>,
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

pub fn emit_body_with_default_args<'b, 'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl, D>,
    namespace: RcOc<namespace_env::Env>,
    body: &'b tast::Program,
    return_value: InstrSeq<'arena>,
) -> Result<HhasBody<'arena>> {
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
        alloc,
        emitter,
        namespace,
        Either::Left(body),
        return_value,
        Scope::toplevel(),
        args,
    )
    .map(|r| r.0)
}

pub fn emit_body<'b, 'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl, D>,
    namespace: RcOc<namespace_env::Env>,
    body: AstBody<'b>,
    return_value: InstrSeq<'arena>,
    scope: Scope<'_>,
    args: Args<'_, 'arena>,
) -> Result<(HhasBody<'arena>, bool, bool)> {
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
        alloc,
        args.flags.contains(Flags::SKIP_AWAITABLE),
        args.flags.contains(Flags::NATIVE),
        args.ret,
        &tp_names,
    )?;

    let params = make_params(
        alloc,
        emitter,
        &mut tp_names,
        args.ast_params,
        &scope,
        args.flags,
    )?;

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
    let num_closures = match emitter
        .emit_global_state()
        .num_closures
        .get(&get_unique_id_for_scope(&env.scope))
    {
        Some(num) => *num,
        None => 0,
    };
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
            alloc,
            emitter,
            body_instrs,
            decl_vars,
            false, // is_memoize_wrapper
            false, // is_memoize_wrapper_lsb
            num_closures,
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

fn make_body_instrs<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    emitter: &mut Emitter<'arena, 'decl, D>,
    env: &mut Env<'a, 'arena>,
    params: &[HhasParam<'arena>],
    tparams: &[tast::Tparam],
    decl_vars: &[String],
    body: AstBody,
    is_generator: bool,
    deprecation_info: Option<&[TypedValue<'arena>]>,
    pos: &Pos,
    ast_params: &[tast::FunParam],
    flags: Flags,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let stmt_instrs = if flags.contains(Flags::NATIVE) {
        instr::nativeimpl(alloc)
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
        InstrSeq::gather(alloc, vec![begin_label, instr::entrynop(alloc)])
    } else {
        InstrSeq::gather(alloc, vec![begin_label, header_content])
    };

    let mut body_instrs = InstrSeq::gather(alloc, vec![header, stmt_instrs, default_value_setters]);
    if flags.contains(Flags::DEBUGGER_MODIFY_PROGRAM) {
        modify_prog_for_debugger_eval(&mut body_instrs);
    };
    Ok(body_instrs)
}

fn make_header_content<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    emitter: &mut Emitter<'arena, 'decl, D>,
    env: &mut Env<'a, 'arena>,
    params: &[HhasParam<'arena>],
    tparams: &[tast::Tparam],
    _decl_vars: &[String],
    is_generator: bool,
    deprecation_info: Option<&[TypedValue<'arena>]>,
    pos: &Pos,
    ast_params: &[tast::FunParam],
    flags: Flags,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let method_prolog = if flags.contains(Flags::NATIVE) {
        instr::empty(alloc)
    } else {
        emit_method_prolog(emitter, env, pos, params, ast_params, tparams)?
    };

    let deprecation_warning =
        emit_deprecation_info(alloc, &env.scope, deprecation_info, emitter.systemlib())?;

    let generator_info = if is_generator {
        InstrSeq::gather(alloc, vec![instr::createcont(alloc), instr::popc(alloc)])
    } else {
        instr::empty(alloc)
    };

    Ok(InstrSeq::gather(
        alloc,
        vec![method_prolog, deprecation_warning, generator_info],
    ))
}

fn make_decl_vars<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    emitter: &mut Emitter<'arena, 'decl, D>,
    scope: &Scope<'a>,
    immediate_tparams: &[tast::Tparam],
    params: &[HhasParam<'arena>],
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

pub fn emit_return_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    tp_names: &[&str],
    skip_awaitable: bool,
    ret: Option<&aast::Hint>,
) -> Result<HhasTypeInfo> {
    match ret {
        None => Ok(HhasTypeInfo::make(
            Some("".to_string()),
            hhbc_by_ref_hhas_type::constraint::Type::default(),
        )),
        Some(hint) => emit_type_hint::hint_to_type_info(
            alloc,
            &emit_type_hint::Kind::Return,
            skip_awaitable,
            false, // nullable
            tp_names,
            &hint,
        ),
    }
}

fn make_return_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    skip_awaitable: bool,
    is_native: bool,
    ret: Option<&aast::Hint>,
    tp_names: &[&str],
) -> Result<HhasTypeInfo> {
    let return_type_info = emit_return_type_info(alloc, tp_names, skip_awaitable, ret);
    if is_native {
        return return_type_info.map(|rti| {
            emit_type_hint::emit_type_constraint_for_native_function(tp_names, ret, rti)
        });
    };
    return_type_info
}

pub fn make_env<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    namespace: RcOc<namespace_env::Env>,
    scope: Scope<'a>,
    call_context: Option<String>,
) -> Env<'a, 'arena> {
    let mut env = Env::default(alloc, namespace);
    env.call_context = call_context;
    env.scope = scope;
    env
}

fn make_params<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl, D>,
    tp_names: &mut Vec<&str>,
    ast_params: &[tast::FunParam],
    scope: &Scope<'a>,
    flags: Flags,
) -> Result<Vec<HhasParam<'arena>>> {
    let generate_defaults = !flags.contains(Flags::MEMOIZE);
    emit_param::from_asts(
        alloc,
        emitter,
        tp_names,
        generate_defaults,
        scope,
        ast_params,
    )
}

pub fn make_body<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl, D>,
    mut body_instrs: InstrSeq<'arena>,
    decl_vars: Vec<String>,
    is_memoize_wrapper: bool,
    is_memoize_wrapper_lsb: bool,
    num_closures: u32,
    upper_bounds: Vec<(String, Vec<HhasTypeInfo>)>,
    shadowed_tparams: Vec<String>,
    mut params: Vec<HhasParam<'arena>>,
    return_type_info: Option<HhasTypeInfo>,
    doc_comment: Option<DocComment>,
    opt_env: Option<&Env<'a, 'arena>>,
) -> Result<HhasBody<'arena>> {
    emit_adata::rewrite_typed_values(alloc, emitter, &mut body_instrs)?;
    if emitter
        .options()
        .hack_compiler_flags
        .contains(CompilerFlags::RELABEL)
    {
        label_rewriter::relabel_function(alloc, &mut params, &mut body_instrs);
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
        num_closures,
        upper_bounds,
        shadowed_tparams,
        params,
        return_type_info,
        doc_comment,
        env: body_env,
    })
}

fn emit_ast_body<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    env: &mut Env<'a, 'arena>,
    e: &mut Emitter<'arena, 'decl, D>,
    body: &AstBody,
) -> Result<InstrSeq<'arena>> {
    match body {
        Either::Left(p) => emit_defs(env, e, p),
        Either::Right(b) => emit_final_stmts(e, env, b),
    }
}

fn emit_defs<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    env: &mut Env<'a, 'arena>,
    emitter: &mut Emitter<'arena, 'decl, D>,
    prog: &[tast::Def],
) -> Result<InstrSeq<'arena>> {
    use tast::Def;
    fn emit_def<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
        env: &mut Env<'a, 'arena>,
        emitter: &mut Emitter<'arena, 'decl, D>,
        def: &tast::Def,
    ) -> Result<InstrSeq<'arena>> {
        let alloc = env.arena;
        match def {
            Def::Stmt(s) => emit_statement::emit_stmt(emitter, env, s),
            Def::Namespace(ns) => emit_defs(env, emitter, &ns.1),
            _ => Ok(instr::empty(alloc)),
        }
    }
    fn aux<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
        env: &mut Env<'a, 'arena>,
        emitter: &mut Emitter<'arena, 'decl, D>,
        defs: &[tast::Def],
    ) -> Result<InstrSeq<'arena>> {
        let alloc = env.arena;
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
                _ => Ok(InstrSeq::gather(
                    alloc,
                    vec![
                        emit_statement::emit_stmt(emitter, env, s1)?,
                        emit_statement::emit_final_stmt(emitter, env, &s2)?,
                    ],
                )),
            },
            [def, ..] => Ok(InstrSeq::gather(
                alloc,
                vec![emit_def(env, emitter, def)?, aux(env, emitter, &defs[1..])?],
            )),
        }
    }
    let alloc = env.arena;
    match prog {
        [Def::Stmt(s), ..] if s.1.is_markup() => Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_statement::emit_markup(emitter, env, s.1.as_markup().unwrap(), true)?,
                aux(env, emitter, &prog[1..])?,
            ],
        )),
        [] | [_] => aux(env, emitter, &prog[..]),
        [def, ..] => {
            let i1 = emit_def(env, emitter, def)?;
            if i1.is_empty() {
                emit_defs(env, emitter, &prog[1..])
            } else {
                Ok(InstrSeq::gather(
                    alloc,
                    vec![i1, aux(env, emitter, &prog[1..])?],
                ))
            }
        }
    }
}

pub fn has_type_constraint<'a, 'arena>(
    env: &Env<'a, 'arena>,
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
    use hhbc_by_ref_instruction_sequence::{instr, unrecoverable, InstrSeq, Result};
    use hhbc_by_ref_local::Type::Named;
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

    pub fn emit_clscnsl<'arena>(
        alloc: &'arena bumpalo::Bump,
        param: &HhasParam<'arena>,
        pos: &Pos,
        cls_instrs: InstrSeq<'arena>,
        msg: &str,
        label_not_a_class: Label,
        label_done: Label,
    ) -> Result<InstrSeq<'arena>> {
        let param_name = &param.name;
        let loc =
            Named(bumpalo::collections::String::from_str_in(param_name, alloc).into_bump_str());
        Ok(InstrSeq::gather(
            alloc,
            vec![
                cls_instrs,
                instr::classgetc(alloc),
                instr::clscnsl(alloc, loc.clone()),
                instr::popl(alloc, loc),
                instr::jmp(alloc, label_done.clone()),
                instr::label(alloc, label_not_a_class),
                emit_fatal_runtime(alloc, &pos, msg),
                instr::label(alloc, label_done),
            ],
        ))
    }
} //mod atom_helpers

////////////////////////////////////////////////////////////////////////////////
// atom_instrs

fn atom_instrs<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    emitter: &mut Emitter<'arena, 'decl, D>,
    env: &mut Env<'a, 'arena>,
    param: &HhasParam<'arena>,
    ast_param: &tast::FunParam,
    tparams: &[tast::Tparam],
) -> Result<Option<InstrSeq<'arena>>> {
    let alloc = env.arena;
    if !param
        .user_attributes
        .iter()
        .any(|a| a.is(|x| x == ua::VIA_LABEL))
    {
        return Ok(None); // Not an atom. Nothing to do.
    }
    match &ast_param.type_hint {
        TypeHint(_, None) => Err(raise_fatal_parse(
            &ast_param.pos,
            ua::VIA_LABEL.to_owned() + " param type hint unavailable",
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
                                                alloc,
                                                param,
                                                &pos,
                                                InstrSeq::gather(
                                                    alloc,
                                                    vec![
                                                        emit_expression::emit_expr(
                                                            emitter,
                                                            env,
                                                            &(tast::Expr(
                                                                Pos::make_none(),
                                                                aast::Expr_::String(
                                                                    bstr::BString::from(
                                                                        tag.to_owned(),
                                                                    ),
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
                                                        instr::oodeclexists(
                                                            alloc,
                                                            ClassKind::Class,
                                                        ),
                                                        instr::jmpz(
                                                            alloc,
                                                            label_not_a_class.clone(),
                                                        ),
                                                        emit_expression::emit_expr(
                                                            emitter,
                                                            env,
                                                            &(tast::Expr(
                                                                Pos::make_none(),
                                                                aast::Expr_::String(
                                                                    bstr::BString::from(
                                                                        tag.to_owned(),
                                                                    ),
                                                                ),
                                                            )),
                                                        )?,
                                                    ],
                                                ),
                                                "Type is not a class",
                                                label_not_a_class,
                                                label_done,
                                            )?))
                                        } else {
                                            //'tag' is a reified generic.
                                            Ok(Some(atom_helpers::emit_clscnsl(
                                                alloc,
                                                param,
                                                &pos,
                                                InstrSeq::gather(
                                                    alloc,
                                                    vec![
                                                    emit_expression::emit_reified_generic_instrs(
                                                        alloc,
                                                        &Pos::make_none(),
                                                        true,
                                                        atom_helpers::index_of_generic(
                                                            tparams, tag,
                                                        )?,
                                                    )?,
                                                    instr::basec(alloc, 0, MemberOpMode::ModeNone),
                                                    instr::querym(
                                                        alloc,
                                                        1,
                                                        QueryOp::CGetQuiet,
                                                        MemberKey::ET(
                                                            "classname".into(),
                                                            ReadOnlyOp::Any,
                                                        ),
                                                    ),
                                                    instr::dup(alloc),
                                                    instr::istypec(alloc, IstypeOp::OpNull),
                                                    instr::jmpnz(alloc, label_not_a_class.clone()),
                                                ],
                                                ),
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
                                                    alloc,
                                                    param,
                                                    &pos,
                                                    InstrSeq::gather(alloc, vec![
                                                        emit_expression::get_type_structure_for_hint(
                                                            alloc,
                                                            emitter,
                                                            tparams
                                                                .iter()
                                                                .map(|fp| fp.name.1.as_str())
                                                                .collect::<Vec<_>>()
                                                                .as_slice(),
                                                            &IndexSet::new(),
                                                            hint,
                                                        )?,
                                                        instr::combine_and_resolve_type_struct(alloc, 1),
                                                        instr::basec(alloc, 0, MemberOpMode::ModeNone),
                                                        instr::querym(alloc, 1, QueryOp::CGetQuiet, MemberKey::ET("classname", ReadOnlyOp::Any)),
                                                        instr::dup(alloc),
                                                        instr::istypec(alloc, IstypeOp::OpNull),
                                                        instr::jmpnz(alloc, label_not_a_class.clone()),
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
                    "'".to_owned() + ua::VIA_LABEL + "' applied to a non-HH\\MemberOf parameter",
                )),
            }
        }
    }
}

pub fn emit_method_prolog<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    emitter: &mut Emitter<'arena, 'decl, D>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    params: &[HhasParam<'arena>],
    ast_params: &[tast::FunParam],
    tparams: &[tast::Tparam],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let mut make_param_instr =
        |(param, ast_param): (&HhasParam<'arena>, &tast::FunParam)| -> Result<Option<InstrSeq<'arena>>> {
            let param_name = &param.name;
            let param_name = || ParamId::ParamNamed(Slice::new(bumpalo::collections::String::from_str_in(param_name, alloc).into_bump_str().as_bytes()));
            if param.is_variadic {
                Ok(None)
            } else {
                use RGH::ReificationLevel as L;
                let param_checks =
                    match has_type_constraint(env, param.type_info.as_ref(), ast_param) {
                        (L::Unconstrained, _) => Ok(None),
                        (L::Not, _) => Ok(Some(instr::verify_param_type(alloc, param_name()))),
                        (L::Maybe, Some(h)) => Ok(Some(InstrSeq::gather(alloc, vec![
                            emit_expression::get_type_structure_for_hint(
                                alloc,
                                emitter,
                                tparams
                                    .iter()
                                    .map(|fp| fp.name.1.as_str())
                                    .collect::<Vec<_>>()
                                    .as_slice(),
                                &IndexSet::new(),
                                &h,
                            )?,
                            instr::verify_param_type_ts(alloc, param_name()),
                        ]))),
                        (L::Definitely, Some(h)) => {
                            let check = instr::istypel(
                                alloc,
                                hhbc_by_ref_local::Type::Named(bumpalo::collections::String::from_str_in(param.name.as_str(), alloc).into_bump_str()),
                                IstypeOp::OpNull,
                            );
                            let verify_instr = instr::verify_param_type_ts(alloc, param_name());
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
                    (Some(is), Some(js)) => Ok(Some(InstrSeq::gather(alloc, vec![is, js]))),
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
    let mut instrs = vec![emit_pos(alloc, pos)];
    for i in param_instrs.iter() {
        instrs.push(InstrSeq::clone(alloc, i));
    }
    Ok(InstrSeq::gather(alloc, instrs))
}

pub fn emit_deprecation_info<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    scope: &Scope<'a>,
    deprecation_info: Option<&[TypedValue<'arena>]>,
    is_systemlib: bool,
) -> Result<InstrSeq<'arena>> {
    Ok(match deprecation_info {
        None => instr::empty(alloc),
        Some(args) => {
            fn strip_id<'a>(id: &'a tast::Id) -> &'a str {
                string_utils::strip_global_ns(id.1.as_str())
            }
            let (class_name, trait_instrs, concat_instruction): (String, _, _) =
                match scope.get_class() {
                    None => ("".into(), instr::empty(alloc), instr::empty(alloc)),
                    Some(c) if c.get_kind() == tast::ClassKind::Ctrait => (
                        "::".into(),
                        InstrSeq::gather(alloc, vec![instr::self_(alloc), instr::classname(alloc)]),
                        instr::concat(alloc),
                    ),
                    Some(c) => (
                        strip_id(c.get_name()).to_string() + "::",
                        instr::empty(alloc),
                        instr::empty(alloc),
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
                    s
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
                instr::empty(alloc)
            } else {
                InstrSeq::gather(
                    alloc,
                    vec![
                        instr::nulluninit(alloc),
                        instr::nulluninit(alloc),
                        trait_instrs,
                        instr::string(alloc, deprecation_string),
                        concat_instruction,
                        instr::int64(alloc, sampling_rate),
                        instr::int(alloc, error_code),
                        instr::fcallfuncd(
                            alloc,
                            FcallArgs::new(
                                FcallFlags::default(),
                                1,
                                Slice::new(bumpalo::vec![in alloc;].into_bump_slice()),
                                None,
                                3,
                                None,
                            ),
                            function::from_raw_string(alloc, "trigger_sampled_error"),
                        ),
                        instr::popc(alloc),
                    ],
                )
            }
        }
    })
}

fn set_emit_statement_state<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl, D>,
    default_return_value: InstrSeq<'arena>,
    params: &[HhasParam<'arena>],
    return_type_info: &HhasTypeInfo,
    return_type: Option<&tast::Hint>,
    pos: &Pos,
    default_dropthrough: Option<InstrSeq<'arena>>,
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
        Some(InstrSeq::gather(
            alloc,
            vec![
                instr::null(alloc),
                instr::verify_ret_type_c(alloc),
                instr::retc(alloc),
            ],
        ))
    } else {
        None
    };
    let (num_out, verify_out) = emit_verify_out(alloc, params);

    emit_statement::set_state(
        alloc,
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
    params: &[HhasParam<'arena>],
) -> (usize, InstrSeq<'arena>) {
    let param_instrs: Vec<InstrSeq<'arena>> = params
        .iter()
        .enumerate()
        .filter_map(|(i, p)| {
            if p.is_inout {
                Some(InstrSeq::gather(
                    alloc,
                    vec![
                        instr::cgetl(
                            alloc,
                            hhbc_by_ref_local::Type::Named(
                                bumpalo::collections::String::from_str_in(p.name.as_str(), alloc)
                                    .into_bump_str(),
                            ),
                        ),
                        match p.type_info.as_ref() {
                            Some(HhasTypeInfo { user_type, .. })
                                if user_type.as_ref().map_or(true, |t| {
                                    !(t.ends_with("HH\\mixed") || t.ends_with("HH\\dynamic"))
                                }) =>
                            {
                                instr::verify_out_type(alloc, ParamId::ParamUnnamed(i as isize))
                            }
                            _ => instr::empty(alloc),
                        },
                    ],
                ))
            } else {
                None
            }
        })
        .collect();
    (
        param_instrs.len(),
        InstrSeq::gather(alloc, param_instrs.into_iter().rev().collect()),
    )
}

pub fn emit_generics_upper_bounds<'arena>(
    alloc: &'arena bumpalo::Bump,
    immediate_tparams: &[tast::Tparam],
    class_tparam_names: &[&str],
    skip_awaitable: bool,
) -> Vec<(String, Vec<HhasTypeInfo>)> {
    let constraint_filter = |(kind, hint): &(ast_defs::ConstraintKind, tast::Hint)| {
        if let ast_defs::ConstraintKind::ConstraintAs = &kind {
            let mut tparam_names = get_tp_names(immediate_tparams);
            tparam_names.extend_from_slice(class_tparam_names);
            emit_type_hint::hint_to_type_info(
                alloc,
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

#[allow(clippy::needless_lifetimes)]
fn modify_prog_for_debugger_eval<'arena>(_body_instrs: &mut InstrSeq<'arena>) {
    unimplemented!() // SF(2021-03-17): I found it like this.
}

fn set_function_jmp_targets<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    emitter: &mut Emitter<'arena, 'decl, D>,
    env: &mut Env<'a, 'arena>,
) -> bool {
    let function_state_key = get_unique_id_for_scope(&env.scope);
    emitter
        .emit_global_state()
        .functions_with_finally
        .contains(&function_state_key)
}
