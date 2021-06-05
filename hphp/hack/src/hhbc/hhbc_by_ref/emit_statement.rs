// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::try_finally_rewriter as tfr;

use hhbc_by_ref_emit_expression::{self as emit_expr, emit_await, emit_expr, LValOp, Setrange};
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_emit_pos::{emit_pos, emit_pos_then};
use hhbc_by_ref_env::{emitter::Emitter, Env};
use hhbc_by_ref_hhbc_ast::*;
use hhbc_by_ref_hhbc_id::{self as hhbc_id, Id};
use hhbc_by_ref_instruction_sequence::{instr, Error::Unrecoverable, InstrSeq, Result};
use hhbc_by_ref_label::Label;
use hhbc_by_ref_label_rewriter as label_rewriter;
use hhbc_by_ref_local as local;
use hhbc_by_ref_scope::scope;
use hhbc_by_ref_statement_state::StatementState;

use lazy_static::lazy_static;
use naming_special_names_rust::{special_functions, special_idents, superglobals};
use oxidized::{aast as a, ast as tast, ast_defs, local_id, pos::Pos};
use regex::Regex;

// Expose a mutable ref to state for emit_body so that it can set it appropriately
pub(crate) fn set_state<'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    state: StatementState<'arena>,
) {
    *e.emit_statement_state_mut(alloc) = state;
}

pub(crate) type Level = usize;

// Wrapper functions

fn emit_return<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
) -> Result<InstrSeq<'arena>> {
    tfr::emit_return(e, false, env)
}

fn set_bytes_kind(name: &str) -> Option<Setrange> {
    lazy_static! {
        static ref RE: Regex =
            Regex::new(r#"(?i)^hh\\set_bytes(_rev)?_([a-z0-9]+)(_vec)?$"#).unwrap();
    }
    RE.captures(name).map_or(None, |groups| {
        let op = if groups.get(1).is_some() {
            // == _rev
            SetrangeOp::Reverse
        } else {
            SetrangeOp::Forward
        };
        let kind = groups.get(2).unwrap().as_str();
        let vec = groups.get(3).is_some(); // == _vec
        if kind == "string" && !vec {
            Some(Setrange {
                size: 1,
                vec: true,
                op,
            })
        } else {
            let size = match kind {
                "bool" | "int8" => 1,
                "int16" => 2,
                "int32" | "float32" => 4,
                "int64" | "float64" => 8,
                _ => return None,
            };
            Some(Setrange { size, vec, op })
        }
    })
}

pub fn emit_stmt<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    stmt: &tast::Stmt,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let pos = &stmt.0;
    match &stmt.1 {
        a::Stmt_::YieldBreak => Ok(InstrSeq::gather(
            alloc,
            vec![instr::null(alloc), emit_return(e, env)?],
        )),
        a::Stmt_::Expr(e_) => match &e_.1 {
            a::Expr_::Await(a) => Ok(InstrSeq::gather(
                alloc,
                vec![emit_await(e, env, &e_.0, a)?, instr::popc(alloc)],
            )),
            a::Expr_::Call(c) => {
                if let (a::Expr(_, a::Expr_::Id(sid)), _, exprs, None) = c.as_ref() {
                    let ft = hhbc_id::function::Type::from_ast_name(alloc, &sid.1);
                    let fname = ft.to_raw_string();
                    if fname.eq_ignore_ascii_case("unset") {
                        Ok(InstrSeq::gather(
                            alloc,
                            exprs
                                .iter()
                                .map(|ex| emit_expr::emit_unset_expr(e, env, ex))
                                .collect::<std::result::Result<Vec<_>, _>>()?,
                        ))
                    } else {
                        if let Some(kind) = set_bytes_kind(fname) {
                            let exprs = exprs.iter().collect::<Vec<&tast::Expr>>();
                            match exprs.first() {
                                Some(a::Expr(_, a::Expr_::Callconv(cc))) if cc.0.is_pinout() => {
                                    let mut args = vec![&cc.1];
                                    args.extend_from_slice(&exprs[1..exprs.len()]);
                                    emit_expr::emit_set_range_expr(
                                        e,
                                        env,
                                        &e_.0,
                                        fname,
                                        kind,
                                        &args[..],
                                    )
                                }
                                _ => emit_expr::emit_set_range_expr(
                                    e,
                                    env,
                                    &e_.0,
                                    fname,
                                    kind,
                                    &exprs[..],
                                ),
                            }
                        } else {
                            emit_expr::emit_ignored_expr(e, env, &e_.0, e_)
                        }
                    }
                } else {
                    emit_expr::emit_ignored_expr(e, env, pos, e_)
                }
            }
            a::Expr_::Binop(bop) => {
                if let (ast_defs::Bop::Eq(None), e_lhs, e_rhs) = bop.as_ref() {
                    if let Some(e_await) = e_rhs.1.as_await() {
                        let await_pos = &e_rhs.0;
                        if let Some(l) = e_lhs.1.as_list() {
                            let awaited_instrs = emit_await(e, env, await_pos, e_await)?;
                            let has_elements = l.iter().any(|e| !e.1.is_omitted());
                            if has_elements {
                                scope::with_unnamed_local(alloc, e, |alloc, e, temp| {
                                    Ok((
                                        InstrSeq::gather(
                                            alloc,
                                            vec![awaited_instrs, instr::popl(alloc, temp)],
                                        ),
                                        (
                                            alloc,
                                            emit_expr::emit_lval_op_list(
                                                e,
                                                env,
                                                pos,
                                                Some(&temp),
                                                &[],
                                                e_lhs,
                                                false,
                                            )?,
                                        )
                                            .into(),
                                        instr::unsetl(alloc, temp),
                                    ))
                                })
                            } else {
                                Ok(InstrSeq::gather(
                                    alloc,
                                    vec![awaited_instrs, instr::popc(alloc)],
                                ))
                            }
                        } else {
                            emit_await_assignment(e, env, await_pos, e_lhs, e_await)
                        }
                    } else {
                        emit_expr::emit_ignored_expr(e, env, pos, e_)
                    }
                } else {
                    emit_expr::emit_ignored_expr(e, env, pos, e_)
                }
            }
            _ => emit_expr::emit_ignored_expr(e, env, pos, e_),
        },
        a::Stmt_::Return(r_opt) => match r_opt.as_ref() {
            Some(r) => {
                let ret = emit_return(e, env)?;
                let expr_instr = if let Some(e_) = r.1.as_await() {
                    emit_await(e, env, &r.0, e_)?
                } else {
                    emit_expr(e, env, &r)?
                };
                Ok(InstrSeq::gather(
                    alloc,
                    vec![expr_instr, emit_pos(alloc, pos), ret],
                ))
            }
            None => Ok(InstrSeq::gather(
                alloc,
                vec![
                    instr::null(alloc),
                    emit_pos(alloc, pos),
                    emit_return(e, env)?,
                ],
            )),
        },
        a::Stmt_::Block(b) => emit_block(env, e, &b),
        a::Stmt_::If(f) => emit_if(e, env, pos, &f.0, &f.1, &f.2),
        a::Stmt_::While(x) => emit_while(e, env, &x.0, &x.1),
        a::Stmt_::Using(x) => emit_using(e, env, &**x),
        a::Stmt_::Break => Ok(emit_break(e, env, pos)),
        a::Stmt_::Continue => Ok(emit_continue(e, env, pos)),
        a::Stmt_::Do(x) => emit_do(e, env, &x.0, &x.1),
        a::Stmt_::For(x) => emit_for(e, env, &x.0, &x.1, &x.2, &x.3),
        a::Stmt_::Throw(x) => Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_expr::emit_expr(e, env, x)?,
                emit_pos(alloc, pos),
                instr::throw(alloc),
            ],
        )),
        a::Stmt_::Try(x) => {
            let (try_block, catch_list, finally_block) = &**x;
            if catch_list.is_empty() {
                emit_try_finally(e, env, pos, &try_block, &finally_block)
            } else if finally_block.is_empty() {
                emit_try_catch(e, env, pos, &try_block, &catch_list[..])
            } else {
                emit_try_catch_finally(e, env, pos, &try_block, &catch_list[..], &finally_block)
            }
        }
        a::Stmt_::Switch(x) => emit_switch(e, env, pos, &x.0, &x.1),
        a::Stmt_::Foreach(x) => emit_foreach(e, env, pos, &x.0, &x.1, &x.2),
        a::Stmt_::Awaitall(x) => emit_awaitall(e, env, pos, &x.0, &x.1),
        a::Stmt_::Markup(x) => emit_markup(e, env, &x, false),
        a::Stmt_::Fallthrough | a::Stmt_::Noop => Ok(instr::empty(alloc)),
        a::Stmt_::AssertEnv(_) => Ok(instr::empty(alloc)),
    }
}

fn emit_case<'c, 'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    case: &'c tast::Case,
) -> Result<(
    InstrSeq<'arena>,
    (Option<(&'c tast::Expr, Label)>, Option<Label>),
)> {
    let alloc = env.arena;
    let l = e.label_gen_mut().next_regular();
    Ok(match case {
        tast::Case::Case(case_expr, b) => (
            InstrSeq::gather(alloc, vec![instr::label(alloc, l), emit_block(env, e, b)?]),
            (Some((case_expr, l)), None),
        ),
        tast::Case::Default(_, b) => (
            InstrSeq::gather(alloc, vec![instr::label(alloc, l), emit_block(env, e, b)?]),
            (None, Some(l)),
        ),
    })
}

fn emit_check_case<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    scrutinee_expr: &tast::Expr,
    (case_expr, case_handler_label): (&tast::Expr, Label),
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    Ok(if scrutinee_expr.1.is_lvar() {
        InstrSeq::gather(
            alloc,
            vec![
                emit_expr::emit_two_exprs(e, env, &case_expr.0, scrutinee_expr, &case_expr)?,
                instr::eq(alloc),
                instr::jmpnz(alloc, case_handler_label),
            ],
        )
    } else {
        let next_case_label = e.label_gen_mut().next_regular();
        InstrSeq::gather(
            alloc,
            vec![
                instr::dup(alloc),
                emit_expr::emit_expr(e, env, &case_expr)?,
                emit_pos(alloc, &case_expr.0),
                instr::eq(alloc),
                instr::jmpz(alloc, next_case_label),
                instr::popc(alloc),
                instr::jmp(alloc, case_handler_label),
                instr::label(alloc, next_case_label),
            ],
        )
    })
}

fn emit_awaitall<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    el: &[(Option<tast::Lid>, tast::Expr)],
    block: &tast::Block,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match el {
        [] => Ok(instr::empty(alloc)),
        [(lvar, expr)] => emit_awaitall_single(e, env, pos, lvar, expr, block),
        _ => emit_awaitall_multi(e, env, pos, el, block),
    }
}

fn emit_awaitall_single<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    lval: &Option<tast::Lid>,
    expr: &tast::Expr,
    block: &tast::Block,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    scope::with_unnamed_locals(alloc, e, |alloc, e| {
        let load_arg = emit_expr::emit_await(e, env, pos, expr)?;
        let (load, unset) = match lval {
            None => (instr::popc(alloc), instr::empty(alloc)),
            Some(tast::Lid(_, id)) => {
                let l = e
                    .local_gen_mut()
                    .init_unnamed_for_tempname(local_id::get_name(&id));
                (instr::popl(alloc, *l), instr::unsetl(alloc, *l))
            }
        };
        Ok((
            InstrSeq::gather(alloc, vec![load_arg, load]),
            emit_stmts(e, env, block)?,
            unset,
        ))
    })
}

fn emit_awaitall_multi<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    el: &[(Option<tast::Lid>, tast::Expr)],
    block: &tast::Block,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    scope::with_unnamed_locals(alloc, e, |alloc, e| {
        let load_args = InstrSeq::gather(
            alloc,
            el.iter()
                .map(|(_, expr)| emit_expr::emit_expr(e, env, expr))
                .collect::<Result<Vec<_>>>()?,
        );
        let locals: Vec<local::Type> = el
            .iter()
            .map(|(lvar, _)| match lvar {
                None => e.local_gen_mut().get_unnamed(),
                Some(tast::Lid(_, id)) => e
                    .local_gen_mut()
                    .init_unnamed_for_tempname(local_id::get_name(&id))
                    .to_owned(),
            })
            .collect();
        let init_locals = InstrSeq::gather(
            alloc,
            locals
                .iter()
                .rev()
                .map(|l| instr::popl(alloc, *l))
                .collect(),
        );
        let unset_locals = InstrSeq::gather(
            alloc,
            locals.iter().map(|l| instr::unsetl(alloc, *l)).collect(),
        );
        let unpack = InstrSeq::gather(
            alloc,
            locals
                .iter()
                .map(|l| {
                    let label_done = e.label_gen_mut().next_regular();
                    InstrSeq::gather(
                        alloc,
                        vec![
                            instr::pushl(alloc, *l),
                            instr::dup(alloc),
                            instr::istypec(alloc, IstypeOp::OpNull),
                            instr::jmpnz(alloc, label_done),
                            instr::whresult(alloc),
                            instr::label(alloc, label_done),
                            instr::popl(alloc, *l),
                        ],
                    )
                })
                .collect(),
        );
        let await_all = InstrSeq::gather(
            alloc,
            vec![instr::awaitall_list(alloc, locals), instr::popc(alloc)],
        );
        let block_instrs = emit_stmts(e, env, block)?;
        Ok((
            // before
            InstrSeq::gather(alloc, vec![load_args, init_locals]),
            // inner
            InstrSeq::gather(
                alloc,
                vec![emit_pos(alloc, pos), await_all, unpack, block_instrs],
            ),
            // after
            unset_locals,
        ))
    })
}

fn emit_using<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    using: &tast::UsingStmt,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let block_pos = block_pos(&using.block)?;
    if using.exprs.1.len() > 1 {
        emit_stmts(
            e,
            env,
            using
                .exprs
                .1
                .iter()
                .rev()
                .fold(using.block.clone(), |block, expr| {
                    vec![tast::Stmt(
                        expr.0.clone(),
                        tast::Stmt_::mk_using(tast::UsingStmt {
                            has_await: using.has_await,
                            is_block_scoped: using.is_block_scoped,
                            exprs: (expr.0.clone(), vec![expr.clone()]),
                            block,
                        }),
                    )]
                })
                .as_slice(),
        )
    } else {
        e.local_scope(|e| {
            let (local, preamble) = match &(using.exprs.1[0].1) {
                tast::Expr_::Binop(x) => match (&x.0, (x.1).1.as_lvar()) {
                    (ast_defs::Bop::Eq(None), Some(tast::Lid(_, id))) => (
                        local::Type::Named(
                            bumpalo::collections::String::from_str_in(
                                local_id::get_name(&id).as_str(),
                                alloc,
                            )
                            .into_bump_str(),
                        ),
                        InstrSeq::gather(
                            alloc,
                            vec![
                                emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                                emit_pos(alloc, &block_pos),
                                instr::popc(alloc),
                            ],
                        ),
                    ),
                    _ => {
                        let l = e.local_gen_mut().get_unnamed();
                        (
                            l,
                            InstrSeq::gather(
                                alloc,
                                vec![
                                    emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                                    instr::setl(alloc, l),
                                    instr::popc(alloc),
                                ],
                            ),
                        )
                    }
                },
                tast::Expr_::Lvar(lid) => (
                    local::Type::Named(
                        bumpalo::collections::String::from_str_in(
                            local_id::get_name(&lid.1).as_str(),
                            alloc,
                        )
                        .into_bump_str(),
                    ),
                    InstrSeq::gather(
                        alloc,
                        vec![
                            emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                            emit_pos(alloc, &block_pos),
                            instr::popc(alloc),
                        ],
                    ),
                ),
                _ => {
                    let l = e.local_gen_mut().get_unnamed();
                    (
                        l,
                        InstrSeq::gather(
                            alloc,
                            vec![
                                emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                                instr::setl(alloc, l),
                                instr::popc(alloc),
                            ],
                        ),
                    )
                }
            };
            let finally_start = e.label_gen_mut().next_regular();
            let finally_end = e.label_gen_mut().next_regular();
            let body = env.do_in_using_body(e, finally_start, &using.block, emit_block)?;
            let jump_instrs = tfr::JumpInstructions::collect(&body, &mut env.jump_targets_gen);
            let jump_instrs_is_empty = jump_instrs.is_empty();
            let finally_epilogue =
                tfr::emit_finally_epilogue(e, env, &using.exprs.1[0].0, jump_instrs, finally_end)?;
            let try_instrs = if jump_instrs_is_empty {
                body
            } else {
                tfr::cleanup_try_body(alloc, &body)
            };

            let emit_finally = |
                e: &mut Emitter<'arena>,
                local: local::Type<'arena>,
                has_await: bool,
                is_block_scoped: bool,
            | -> InstrSeq<'arena> {
                let (epilogue, async_eager_label) = if has_await {
                    let after_await = e.label_gen_mut().next_regular();
                    (
                        InstrSeq::gather(
                            alloc,
                            vec![
                                instr::await_(alloc),
                                instr::label(alloc, after_await),
                                instr::popc(alloc),
                            ],
                        ),
                        Some(after_await),
                    )
                } else {
                    (instr::popc(alloc), None)
                };
                let fn_name = hhbc_id::method::from_raw_string(
                    alloc,
                    if has_await {
                        "__disposeAsync"
                    } else {
                        "__dispose"
                    },
                );
                InstrSeq::gather(
                    alloc,
                    vec![
                        instr::cgetl(alloc, local),
                        instr::nulluninit(alloc),
                        instr::fcallobjmethodd(
                            alloc,
                            FcallArgs::new(
                                FcallFlags::empty(),
                                1,
                                bumpalo::vec![in alloc;].into_bump_slice(),
                                async_eager_label,
                                0,
                                env.call_context
                                    .as_ref()
                                    .map(|s| -> &str { alloc.alloc_str(s.as_ref()) }),
                            ),
                            fn_name,
                            ObjNullFlavor::NullThrows,
                        ),
                        epilogue,
                        if is_block_scoped {
                            instr::unsetl(alloc, local)
                        } else {
                            instr::empty(alloc)
                        },
                    ],
                )
            };
            let exn_local = e.local_gen_mut().get_unnamed();
            let middle = if is_empty_block(&using.block) {
                instr::empty(alloc)
            } else {
                let finally_instrs = emit_finally(e, local, using.has_await, using.is_block_scoped);
                let catch_instrs = InstrSeq::gather(
                    alloc,
                    vec![
                        emit_pos(alloc, &block_pos),
                        make_finally_catch(alloc, e, exn_local, finally_instrs),
                        emit_pos(alloc, &using.exprs.1[0].0),
                    ],
                );
                InstrSeq::create_try_catch(
                    alloc,
                    e.label_gen_mut(),
                    None,
                    true,
                    try_instrs,
                    catch_instrs,
                )
            };
            Ok(InstrSeq::gather(
                alloc,
                vec![
                    preamble,
                    middle,
                    instr::label(alloc, finally_start),
                    emit_finally(e, local, using.has_await, using.is_block_scoped),
                    finally_epilogue,
                    instr::label(alloc, finally_end),
                ],
            ))
        })
    }
}

fn block_pos(block: &tast::Block) -> Result<Pos> {
    if block.iter().all(|b| b.0.is_none()) {
        return Ok(Pos::make_none());
    }
    let mut first = 0;
    let mut last = block.len() - 1;
    loop {
        if !block[first].0.is_none() && !block[last].0.is_none() {
            return Pos::btw(&block[first].0, &block[last].0).map_err(|s| Unrecoverable(s));
        }
        if block[first].0.is_none() {
            first += 1;
        }
        if block[last].0.is_none() {
            last -= 1;
        }
    }
}

fn emit_cases<'a, 'arena>(
    env: &mut Env<'a, 'arena>,
    e: &mut Emitter<'arena>,
    pos: &Pos,
    break_label: Label,
    scrutinee_expr: &tast::Expr,
    cases: &[tast::Case],
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, Label)> {
    let alloc = env.arena;
    let has_default = cases.iter().any(|c| c.is_default());
    match cases.split_last() {
        None => {
            return Err(Unrecoverable(
                "impossible - switch statements must have at least one case".into(),
            ));
        }
        Some((last, rest)) => {
            // Emit all the cases except the last one
            let mut res = rest
                .iter()
                .map(|case| emit_case(e, env, case))
                .collect::<Result<Vec<_>>>()?;

            if has_default {
                // If there is a default, emit the last case as usual
                res.push(emit_case(e, env, last)?)
            } else {
                // Otherwise, emit the last case with an added break
                match last {
                    tast::Case::Case(expr, block) => {
                        let l = e.label_gen_mut().next_regular();
                        res.push((
                            InstrSeq::gather(
                                alloc,
                                vec![
                                    instr::label(alloc, l),
                                    emit_block(env, e, block)?,
                                    emit_break(e, env, &Pos::make_none()),
                                ],
                            ),
                            (Some((expr, l)), None),
                        ))
                    }
                    tast::Case::Default(_, _) => {
                        return Err(Unrecoverable(
                            "impossible - there shouldn't be a default".into(),
                        ));
                    }
                };
                // ...and emit warning/exception for missing default
                let l = e.label_gen_mut().next_regular();
                res.push((
                    InstrSeq::gather(
                        alloc,
                        vec![
                            instr::label(alloc, l),
                            emit_pos_then(alloc, pos, instr::throw_non_exhaustive_switch(alloc)),
                        ],
                    ),
                    (None, Some(l)),
                ))
            };
            let (case_body_instrs, case_exprs_and_default_labels): (Vec<InstrSeq<'arena>>, Vec<_>) =
                res.into_iter().unzip();
            let (case_exprs, default_labels): (Vec<Option<(&tast::Expr, Label)>>, Vec<_>) =
                case_exprs_and_default_labels.into_iter().unzip();

            let default_label = match default_labels
                .iter()
                .filter_map(|lopt| lopt.as_ref())
                .collect::<Vec<_>>()
                .as_slice()
            {
                [] => break_label,
                [l] => **l,
                _ => {
                    return Err(emit_fatal::raise_fatal_runtime(
                        pos,
                        "Switch statements may only contain one 'default' clause.",
                    ));
                }
            };
            let case_expr_instrs = case_exprs
                .into_iter()
                .filter_map(|x| x.map(|x| emit_check_case(e, env, scrutinee_expr, x)))
                .collect::<Result<Vec<_>>>()?;

            Ok((
                InstrSeq::gather(alloc, case_expr_instrs),
                InstrSeq::gather(alloc, case_body_instrs),
                default_label,
            ))
        }
    }
}

fn emit_switch<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    scrutinee_expr: &tast::Expr,
    cl: &Vec<tast::Case>,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let (instr_init, instr_free) = if scrutinee_expr.1.is_lvar() {
        (instr::empty(alloc), instr::empty(alloc))
    } else {
        (
            emit_expr::emit_expr(e, env, scrutinee_expr)?,
            instr::popc(alloc),
        )
    };
    let break_label = e.label_gen_mut().next_regular();

    let (case_expr_instrs, case_body_instrs, default_label) =
        env.do_in_switch_body(e, break_label, cl, |env, e, cases| {
            emit_cases(env, e, pos, break_label, scrutinee_expr, cases)
        })?;
    Ok(InstrSeq::gather(
        alloc,
        vec![
            instr_init,
            case_expr_instrs,
            instr_free,
            instr::jmp(alloc, default_label),
            case_body_instrs,
            instr::label(alloc, break_label),
        ],
    ))
}

fn is_empty_block(b: &[tast::Stmt]) -> bool {
    b.iter().all(|s| s.1.is_noop())
}

fn emit_try_catch_finally<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    r#try: &[tast::Stmt],
    catch: &[tast::Catch],
    finally: &tast::Block,
) -> Result<InstrSeq<'arena>> {
    let is_try_block_empty = false;
    let emit_try_block =
        |env: &mut Env<'a, 'arena>, e: &mut Emitter<'arena>, finally_start: Label| {
            env.do_in_try_catch_body(e, finally_start, r#try, catch, |env, e, t, c| {
                emit_try_catch(e, env, pos, t, c)
            })
        };
    e.local_scope(|e| emit_try_finally_(e, env, pos, emit_try_block, finally, is_try_block_empty))
}

fn emit_try_finally<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    try_block: &[tast::Stmt],
    finally_block: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let is_try_block_empty = is_empty_block(try_block);
    let emit_try_block =
        |env: &mut Env<'a, 'arena>, e: &mut Emitter<'arena>, finally_start: Label| {
            env.do_in_try_body(e, finally_start, try_block, emit_block)
        };
    e.local_scope(|e| {
        emit_try_finally_(
            e,
            env,
            pos,
            emit_try_block,
            finally_block,
            is_try_block_empty,
        )
    })
}

fn emit_try_finally_<
    'a,
    'arena,
    E: Fn(&mut Env<'a, 'arena>, &mut Emitter<'arena>, Label) -> Result<InstrSeq<'arena>>,
>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    emit_try_block: E,
    finally_block: &[tast::Stmt],
    is_try_block_empty: bool,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    if is_try_block_empty {
        return env.do_in_finally_body(e, finally_block, emit_block);
    };
    // We need to generate four things:
    // (1) the try-body, which will be followed by
    // (2) the normal-continuation finally body, and
    // (3) an epilogue to the finally body that deals with finally-blocked
    //     break and continue
    // (4) the exceptional-continuation catch body.
    //

    //     (1) Try body

    // The try body might have un-rewritten continues and breaks which
    // branch to a label outside of the try. This means that we must
    // first run the normal-continuation finally, and then branch to the
    // appropriate label.

    // We do this by running a rewriter which turns continues and breaks
    // inside the try body into setting temp_local to an integer which indicates
    // what action the finally must perform when it is finished, followed by a
    // jump directly to the finally.
    let finally_start = e.label_gen_mut().next_regular();
    let finally_end = e.label_gen_mut().next_regular();

    let in_try = env.flags.contains(hhbc_by_ref_env::Flags::IN_TRY);
    env.flags.set(hhbc_by_ref_env::Flags::IN_TRY, true);
    let try_body_result = emit_try_block(env, e, finally_start);
    env.flags.set(hhbc_by_ref_env::Flags::IN_TRY, in_try);

    let try_body = try_body_result?;
    let jump_instrs = tfr::JumpInstructions::collect(&try_body, &mut env.jump_targets_gen);
    let jump_instrs_is_empty = jump_instrs.is_empty();

    //  (2) Finally body

    // Note that this is used both in the normal-continuation and
    // exceptional-continuation cases; we generate the same code twice.

    // TODO: We might consider changing the codegen so that the finally block
    // is only generated once. We could do this by making the catch block set a
    // temp local to -1, and then branch to the finally block. In the finally block
    // epilogue it can check to see if the local is -1, and if so, issue an unwind
    // instruction.

    // It is illegal to have a continue or break which branches out of a finally.
    // Unfortunately we at present do not detect this at parse time; rather, we
    // generate an exception at run-time by rewriting continue and break
    // instructions found inside finally blocks.

    // TODO: If we make this illegal at parse time then we can remove this pass.
    let exn_local = e.local_gen_mut().get_unnamed();
    let finally_body = env.do_in_finally_body(e, finally_block, emit_block)?;
    let mut finally_body_for_catch = InstrSeq::clone(alloc, &finally_body);
    label_rewriter::clone_with_fresh_regular_labels(e, &mut finally_body_for_catch);

    //  (3) Finally epilogue
    let finally_epilogue = tfr::emit_finally_epilogue(e, env, pos, jump_instrs, finally_end)?;

    //  (4) Catch body

    // We now emit the catch body; it is just cleanup code for the temp_local,
    // a copy of the finally body (without the branching epilogue, since we are
    // going to unwind rather than branch), and an unwind instruction.

    // TODO: The HHVM emitter sometimes emits seemingly spurious
    // unset-unnamed-local instructions into the catch block.  These look
    // like bugs in the emitter. Investigate; if they are bugs in the HHVM
    // emitter, get them fixed there. If not, get a clear explanation of
    // what they are for and why they are required.

    let enclosing_span = env.scope.get_span();
    let try_instrs = if jump_instrs_is_empty {
        try_body
    } else {
        tfr::cleanup_try_body(alloc, &try_body)
    };
    let catch_instrs = InstrSeq::gather(
        alloc,
        vec![
            emit_pos(alloc, &enclosing_span),
            make_finally_catch(alloc, e, exn_local, finally_body_for_catch),
        ],
    );
    let middle = InstrSeq::create_try_catch(
        alloc,
        e.label_gen_mut(),
        None,
        true,
        try_instrs,
        catch_instrs,
    );

    // Putting it all together
    Ok(InstrSeq::gather(
        alloc,
        vec![
            middle,
            instr::label(alloc, finally_start),
            emit_pos(alloc, pos),
            finally_body,
            finally_epilogue,
            instr::label(alloc, finally_end),
        ],
    ))
}

fn make_finally_catch<'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    exn_local: local::Type<'arena>,
    finally_body: InstrSeq<'arena>,
) -> InstrSeq<'arena> {
    let l2 = instr::unsetl(alloc, *e.local_gen_mut().get_retval());
    let l1 = instr::unsetl(alloc, *e.local_gen_mut().get_label());
    InstrSeq::gather(
        alloc,
        vec![
            instr::popl(alloc, exn_local),
            l1,
            l2,
            InstrSeq::create_try_catch(
                alloc,
                e.label_gen_mut(),
                None,
                false,
                finally_body,
                InstrSeq::gather(
                    alloc,
                    vec![instr::pushl(alloc, exn_local), instr::chain_faults(alloc)],
                ),
            ),
            instr::pushl(alloc, exn_local),
            instr::throw(alloc),
        ],
    )
}

fn emit_try_catch<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    try_block: &[tast::Stmt],
    catch_list: &[tast::Catch],
) -> Result<InstrSeq<'arena>> {
    e.local_scope(|e| emit_try_catch_(e, env, pos, try_block, catch_list))
}

fn emit_try_catch_<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    try_block: &[tast::Stmt],
    catch_list: &[tast::Catch],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    if is_empty_block(&try_block) {
        return Ok(instr::empty(alloc));
    };
    let end_label = e.label_gen_mut().next_regular();

    let catch_instrs = InstrSeq::gather(
        alloc,
        catch_list
            .iter()
            .map(|catch| emit_catch(e, env, pos, end_label, catch))
            .collect::<Result<Vec<_>>>()?,
    );
    let in_try = env.flags.contains(hhbc_by_ref_env::Flags::IN_TRY);
    env.flags.set(hhbc_by_ref_env::Flags::IN_TRY, true);
    let try_body = emit_stmts(e, env, try_block);
    env.flags.set(hhbc_by_ref_env::Flags::IN_TRY, in_try);

    let try_instrs = InstrSeq::gather(alloc, vec![try_body?, emit_pos(alloc, pos)]);
    Ok(InstrSeq::create_try_catch(
        alloc,
        e.label_gen_mut(),
        Some(end_label),
        false,
        try_instrs,
        catch_instrs,
    ))
}

fn emit_catch<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    end_label: Label,
    catch: &tast::Catch,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    // Note that this is a "regular" label; we're not going to branch to
    // it directly in the event of an exception.
    let next_catch = e.label_gen_mut().next_regular();
    let id = hhbc_id::class::Type::from_ast_name_and_mangle(alloc, &(catch.0).1);
    Ok(InstrSeq::gather(
        alloc,
        vec![
            instr::dup(alloc),
            instr::instanceofd(alloc, id),
            instr::jmpz(alloc, next_catch),
            instr::setl(
                alloc,
                local::Type::Named(
                    bumpalo::collections::String::from_str_in(&((catch.1).1).1, alloc)
                        .into_bump_str(),
                ),
            ),
            instr::popc(alloc),
            emit_stmts(e, env, &catch.2)?,
            emit_pos(alloc, pos),
            instr::jmp(alloc, end_label),
            instr::label(alloc, next_catch),
        ],
    ))
}

fn emit_foreach<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    collection: &tast::Expr,
    iterator: &tast::AsExpr,
    block: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    use tast::AsExpr as A;
    e.local_scope(|e| match iterator {
        A::AsV(_) | A::AsKv(_, _) => emit_foreach_(e, env, pos, collection, iterator, block),
        A::AwaitAsV(pos, _) | A::AwaitAsKv(pos, _, _) => {
            emit_foreach_await(e, env, pos, collection, iterator, block)
        }
    })
}

fn emit_foreach_<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    collection: &tast::Expr,
    iterator: &tast::AsExpr,
    block: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let collection_instrs = emit_expr::emit_expr(e, env, collection)?;
    scope::with_unnamed_locals_and_iterators(alloc, e, |alloc, e| {
        let iter_id = e.iterator_mut().get();
        let loop_break_label = e.label_gen_mut().next_regular();
        let loop_continue_label = e.label_gen_mut().next_regular();
        let loop_head_label = e.label_gen_mut().next_regular();
        let (key_id, val_id, preamble) = emit_iterator_key_value_storage(e, env, iterator)?;
        let iter_args = IterArgs {
            iter_id,
            key_id,
            val_id,
        };
        let body = env.do_in_loop_body(
            e,
            loop_break_label,
            loop_continue_label,
            Some(iter_id),
            block,
            emit_block,
        )?;
        let iter_init = InstrSeq::gather(
            alloc,
            vec![
                collection_instrs,
                emit_pos(alloc, &collection.0),
                instr::iterinit(alloc, iter_args.clone(), loop_break_label),
            ],
        );
        let iterate = InstrSeq::gather(
            alloc,
            vec![
                instr::label(alloc, loop_head_label),
                preamble,
                body,
                instr::label(alloc, loop_continue_label),
                emit_pos(alloc, pos),
                instr::iternext(alloc, iter_args, loop_head_label),
            ],
        );
        let iter_done = instr::label(alloc, loop_break_label);
        Ok((iter_init, iterate, iter_done))
    })
}

fn emit_foreach_await<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    collection: &tast::Expr,
    iterator: &tast::AsExpr,
    block: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let instr_collection = emit_expr::emit_expr(e, env, collection)?;
    scope::with_unnamed_local(alloc, e, |alloc, e, iter_temp_local| {
        let input_is_async_iterator_label = e.label_gen_mut().next_regular();
        let next_label = e.label_gen_mut().next_regular();
        let exit_label = e.label_gen_mut().next_regular();
        let pop_and_exit_label = e.label_gen_mut().next_regular();
        let async_eager_label = e.label_gen_mut().next_regular();
        let next_meth = hhbc_id::method::from_raw_string(alloc, "next");
        let iter_init = InstrSeq::gather(
            alloc,
            vec![
                instr_collection,
                instr::dup(alloc),
                instr::instanceofd(
                    alloc,
                    hhbc_id::class::from_raw_string(alloc, "HH\\AsyncIterator"),
                ),
                instr::jmpnz(alloc, input_is_async_iterator_label),
                emit_fatal::emit_fatal_runtime(
                    alloc,
                    pos,
                    "Unable to iterate non-AsyncIterator asynchronously",
                ),
                instr::label(alloc, input_is_async_iterator_label),
                instr::popl(alloc, iter_temp_local),
            ],
        );
        let loop_body_instr =
            env.do_in_loop_body(e, exit_label, next_label, None, block, emit_block)?;
        let iterate = InstrSeq::gather(
            alloc,
            vec![
                instr::label(alloc, next_label),
                instr::cgetl(alloc, iter_temp_local),
                instr::nulluninit(alloc),
                instr::fcallobjmethodd(
                    alloc,
                    FcallArgs::new(
                        FcallFlags::empty(),
                        1,
                        bumpalo::vec![in alloc;].into_bump_slice(),
                        Some(async_eager_label),
                        0,
                        None,
                    ),
                    next_meth,
                    ObjNullFlavor::NullThrows,
                ),
                instr::await_(alloc),
                instr::label(alloc, async_eager_label),
                instr::dup(alloc),
                instr::istypec(alloc, IstypeOp::OpNull),
                instr::jmpnz(alloc, pop_and_exit_label),
                emit_foreach_await_key_value_storage(e, env, iterator)?,
                loop_body_instr,
                emit_pos(alloc, pos),
                instr::jmp(alloc, next_label),
                instr::label(alloc, pop_and_exit_label),
                instr::popc(alloc),
                instr::label(alloc, exit_label),
            ],
        );
        let iter_done = instr::unsetl(alloc, iter_temp_local);
        Ok((iter_init, iterate, iter_done))
    })
}

// Assigns a location to store values for foreach-key and foreach-value and
// creates a code to populate them.
// NOT suitable for foreach (... await ...) which uses different code-gen
// Returns: key_local_opt * value_local * key_preamble * value_preamble
// where:
// - key_local_opt - local variable to store a foreach-key value if it is
//     declared
// - value_local - local variable to store a foreach-value
// - key_preamble - list of instructions to populate foreach-key
// - value_preamble - list of instructions to populate foreach-value
fn emit_iterator_key_value_storage<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    iterator: &tast::AsExpr,
) -> Result<(
    Option<local::Type<'arena>>,
    local::Type<'arena>,
    InstrSeq<'arena>,
)> {
    use tast::AsExpr as A;
    let alloc = env.arena;
    fn get_id_of_simple_lvar_opt(lvar: &tast::Expr_) -> Result<Option<&str>> {
        if let Some(tast::Lid(pos, id)) = lvar.as_lvar() {
            let name = local_id::get_name(&id);
            if name == special_idents::THIS {
                return Err(emit_fatal::raise_fatal_parse(
                    &pos,
                    "Cannot re-assign $this",
                ));
            } else if !(superglobals::is_superglobal(&name)) {
                return Ok(Some(name));
            }
        };
        Ok(None)
    }
    match iterator {
        A::AsKv(k, v) => Ok(
            match (
                get_id_of_simple_lvar_opt(&k.1)?,
                get_id_of_simple_lvar_opt(&v.1)?,
            ) {
                (Some(key_id), Some(val_id)) => (
                    Some(local::Type::Named(alloc.alloc_str(key_id))),
                    local::Type::Named(alloc.alloc_str(val_id)),
                    instr::empty(alloc),
                ),
                _ => {
                    let key_local = e.local_gen_mut().get_unnamed();
                    let val_local = e.local_gen_mut().get_unnamed();
                    let (mut key_preamble, key_load) =
                        emit_iterator_lvalue_storage(e, env, k, key_local)?;
                    let (mut val_preamble, val_load) =
                        emit_iterator_lvalue_storage(e, env, v, val_local)?;
                    // HHVM prepends code to initialize non-plain, non-list foreach-key
                    // to the value preamble - do the same to minimize diffs
                    if !(k.1).is_list() {
                        key_preamble.extend(val_preamble);
                        val_preamble = key_preamble;
                        key_preamble = vec![];
                    };
                    (
                        Some(key_local),
                        val_local,
                        InstrSeq::gather(
                            alloc,
                            vec![
                                InstrSeq::gather(alloc, val_preamble),
                                InstrSeq::gather(alloc, val_load),
                                InstrSeq::gather(alloc, key_preamble),
                                InstrSeq::gather(alloc, key_load),
                            ],
                        ),
                    )
                }
            },
        ),
        A::AsV(v) => Ok(match get_id_of_simple_lvar_opt(&v.1)? {
            Some(val_id) => (
                None,
                local::Type::Named(alloc.alloc_str(val_id)),
                instr::empty(alloc),
            ),
            None => {
                let val_local = e.local_gen_mut().get_unnamed();
                let (val_preamble, val_load) = emit_iterator_lvalue_storage(e, env, v, val_local)?;
                (
                    None,
                    val_local,
                    InstrSeq::gather(
                        alloc,
                        vec![
                            InstrSeq::gather(alloc, val_preamble),
                            InstrSeq::gather(alloc, val_load),
                        ],
                    ),
                )
            }
        }),
        _ => Err(Unrecoverable(
            "emit_iterator_key_value_storage with iterator using await".into(),
        )),
    }
}

fn emit_iterator_lvalue_storage<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    lvalue: &tast::Expr,
    local: local::Type<'arena>,
) -> Result<(Vec<InstrSeq<'arena>>, Vec<InstrSeq<'arena>>)> {
    let alloc = env.arena;
    match &lvalue.1 {
        tast::Expr_::Call(_) => Err(emit_fatal::raise_fatal_parse(
            &lvalue.0,
            "Can't use return value in write context",
        )),
        tast::Expr_::List(es) => {
            let (preamble, load_values) = emit_load_list_elements(
                e,
                env,
                vec![instr::basel(alloc, local, MemberOpMode::Warn)],
                es,
            )?;
            let load_values = vec![
                InstrSeq::gather(alloc, load_values.into_iter().rev().collect()),
                instr::unsetl(alloc, local),
            ];
            Ok((preamble, load_values))
        }
        _ => {
            let (lhs, rhs, set_op) = emit_expr::emit_lval_op_nonlist_steps(
                e,
                env,
                &lvalue.0,
                LValOp::Set,
                lvalue,
                instr::cgetl(alloc, local),
                1,
                false,
            )?;
            Ok((
                vec![lhs],
                vec![rhs, set_op, instr::popc(alloc), instr::unsetl(alloc, local)],
            ))
        }
    }
}

fn emit_load_list_elements<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    path: Vec<InstrSeq<'arena>>,
    es: &[tast::Expr],
) -> Result<(Vec<InstrSeq<'arena>>, Vec<InstrSeq<'arena>>)> {
    let alloc = env.arena;
    let (preamble, load_value): (Vec<Vec<InstrSeq<'arena>>>, Vec<Vec<InstrSeq<'arena>>>) = es
        .iter()
        .enumerate()
        .map(|(i, x)| {
            emit_load_list_element(
                e,
                env,
                path.iter()
                    .map(|x| InstrSeq::clone(alloc, x))
                    .collect::<Vec<_>>(),
                i,
                x,
            )
        })
        .collect::<Result<Vec<_>>>()?
        .into_iter()
        .unzip();
    Ok((
        preamble.into_iter().flatten().collect(),
        load_value.into_iter().flatten().collect(),
    ))
}

fn emit_load_list_element<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    mut path: Vec<InstrSeq<'arena>>,
    i: usize,
    elem: &tast::Expr,
) -> Result<(Vec<InstrSeq<'arena>>, Vec<InstrSeq<'arena>>)> {
    let alloc = env.arena;
    let query_value = |path| {
        InstrSeq::gather(
            alloc,
            vec![
                InstrSeq::gather(alloc, path),
                instr::querym(
                    alloc,
                    0,
                    QueryOp::CGet,
                    MemberKey::EI(i as i64, ReadOnlyOp::Any),
                ),
            ],
        )
    };
    Ok(match &elem.1 {
        tast::Expr_::Lvar(lid) => {
            let load_value = InstrSeq::gather(
                alloc,
                vec![
                    query_value(path),
                    instr::setl(
                        alloc,
                        local::Type::Named(
                            bumpalo::collections::String::from_str_in(
                                local_id::get_name(&lid.1),
                                alloc,
                            )
                            .into_bump_str(),
                        ),
                    ),
                    instr::popc(alloc),
                ],
            );
            (vec![], vec![load_value])
        }
        tast::Expr_::List(es) => {
            let instr_dim = instr::dim(
                alloc,
                MemberOpMode::Warn,
                MemberKey::EI(i as i64, ReadOnlyOp::Any),
            );
            path.push(instr_dim);
            emit_load_list_elements(e, env, path, es)?
        }
        _ => {
            let set_instrs = emit_expr::emit_lval_op_nonlist(
                e,
                env,
                &elem.0,
                LValOp::Set,
                elem,
                query_value(path),
                1,
                false,
            )?;
            let load_value = InstrSeq::gather(alloc, vec![set_instrs, instr::popc(alloc)]);
            (vec![], vec![load_value])
        }
    })
}

//Emit code for the value and possibly key l-value operation in a foreach
// await statement. The result of invocation of the `next` method has been
// stored on top of the stack. For example:
//   foreach (foo() await as $a->f => list($b[0], $c->g)) { ... }
// Here, we need to construct l-value operations that access the [0] (for $a->f)
// and [1;0] (for $b[0]) and [1;1] (for $c->g) indices of the array returned
// from the `next` method.
fn emit_foreach_await_key_value_storage<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    iterator: &tast::AsExpr,
) -> Result<InstrSeq<'arena>> {
    use tast::AsExpr as A;
    let alloc = env.arena;
    match iterator {
        A::AwaitAsKv(_, k, v) | A::AsKv(k, v) => Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_foreach_await_lvalue_storage(e, env, k, &[0], true)?,
                emit_foreach_await_lvalue_storage(e, env, v, &[1], false)?,
            ],
        )),
        A::AwaitAsV(_, v) | A::AsV(v) => emit_foreach_await_lvalue_storage(e, env, v, &[1], false),
    }
}

// Emit code for either the key or value l-value operation in foreach await.
// `indices` is the initial prefix of the array indices ([0] for key or [1] for
// value) that is prepended onto the indices needed for list destructuring
//
// TODO: we don't need unnamed local if the target is a local
fn emit_foreach_await_lvalue_storage<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    lvalue: &tast::Expr,
    indices: &[isize],
    keep_on_stack: bool,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    scope::with_unnamed_local(alloc, e, |alloc, e, local| {
        Ok((
            instr::popl(alloc, local),
            (
                alloc,
                emit_expr::emit_lval_op_list(
                    e,
                    env,
                    &lvalue.0,
                    Some(&local),
                    indices,
                    lvalue,
                    false,
                )?,
            )
                .into(),
            if keep_on_stack {
                instr::pushl(alloc, local)
            } else {
                instr::unsetl(alloc, local)
            },
        ))
    })
}

fn emit_stmts<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    stl: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    Ok(InstrSeq::gather(
        alloc,
        stl.iter()
            .map(|s| emit_stmt(e, env, s))
            .collect::<Result<Vec<_>>>()?,
    ))
}

fn emit_block<'a, 'arena>(
    env: &mut Env<'a, 'arena>,
    emitter: &mut Emitter<'arena>,
    block: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    emit_stmts(emitter, env, block)
}

fn emit_do<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    body: &[tast::Stmt],
    cond: &tast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let break_label = e.label_gen_mut().next_regular();
    let cont_label = e.label_gen_mut().next_regular();
    let start_label = e.label_gen_mut().next_regular();
    let jmpnz_instr = emit_expr::emit_jmpnz(e, env, cond, start_label)?.instrs;
    Ok(InstrSeq::gather(
        alloc,
        vec![
            instr::label(alloc, start_label),
            env.do_in_loop_body(e, break_label, cont_label, None, body, emit_block)?,
            instr::label(alloc, cont_label),
            jmpnz_instr,
            instr::label(alloc, break_label),
        ],
    ))
}

fn emit_while<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    cond: &tast::Expr,
    body: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let break_label = e.label_gen_mut().next_regular();
    let cont_label = e.label_gen_mut().next_regular();
    let start_label = e.label_gen_mut().next_regular();
    /* TODO: This is *bizarre* codegen for a while loop.
             It would be better to generate this as
             instr_label continue_label;
             emit_expr e;
             instr_jmpz break_label;
             body;
             instr_jmp continue_label;
             instr_label break_label;
    */
    let i3 = emit_expr::emit_jmpnz(e, env, cond, start_label)?.instrs;
    let i2 = env.do_in_loop_body(e, break_label, cont_label, None, body, emit_block)?;
    let i1 = emit_expr::emit_jmpz(e, env, cond, break_label)?.instrs;
    Ok(InstrSeq::gather(
        alloc,
        vec![
            i1,
            instr::label(alloc, start_label),
            i2,
            instr::label(alloc, cont_label),
            i3,
            instr::label(alloc, break_label),
        ],
    ))
}

fn emit_for<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    e1: &Vec<tast::Expr>,
    e2: &Option<tast::Expr>,
    e3: &Vec<tast::Expr>,
    body: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let break_label = e.label_gen_mut().next_regular();
    let cont_label = e.label_gen_mut().next_regular();
    let start_label = e.label_gen_mut().next_regular();
    fn emit_cond<'a, 'arena>(
        emitter: &mut Emitter<'arena>,
        env: &mut Env<'a, 'arena>,
        jmpz: bool,
        label: Label,
        cond: &Option<tast::Expr>,
    ) -> Result<InstrSeq<'arena>> {
        let alloc = env.arena;
        Ok(match cond {
            None => {
                if jmpz {
                    instr::empty(alloc)
                } else {
                    instr::jmp(alloc, label)
                }
            }
            Some(cond) => {
                if jmpz {
                    emit_expr::emit_jmpz(emitter, env, cond, label)
                } else {
                    emit_expr::emit_jmpnz(emitter, env, cond, label)
                }?
                .instrs
            }
        })
    }
    // TODO: this is bizarre codegen for a "for" loop.
    //  This should be codegen'd as
    //  emit_ignored_expr initializer;
    //  instr_label start_label;
    //  from_expr condition;
    //  instr_jmpz break_label;
    //  body;
    //  instr_label continue_label;
    //  emit_ignored_expr increment;
    //  instr_jmp start_label;
    //  instr_label break_label;
    let i5 = emit_cond(e, env, false, start_label, e2)?;
    let i4 = emit_expr::emit_ignored_exprs(e, env, &Pos::make_none(), e3)?;
    let i3 = env.do_in_loop_body(e, break_label, cont_label, None, body, emit_block)?;
    let i2 = emit_cond(e, env, true, break_label, e2)?;
    let i1 = emit_expr::emit_ignored_exprs(e, env, &Pos::make_none(), e1)?;
    Ok(InstrSeq::gather(
        alloc,
        vec![
            i1,
            i2,
            instr::label(alloc, start_label),
            i3,
            instr::label(alloc, cont_label),
            i4,
            i5,
            instr::label(alloc, break_label),
        ],
    ))
}

pub fn emit_dropthrough_return<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match e.emit_statement_state().default_dropthrough.as_ref() {
        Some(instrs) => Ok(InstrSeq::clone(alloc, instrs)),
        None => {
            let ret = emit_return(e, env)?;
            let state = e.emit_statement_state();
            Ok(emit_pos_then(
                alloc,
                &(state.function_pos.last_char()),
                InstrSeq::gather(
                    alloc,
                    vec![InstrSeq::clone(alloc, &state.default_return_value), ret],
                ),
            ))
        }
    }
}

pub fn emit_final_stmt<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    stmt: &tast::Stmt,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match &stmt.1 {
        a::Stmt_::Throw(_) | a::Stmt_::Return(_) | a::Stmt_::YieldBreak => emit_stmt(e, env, stmt),
        a::Stmt_::Block(stmts) => emit_final_stmts(e, env, stmts),
        _ => {
            let ret = emit_dropthrough_return(e, env)?;
            Ok(InstrSeq::gather(alloc, vec![emit_stmt(e, env, stmt)?, ret]))
        }
    }
}

pub fn emit_final_stmts<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    block: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match block {
        [] => emit_dropthrough_return(e, env),
        _ => {
            let mut ret = Vec::with_capacity(block.len());
            for (i, s) in block.iter().enumerate() {
                let instrs = if i == block.len() - 1 {
                    emit_final_stmt(e, env, s)?
                } else {
                    emit_stmt(e, env, s)?
                };
                ret.push(instrs);
            }
            Ok(InstrSeq::gather(alloc, ret))
        }
    }
}

pub fn emit_markup<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    (_, s): &tast::Pstring,
    check_for_hashbang: bool,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    let mut emit_ignored_call_expr = |fname: String, expr: tast::Expr| {
        let call_expr = tast::Expr(
            Pos::make_none(),
            tast::Expr_::mk_call(
                tast::Expr(
                    Pos::make_none(),
                    tast::Expr_::mk_id(ast_defs::Id(Pos::make_none(), fname)),
                ),
                vec![],
                vec![expr],
                None,
            ),
        );
        emit_expr::emit_ignored_expr(e, env, &Pos::make_none(), &call_expr)
    };
    let mut emit_ignored_call_expr_for_nonempty_str = |fname: String, expr_str: String| {
        if expr_str.is_empty() {
            Ok(instr::empty(alloc))
        } else {
            emit_ignored_call_expr(
                fname,
                tast::Expr(Pos::make_none(), tast::Expr_::mk_string(expr_str.into())),
            )
        }
    };
    let markup = if s.is_empty() {
        instr::empty(alloc)
    } else {
        lazy_static! {
            static ref HASHBANG_PAT: regex::Regex = regex::Regex::new(r"^#!.*\n").unwrap();
        }
        let tail = String::from(match HASHBANG_PAT.shortest_match(&s) {
            Some(i) if check_for_hashbang => {
                // if markup text starts with #!
                // - extract a line with hashbang
                // - it will be emitted as a call to print_hashbang function
                // - emit remaining part of text as regular markup
                &s[i..]
            }
            _ => s,
        });
        emit_ignored_call_expr_for_nonempty_str(special_functions::ECHO.into(), tail)?
    };
    Ok(markup)
}

fn emit_break<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
) -> InstrSeq<'arena> {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::IS_BREAK, env, pos, 1)
}

fn emit_continue<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
) -> InstrSeq<'arena> {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::empty(), env, pos, 1)
}

fn emit_await_assignment<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    lval: &tast::Expr,
    r: &tast::Expr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match lval.1.as_lvar() {
        Some(tast::Lid(_, id)) if !emit_expr::is_local_this(env, &id) => Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_expr::emit_await(e, env, pos, r)?,
                emit_pos(alloc, pos),
                instr::popl(
                    alloc,
                    emit_expr::get_local(e, env, pos, local_id::get_name(&id))?,
                ),
            ],
        )),
        _ => {
            let awaited_instrs = emit_await(e, env, pos, r)?;
            scope::with_unnamed_local(alloc, e, |alloc, e, temp| {
                let rhs_instrs = instr::pushl(alloc, temp);
                let (lhs, rhs, setop) = emit_expr::emit_lval_op_nonlist_steps(
                    e,
                    env,
                    pos,
                    LValOp::Set,
                    lval,
                    rhs_instrs,
                    1,
                    false,
                )?;
                Ok((
                    InstrSeq::gather(alloc, vec![awaited_instrs, instr::popl(alloc, temp)]),
                    lhs,
                    InstrSeq::gather(alloc, vec![rhs, setop, instr::popc(alloc)]),
                ))
            })
        }
    }
}

fn emit_if<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    condition: &tast::Expr,
    consequence: &[tast::Stmt],
    alternative: &[tast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    if alternative.is_empty() || (alternative.len() == 1 && alternative[0].1.is_noop()) {
        let done_label = e.label_gen_mut().next_regular();
        let consequence_instr = emit_stmts(e, env, consequence)?;
        Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_expr::emit_jmpz(e, env, condition, done_label)?.instrs,
                consequence_instr,
                instr::label(alloc, done_label),
            ],
        ))
    } else {
        let alternative_label = e.label_gen_mut().next_regular();
        let done_label = e.label_gen_mut().next_regular();
        let consequence_instr = emit_stmts(e, env, consequence)?;
        let alternative_instr = emit_stmts(e, env, alternative)?;
        Ok(InstrSeq::gather(
            alloc,
            vec![
                emit_expr::emit_jmpz(e, env, condition, alternative_label)?.instrs,
                consequence_instr,
                emit_pos(alloc, pos),
                instr::jmp(alloc, done_label),
                instr::label(alloc, alternative_label),
                alternative_instr,
                instr::label(alloc, done_label),
            ],
        ))
    }
}
