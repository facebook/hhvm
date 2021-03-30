// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::try_finally_rewriter as tfr;

use emit_expression_rust::{self as emit_expr, emit_await, emit_expr, LValOp, Setrange};
use emit_fatal_rust as emit_fatal;
use emit_pos_rust::{emit_pos, emit_pos_then};
use env::{emitter::Emitter, Env};
use hhbc_ast_rust::*;
use hhbc_id_rust::{self as hhbc_id, Id};
use instruction_sequence::{instr, Error::Unrecoverable, InstrSeq, Result};
use label_rewriter_rust as label_rewriter;
use label_rust::Label;
use lazy_static::lazy_static;
use naming_special_names_rust::{special_functions, special_idents, superglobals};
use oxidized::{aast as a, ast as tast, ast_defs, local_id, pos::Pos};
use regex::Regex;
use scope_rust::scope;
use statement_state::StatementState;

// Expose a mutable ref to state for emit_body so that it can set it appropriately
pub(crate) fn set_state(e: &mut Emitter, state: StatementState) {
    *e.emit_statement_state_mut() = state;
}

pub(crate) type Level = usize;

// Wrapper functions

fn emit_return(e: &mut Emitter, env: &mut Env) -> Result {
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

pub fn emit_stmt(e: &mut Emitter, env: &mut Env, stmt: &tast::Stmt) -> Result {
    let pos = &stmt.0;
    match &stmt.1 {
        a::Stmt_::YieldBreak => Ok(InstrSeq::gather(vec![instr::null(), emit_return(e, env)?])),
        a::Stmt_::Expr(e_) => match &e_.1 {
            a::Expr_::Await(a) => Ok(InstrSeq::gather(vec![
                emit_await(e, env, &e_.0, a)?,
                instr::popc(),
            ])),
            a::Expr_::Call(c) => {
                if let (a::Expr(_, a::Expr_::Id(sid)), _, exprs, None) = c.as_ref() {
                    let ft = hhbc_id::function::Type::from_ast_name(&sid.1);
                    let fname = ft.to_raw_string();
                    if fname.eq_ignore_ascii_case("unset") {
                        Ok(InstrSeq::gather(
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
                                scope::with_unnamed_local(e, |e, temp| {
                                    Ok((
                                        InstrSeq::gather(vec![
                                            awaited_instrs,
                                            instr::popl(temp.clone()),
                                        ]),
                                        emit_expr::emit_lval_op_list(
                                            e,
                                            env,
                                            pos,
                                            Some(&temp),
                                            &[],
                                            e_lhs,
                                            false,
                                        )?
                                        .into(),
                                        instr::unsetl(temp),
                                    ))
                                })
                            } else {
                                Ok(InstrSeq::gather(vec![awaited_instrs, instr::popc()]))
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
                Ok(InstrSeq::gather(vec![expr_instr, emit_pos(pos), ret]))
            }
            None => Ok(InstrSeq::gather(vec![
                instr::null(),
                emit_pos(pos),
                emit_return(e, env)?,
            ])),
        },
        a::Stmt_::Block(b) => emit_block(env, e, &b),
        a::Stmt_::If(f) => emit_if(e, env, pos, &f.0, &f.1, &f.2),
        a::Stmt_::While(x) => emit_while(e, env, &x.0, &x.1),
        a::Stmt_::Using(x) => emit_using(e, env, pos, &**x),
        a::Stmt_::Break => Ok(emit_break(e, env, pos)),
        a::Stmt_::Continue => Ok(emit_continue(e, env, pos)),
        a::Stmt_::Do(x) => emit_do(e, env, &x.0, &x.1),
        a::Stmt_::For(x) => emit_for(e, env, &x.0, &x.1, &x.2, &x.3),
        a::Stmt_::Throw(x) => Ok(InstrSeq::gather(vec![
            emit_expr::emit_expr(e, env, x)?,
            emit_pos(pos),
            instr::throw(),
        ])),
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
        a::Stmt_::Fallthrough | a::Stmt_::Noop => Ok(instr::empty()),
        a::Stmt_::AssertEnv(_) => Ok(instr::empty()),
    }
}

fn emit_case<'c>(
    e: &mut Emitter,
    env: &mut Env,
    case: &'c tast::Case,
) -> Result<(InstrSeq, (Option<(&'c tast::Expr, Label)>, Option<Label>))> {
    let l = e.label_gen_mut().next_regular();
    Ok(match case {
        tast::Case::Case(case_expr, b) => (
            InstrSeq::gather(vec![instr::label(l.clone()), emit_block(env, e, b)?]),
            (Some((case_expr, l)), None),
        ),
        tast::Case::Default(_, b) => (
            InstrSeq::gather(vec![instr::label(l.clone()), emit_block(env, e, b)?]),
            (None, Some(l)),
        ),
    })
}

fn emit_check_case(
    e: &mut Emitter,
    env: &mut Env,
    scrutinee_expr: &tast::Expr,
    (case_expr, case_handler_label): (&tast::Expr, Label),
) -> Result {
    Ok(if scrutinee_expr.1.is_lvar() {
        InstrSeq::gather(vec![
            emit_expr::emit_two_exprs(e, env, &case_expr.0, scrutinee_expr, &case_expr)?,
            instr::eq(),
            instr::jmpnz(case_handler_label),
        ])
    } else {
        let next_case_label = e.label_gen_mut().next_regular();
        InstrSeq::gather(vec![
            instr::dup(),
            emit_expr::emit_expr(e, env, &case_expr)?,
            emit_pos(&case_expr.0),
            instr::eq(),
            instr::jmpz(next_case_label.clone()),
            instr::popc(),
            instr::jmp(case_handler_label),
            instr::label(next_case_label),
        ])
    })
}

fn emit_awaitall(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    el: &[(Option<tast::Lid>, tast::Expr)],
    block: &tast::Block,
) -> Result {
    match el {
        [] => Ok(instr::empty()),
        [(lvar, expr)] => emit_awaitall_single(e, env, pos, lvar, expr, block),
        _ => emit_awaitall_multi(e, env, pos, el, block),
    }
}

fn emit_awaitall_single(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    lval: &Option<tast::Lid>,
    expr: &tast::Expr,
    block: &tast::Block,
) -> Result {
    scope::with_unnamed_locals(e, |e| {
        let load_arg = emit_expr::emit_await(e, env, pos, expr)?;
        let (load, unset) = match lval {
            None => (instr::popc(), instr::empty()),
            Some(tast::Lid(_, id)) => {
                let l = e
                    .local_gen_mut()
                    .init_unnamed_for_tempname(local_id::get_name(&id));
                (instr::popl(l.clone()), instr::unsetl(l.clone()))
            }
        };
        Ok((
            InstrSeq::gather(vec![load_arg, load]),
            emit_stmts(e, env, block)?,
            unset,
        ))
    })
}

fn emit_awaitall_multi(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    el: &[(Option<tast::Lid>, tast::Expr)],
    block: &tast::Block,
) -> Result {
    scope::with_unnamed_locals(e, |e| {
        let load_args = InstrSeq::gather(
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
            locals
                .iter()
                .rev()
                .map(|l| instr::popl(l.clone()))
                .collect(),
        );
        let unset_locals =
            InstrSeq::gather(locals.iter().map(|l| instr::unsetl(l.clone())).collect());
        let unpack = InstrSeq::gather(
            locals
                .iter()
                .map(|l| {
                    let label_done = e.label_gen_mut().next_regular();
                    InstrSeq::gather(vec![
                        instr::pushl(l.clone()),
                        instr::dup(),
                        instr::istypec(IstypeOp::OpNull),
                        instr::jmpnz(label_done.clone()),
                        instr::whresult(),
                        instr::label(label_done),
                        instr::popl(l.clone()),
                    ])
                })
                .collect(),
        );
        let await_all = InstrSeq::gather(vec![instr::awaitall_list(locals), instr::popc()]);
        let block_instrs = emit_stmts(e, env, block)?;
        Ok((
            // before
            InstrSeq::gather(vec![load_args, init_locals]),
            // inner
            InstrSeq::gather(vec![emit_pos(pos), await_all, unpack, block_instrs]),
            // after
            unset_locals,
        ))
    })
}

fn emit_using(e: &mut Emitter, env: &mut Env, _pos: &Pos, using: &tast::UsingStmt) -> Result {
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
                        local::Type::Named(local_id::get_name(&id).into()),
                        InstrSeq::gather(vec![
                            emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                            emit_pos(&block_pos),
                            instr::popc(),
                        ]),
                    ),
                    _ => {
                        let l = e.local_gen_mut().get_unnamed();
                        (
                            l.clone(),
                            InstrSeq::gather(vec![
                                emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                                instr::setl(l),
                                instr::popc(),
                            ]),
                        )
                    }
                },
                tast::Expr_::Lvar(lid) => (
                    local::Type::Named(local_id::get_name(&lid.1).into()),
                    InstrSeq::gather(vec![
                        emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                        emit_pos(&block_pos),
                        instr::popc(),
                    ]),
                ),
                _ => {
                    let l = e.local_gen_mut().get_unnamed();
                    (
                        l.clone(),
                        InstrSeq::gather(vec![
                            emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                            instr::setl(l),
                            instr::popc(),
                        ]),
                    )
                }
            };
            let finally_start = e.label_gen_mut().next_regular();
            let finally_end = e.label_gen_mut().next_regular();
            let body = env.do_in_using_body(e, finally_start.clone(), &using.block, emit_block)?;
            let jump_instrs = tfr::JumpInstructions::collect(&body, &mut env.jump_targets_gen);
            let jump_instrs_is_empty = jump_instrs.is_empty();
            let finally_epilogue = tfr::emit_finally_epilogue(
                e,
                env,
                &using.exprs.1[0].0,
                jump_instrs,
                finally_end.clone(),
            )?;
            let try_instrs = if jump_instrs_is_empty {
                body
            } else {
                tfr::cleanup_try_body(&body)
            };

            let emit_finally = |
                e: &mut Emitter,
                local: local::Type,
                has_await: bool,
                is_block_scoped: bool,
            | -> InstrSeq {
                let (epilogue, async_eager_label) = if has_await {
                    let after_await = e.label_gen_mut().next_regular();
                    (
                        InstrSeq::gather(vec![
                            instr::await_(),
                            instr::label(after_await.clone()),
                            instr::popc(),
                        ]),
                        Some(after_await),
                    )
                } else {
                    (instr::popc(), None)
                };
                let fn_name = hhbc_id::method::from_raw_string(if has_await {
                    "__disposeAsync"
                } else {
                    "__dispose"
                });
                InstrSeq::gather(vec![
                    instr::cgetl(local.clone()),
                    instr::nulluninit(),
                    instr::fcallobjmethodd(
                        FcallArgs::new(
                            FcallFlags::empty(),
                            1,
                            vec![],
                            async_eager_label,
                            0,
                            env.call_context.clone(),
                        ),
                        fn_name,
                        ObjNullFlavor::NullThrows,
                    ),
                    epilogue,
                    if is_block_scoped {
                        instr::unsetl(local)
                    } else {
                        instr::empty()
                    },
                ])
            };
            let exn_local = e.local_gen_mut().get_unnamed();
            let middle = if is_empty_block(&using.block) {
                instr::empty()
            } else {
                let finally_instrs =
                    emit_finally(e, local.clone(), using.has_await, using.is_block_scoped);
                let catch_instrs = InstrSeq::gather(vec![
                    emit_pos(&block_pos),
                    make_finally_catch(e, exn_local, finally_instrs),
                    emit_pos(&using.exprs.1[0].0),
                ]);
                InstrSeq::create_try_catch(e.label_gen_mut(), None, true, try_instrs, catch_instrs)
            };
            Ok(InstrSeq::gather(vec![
                preamble,
                middle,
                instr::label(finally_start),
                emit_finally(e, local, using.has_await, using.is_block_scoped),
                finally_epilogue,
                instr::label(finally_end),
            ]))
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

fn emit_switch(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    scrutinee_expr: &tast::Expr,
    cl: &Vec<tast::Case>,
) -> Result {
    let (instr_init, instr_free) = if scrutinee_expr.1.is_lvar() {
        (instr::empty(), instr::empty())
    } else {
        (emit_expr::emit_expr(e, env, scrutinee_expr)?, instr::popc())
    };
    let break_label = e.label_gen_mut().next_regular();
    let has_default = cl.iter().any(|c| c.is_default());

    let emit_cases = |
        env: &mut Env,
        e: &mut Emitter,
        cases: &[tast::Case],
    | -> Result<(InstrSeq, InstrSeq, Label)> {
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
                    .into_iter()
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
                                InstrSeq::gather(vec![
                                    instr::label(l.clone()),
                                    emit_block(env, e, block)?,
                                    emit_break(e, env, &Pos::make_none()),
                                ]),
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
                        InstrSeq::gather(vec![
                            instr::label(l.clone()),
                            emit_pos_then(pos, instr::throw_non_exhaustive_switch()),
                        ]),
                        (None, Some(l)),
                    ))
                };
                let (case_body_instrs, case_exprs_and_default_labels): (Vec<InstrSeq>, Vec<_>) =
                    res.into_iter().unzip();
                let (case_exprs, default_labels): (Vec<Option<(&tast::Expr, Label)>>, Vec<_>) =
                    case_exprs_and_default_labels.into_iter().unzip();

                let default_label = match default_labels
                    .iter()
                    .filter_map(|lopt| lopt.as_ref())
                    .collect::<Vec<_>>()
                    .as_slice()
                {
                    [] => break_label.clone(),
                    [l] => (*l).clone(),
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
                    InstrSeq::gather(case_expr_instrs),
                    InstrSeq::gather(case_body_instrs),
                    default_label,
                ))
            }
        }
    };

    let (case_expr_instrs, case_body_instrs, default_label) =
        env.do_in_switch_body(e, break_label.clone(), cl, emit_cases)?;
    Ok(InstrSeq::gather(vec![
        instr_init,
        case_expr_instrs,
        instr_free,
        instr::jmp(default_label),
        case_body_instrs,
        instr::label(break_label),
    ]))
}

fn is_empty_block(b: &tast::Block) -> bool {
    b.iter().all(|s| s.1.is_noop())
}

fn emit_try_catch_finally(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    r#try: &tast::Block,
    catch: &[tast::Catch],
    finally: &tast::Block,
) -> Result {
    let is_try_block_empty = false;
    let emit_try_block = |env: &mut Env, e: &mut Emitter, finally_start: &Label| {
        env.do_in_try_catch_body(e, finally_start.clone(), r#try, catch, |env, e, t, c| {
            emit_try_catch(e, env, pos, t, c)
        })
    };
    e.local_scope(|e| emit_try_finally_(e, env, pos, emit_try_block, finally, is_try_block_empty))
}

fn emit_try_finally(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    try_block: &tast::Block,
    finally_block: &tast::Block,
) -> Result {
    let is_try_block_empty = is_empty_block(try_block);
    let emit_try_block = |env: &mut Env, e: &mut Emitter, finally_start: &Label| {
        env.do_in_try_body(e, finally_start.clone(), try_block, emit_block)
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

fn emit_try_finally_<E: Fn(&mut Env, &mut Emitter, &Label) -> Result>(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    emit_try_block: E,
    finally_block: &tast::Block,
    is_try_block_empty: bool,
) -> Result {
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

    let in_try = env.flags.contains(env::Flags::IN_TRY);
    env.flags.set(env::Flags::IN_TRY, true);
    let try_body_result = emit_try_block(env, e, &finally_start);
    env.flags.set(env::Flags::IN_TRY, in_try);

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
    let mut finally_body_for_catch = finally_body.clone();
    label_rewriter::clone_with_fresh_regular_labels(e, &mut finally_body_for_catch);

    //  (3) Finally epilogue
    let finally_epilogue =
        tfr::emit_finally_epilogue(e, env, pos, jump_instrs, finally_end.clone())?;

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
        tfr::cleanup_try_body(&try_body)
    };
    let catch_instrs = InstrSeq::gather(vec![
        emit_pos(&enclosing_span),
        make_finally_catch(e, exn_local, finally_body_for_catch),
    ]);
    let middle =
        InstrSeq::create_try_catch(e.label_gen_mut(), None, true, try_instrs, catch_instrs);

    // Putting it all together
    Ok(InstrSeq::gather(vec![
        middle,
        instr::label(finally_start),
        emit_pos(pos),
        finally_body,
        finally_epilogue,
        instr::label(finally_end),
    ]))
}

fn make_finally_catch(e: &mut Emitter, exn_local: local::Type, finally_body: InstrSeq) -> InstrSeq {
    let l2 = instr::unsetl(e.local_gen_mut().get_retval().clone());
    let l1 = instr::unsetl(e.local_gen_mut().get_label().clone());
    InstrSeq::gather(vec![
        instr::popl(exn_local.clone()),
        l1,
        l2,
        InstrSeq::create_try_catch(
            e.label_gen_mut(),
            None,
            false,
            finally_body,
            InstrSeq::gather(vec![instr::pushl(exn_local.clone()), instr::chain_faults()]),
        ),
        instr::pushl(exn_local),
        instr::throw(),
    ])
}

fn emit_try_catch(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    try_block: &tast::Block,
    catch_list: &[tast::Catch],
) -> Result {
    e.local_scope(|e| emit_try_catch_(e, env, pos, try_block, catch_list))
}

fn emit_try_catch_(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    try_block: &tast::Block,
    catch_list: &[tast::Catch],
) -> Result {
    if is_empty_block(&try_block) {
        return Ok(instr::empty());
    };
    let end_label = e.label_gen_mut().next_regular();

    let catch_instrs = InstrSeq::gather(
        catch_list
            .iter()
            .map(|catch| emit_catch(e, env, pos, &end_label, catch))
            .collect::<Result<Vec<_>>>()?,
    );
    let in_try = env.flags.contains(env::Flags::IN_TRY);
    env.flags.set(env::Flags::IN_TRY, true);
    let try_body = emit_stmts(e, env, try_block);
    env.flags.set(env::Flags::IN_TRY, in_try);

    let try_instrs = InstrSeq::gather(vec![try_body?, emit_pos(pos)]);
    Ok(InstrSeq::create_try_catch(
        e.label_gen_mut(),
        Some(end_label.clone()),
        false,
        try_instrs,
        catch_instrs,
    ))
}

fn emit_catch(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    end_label: &Label,
    catch: &tast::Catch,
) -> Result {
    // Note that this is a "regular" label; we're not going to branch to
    // it directly in the event of an exception.
    let next_catch = e.label_gen_mut().next_regular();
    let id = hhbc_id::class::Type::from_ast_name_and_mangle(&(catch.0).1);
    Ok(InstrSeq::gather(vec![
        instr::dup(),
        instr::instanceofd(id),
        instr::jmpz(next_catch.clone()),
        instr::setl(local::Type::Named((&((catch.1).1).1).to_string())),
        instr::popc(),
        emit_stmts(e, env, &catch.2)?,
        emit_pos(pos),
        instr::jmp(end_label.clone()),
        instr::label(next_catch),
    ]))
}

fn emit_foreach(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    collection: &tast::Expr,
    iterator: &tast::AsExpr,
    block: &tast::Block,
) -> Result {
    use tast::AsExpr as A;
    e.local_scope(|e| match iterator {
        A::AsV(_) | A::AsKv(_, _) => emit_foreach_(e, env, pos, collection, iterator, block),
        A::AwaitAsV(pos, _) | A::AwaitAsKv(pos, _, _) => {
            emit_foreach_await(e, env, pos, collection, iterator, block)
        }
    })
}

fn emit_foreach_(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    collection: &tast::Expr,
    iterator: &tast::AsExpr,
    block: &tast::Block,
) -> Result {
    let collection_instrs = emit_expr::emit_expr(e, env, collection)?;
    scope::with_unnamed_locals_and_iterators(e, |e| {
        let iter_id = e.iterator_mut().get();
        let loop_break_label = e.label_gen_mut().next_regular();
        let loop_continue_label = e.label_gen_mut().next_regular();
        let loop_head_label = e.label_gen_mut().next_regular();
        let (key_id, val_id, preamble) = emit_iterator_key_value_storage(e, env, iterator)?;
        let iter_args = IterArgs {
            iter_id: iter_id.clone(),
            key_id,
            val_id,
        };
        let body = env.do_in_loop_body(
            e,
            loop_break_label.clone(),
            loop_continue_label.clone(),
            Some(iter_id),
            block,
            emit_block,
        )?;
        let iter_init = InstrSeq::gather(vec![
            collection_instrs,
            emit_pos(&collection.0),
            instr::iterinit(iter_args.clone(), loop_break_label.clone()),
        ]);
        let iterate = InstrSeq::gather(vec![
            instr::label(loop_head_label.clone()),
            preamble,
            body,
            instr::label(loop_continue_label),
            emit_pos(pos),
            instr::iternext(iter_args.clone(), loop_head_label),
        ]);
        let iter_done = instr::label(loop_break_label);
        Ok((iter_init, iterate, iter_done))
    })
}

fn emit_foreach_await(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    collection: &tast::Expr,
    iterator: &tast::AsExpr,
    block: &tast::Block,
) -> Result {
    let instr_collection = emit_expr::emit_expr(e, env, collection)?;
    scope::with_unnamed_local(e, |e, iter_temp_local| {
        let input_is_async_iterator_label = e.label_gen_mut().next_regular();
        let next_label = e.label_gen_mut().next_regular();
        let exit_label = e.label_gen_mut().next_regular();
        let pop_and_exit_label = e.label_gen_mut().next_regular();
        let async_eager_label = e.label_gen_mut().next_regular();
        let next_meth = hhbc_id::method::from_raw_string("next");
        let iter_init = InstrSeq::gather(vec![
            instr_collection,
            instr::dup(),
            instr::instanceofd(hhbc_id::class::from_raw_string("HH\\AsyncIterator")),
            instr::jmpnz(input_is_async_iterator_label.clone()),
            emit_fatal::emit_fatal_runtime(
                pos,
                "Unable to iterate non-AsyncIterator asynchronously",
            ),
            instr::label(input_is_async_iterator_label),
            instr::popl(iter_temp_local.clone()),
        ]);
        let loop_body_instr = env.do_in_loop_body(
            e,
            exit_label.clone(),
            next_label.clone(),
            None,
            block,
            emit_block,
        )?;
        let iterate = InstrSeq::gather(vec![
            instr::label(next_label.clone()),
            instr::cgetl(iter_temp_local.clone()),
            instr::nulluninit(),
            instr::fcallobjmethodd(
                FcallArgs::new(
                    FcallFlags::empty(),
                    1,
                    vec![],
                    Some(async_eager_label.clone()),
                    0,
                    None,
                ),
                next_meth,
                ObjNullFlavor::NullThrows,
            ),
            instr::await_(),
            instr::label(async_eager_label),
            instr::dup(),
            instr::istypec(IstypeOp::OpNull),
            instr::jmpnz(pop_and_exit_label.clone()),
            emit_foreach_await_key_value_storage(e, env, iterator)?,
            loop_body_instr,
            emit_pos(pos),
            instr::jmp(next_label),
            instr::label(pop_and_exit_label),
            instr::popc(),
            instr::label(exit_label),
        ]);
        let iter_done = instr::unsetl(iter_temp_local);
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
fn emit_iterator_key_value_storage(
    e: &mut Emitter,
    env: &mut Env,
    iterator: &tast::AsExpr,
) -> Result<(Option<local::Type>, local::Type, InstrSeq)> {
    use tast::AsExpr as A;
    fn get_id_of_simple_lvar_opt(lvar: &tast::Expr_) -> Result<Option<String>> {
        if let Some(tast::Lid(pos, id)) = lvar.as_lvar() {
            let name = local_id::get_name(&id);
            if name == special_idents::THIS {
                return Err(emit_fatal::raise_fatal_parse(
                    &pos,
                    "Cannot re-assign $this",
                ));
            } else if !(superglobals::is_superglobal(&name)) {
                return Ok(Some(name.into()));
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
                    Some(local::Type::Named(key_id)),
                    local::Type::Named(val_id),
                    instr::empty(),
                ),
                _ => {
                    let key_local = e.local_gen_mut().get_unnamed();
                    let val_local = e.local_gen_mut().get_unnamed();
                    let (mut key_preamble, key_load) =
                        emit_iterator_lvalue_storage(e, env, k, key_local.clone())?;
                    let (mut val_preamble, val_load) =
                        emit_iterator_lvalue_storage(e, env, v, val_local.clone())?;
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
                        InstrSeq::gather(vec![
                            InstrSeq::gather(val_preamble),
                            InstrSeq::gather(val_load),
                            InstrSeq::gather(key_preamble),
                            InstrSeq::gather(key_load),
                        ]),
                    )
                }
            },
        ),
        A::AsV(v) => Ok(match get_id_of_simple_lvar_opt(&v.1)? {
            Some(val_id) => (None, local::Type::Named(val_id), instr::empty()),
            None => {
                let val_local = e.local_gen_mut().get_unnamed();
                let (val_preamble, val_load) =
                    emit_iterator_lvalue_storage(e, env, v, val_local.clone())?;
                (
                    None,
                    val_local,
                    InstrSeq::gather(vec![
                        InstrSeq::gather(val_preamble),
                        InstrSeq::gather(val_load),
                    ]),
                )
            }
        }),
        _ => Err(Unrecoverable(
            "emit_iterator_key_value_storage with iterator using await".into(),
        )),
    }
}

fn emit_iterator_lvalue_storage(
    e: &mut Emitter,
    env: &mut Env,
    lvalue: &tast::Expr,
    local: local::Type,
) -> Result<(Vec<InstrSeq>, Vec<InstrSeq>)> {
    match &lvalue.1 {
        tast::Expr_::Call(_) => Err(emit_fatal::raise_fatal_parse(
            &lvalue.0,
            "Can't use return value in write context",
        )),
        tast::Expr_::List(es) => {
            let (preamble, load_values) = emit_load_list_elements(
                e,
                env,
                vec![instr::basel(local.clone(), MemberOpMode::Warn)],
                es,
            )?;
            let load_values = vec![
                InstrSeq::gather(load_values.into_iter().rev().collect()),
                instr::unsetl(local),
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
                instr::cgetl(local.clone()),
                1,
                false,
            )?;
            Ok((
                vec![lhs],
                vec![rhs, set_op, instr::popc(), instr::unsetl(local)],
            ))
        }
    }
}

fn emit_load_list_elements(
    e: &mut Emitter,
    env: &mut Env,
    path: Vec<InstrSeq>,
    es: &[tast::Expr],
) -> Result<(Vec<InstrSeq>, Vec<InstrSeq>)> {
    let (preamble, load_value): (Vec<Vec<InstrSeq>>, Vec<Vec<InstrSeq>>) = es
        .iter()
        .enumerate()
        .map(|(i, x)| emit_load_list_element(e, env, path.clone(), i, x))
        .collect::<Result<Vec<_>>>()?
        .into_iter()
        .unzip();
    Ok((
        preamble.into_iter().flatten().collect(),
        load_value.into_iter().flatten().collect(),
    ))
}

fn emit_load_list_element(
    e: &mut Emitter,
    env: &mut Env,
    mut path: Vec<InstrSeq>,
    i: usize,
    elem: &tast::Expr,
) -> Result<(Vec<InstrSeq>, Vec<InstrSeq>)> {
    let query_value = |path| {
        InstrSeq::gather(vec![
            InstrSeq::gather(path),
            instr::querym(0, QueryOp::CGet, MemberKey::EI(i as i64, ReadOnlyOp::Any)),
        ])
    };
    Ok(match &elem.1 {
        tast::Expr_::Lvar(lid) => {
            let load_value = InstrSeq::gather(vec![
                query_value(path),
                instr::setl(local::Type::Named(local_id::get_name(&lid.1).into())),
                instr::popc(),
            ]);
            (vec![], vec![load_value])
        }
        tast::Expr_::List(es) => {
            let instr_dim =
                instr::dim(MemberOpMode::Warn, MemberKey::EI(i as i64, ReadOnlyOp::Any));
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
            let load_value = InstrSeq::gather(vec![set_instrs, instr::popc()]);
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
fn emit_foreach_await_key_value_storage(
    e: &mut Emitter,
    env: &mut Env,
    iterator: &tast::AsExpr,
) -> Result {
    use tast::AsExpr as A;
    match iterator {
        A::AwaitAsKv(_, k, v) | A::AsKv(k, v) => Ok(InstrSeq::gather(vec![
            emit_foreach_await_lvalue_storage(e, env, k, &[0], true)?,
            emit_foreach_await_lvalue_storage(e, env, v, &[1], false)?,
        ])),
        A::AwaitAsV(_, v) | A::AsV(v) => emit_foreach_await_lvalue_storage(e, env, v, &[1], false),
    }
}

// Emit code for either the key or value l-value operation in foreach await.
// `indices` is the initial prefix of the array indices ([0] for key or [1] for
// value) that is prepended onto the indices needed for list destructuring
//
// TODO: we don't need unnamed local if the target is a local
fn emit_foreach_await_lvalue_storage(
    e: &mut Emitter,
    env: &mut Env,
    lvalue: &tast::Expr,
    indices: &[isize],
    keep_on_stack: bool,
) -> Result {
    scope::with_unnamed_local(e, |e, local| {
        Ok((
            instr::popl(local.clone()),
            emit_expr::emit_lval_op_list(e, env, &lvalue.0, Some(&local), indices, lvalue, false)?
                .into(),
            if keep_on_stack {
                instr::pushl(local)
            } else {
                instr::unsetl(local)
            },
        ))
    })
}

fn emit_stmts(e: &mut Emitter, env: &mut Env, stl: &[tast::Stmt]) -> Result {
    Ok(InstrSeq::gather(
        stl.iter()
            .map(|s| emit_stmt(e, env, s))
            .collect::<Result<Vec<_>>>()?,
    ))
}

fn emit_block(env: &mut Env, emitter: &mut Emitter, block: &tast::Block) -> Result {
    emit_stmts(emitter, env, block.as_slice())
}

fn emit_do(e: &mut Emitter, env: &mut Env, body: &tast::Block, cond: &tast::Expr) -> Result {
    let break_label = e.label_gen_mut().next_regular();
    let cont_label = e.label_gen_mut().next_regular();
    let start_label = e.label_gen_mut().next_regular();
    let jmpnz_instr = emit_expr::emit_jmpnz(e, env, cond, &start_label)?.instrs;
    Ok(InstrSeq::gather(vec![
        instr::label(start_label.clone()),
        env.do_in_loop_body(
            e,
            break_label.clone(),
            cont_label.clone(),
            None,
            body,
            emit_block,
        )?,
        instr::label(cont_label),
        jmpnz_instr,
        instr::label(break_label),
    ]))
}

fn emit_while(e: &mut Emitter, env: &mut Env, cond: &tast::Expr, body: &tast::Block) -> Result {
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
    let i3 = emit_expr::emit_jmpnz(e, env, cond, &start_label)?.instrs;
    let i2 = env.do_in_loop_body(
        e,
        break_label.clone(),
        cont_label.clone(),
        None,
        body,
        emit_block,
    )?;
    let i1 = emit_expr::emit_jmpz(e, env, cond, &break_label)?.instrs;
    Ok(InstrSeq::gather(vec![
        i1,
        instr::label(start_label.clone()),
        i2,
        instr::label(cont_label),
        i3,
        instr::label(break_label),
    ]))
}

fn emit_for(
    e: &mut Emitter,
    env: &mut Env,
    e1: &Vec<tast::Expr>,
    e2: &Option<tast::Expr>,
    e3: &Vec<tast::Expr>,
    body: &tast::Block,
) -> Result {
    let break_label = e.label_gen_mut().next_regular();
    let cont_label = e.label_gen_mut().next_regular();
    let start_label = e.label_gen_mut().next_regular();
    fn emit_cond(
        emitter: &mut Emitter,
        env: &mut Env,
        jmpz: bool,
        label: &Label,
        cond: &Option<tast::Expr>,
    ) -> Result {
        Ok(match cond {
            None => {
                if jmpz {
                    instr::empty()
                } else {
                    instr::jmp(label.clone())
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
    let i5 = emit_cond(e, env, false, &start_label, e2)?;
    let i4 = emit_expr::emit_ignored_exprs(e, env, &Pos::make_none(), e3)?;
    let i3 = env.do_in_loop_body(
        e,
        break_label.clone(),
        cont_label.clone(),
        None,
        body,
        emit_block,
    )?;
    let i2 = emit_cond(e, env, true, &break_label, e2)?;
    let i1 = emit_expr::emit_ignored_exprs(e, env, &Pos::make_none(), e1)?;
    Ok(InstrSeq::gather(vec![
        i1,
        i2,
        instr::label(start_label),
        i3,
        instr::label(cont_label),
        i4,
        i5,
        instr::label(break_label),
    ]))
}

pub fn emit_dropthrough_return(e: &mut Emitter, env: &mut Env) -> Result {
    match e.emit_statement_state().default_dropthrough.as_ref() {
        Some(instrs) => Ok(instrs.clone()),
        None => {
            let ret = emit_return(e, env)?;
            let state = e.emit_statement_state();
            Ok(emit_pos_then(
                &(state.function_pos.last_char()),
                InstrSeq::gather(vec![state.default_return_value.clone(), ret]),
            ))
        }
    }
}

pub fn emit_final_stmt(e: &mut Emitter, env: &mut Env, stmt: &tast::Stmt) -> Result {
    match &stmt.1 {
        a::Stmt_::Throw(_) | a::Stmt_::Return(_) | a::Stmt_::YieldBreak => emit_stmt(e, env, stmt),
        a::Stmt_::Block(stmts) => emit_final_stmts(e, env, stmts),
        _ => {
            let ret = emit_dropthrough_return(e, env)?;
            Ok(InstrSeq::gather(vec![emit_stmt(e, env, stmt)?, ret]))
        }
    }
}

pub fn emit_final_stmts(e: &mut Emitter, env: &mut Env, block: &[tast::Stmt]) -> Result {
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
            Ok(InstrSeq::gather(ret))
        }
    }
}

pub fn emit_markup(
    e: &mut Emitter,
    env: &mut Env,
    (_, s): &tast::Pstring,
    check_for_hashbang: bool,
) -> Result {
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
            Ok(instr::empty())
        } else {
            emit_ignored_call_expr(
                fname,
                tast::Expr(Pos::make_none(), tast::Expr_::mk_string(expr_str.into())),
            )
        }
    };
    let markup = if s.is_empty() {
        instr::empty()
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

fn emit_break(e: &mut Emitter, env: &mut Env, pos: &Pos) -> InstrSeq {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::IS_BREAK, env, pos, 1)
}

fn emit_continue(e: &mut Emitter, env: &mut Env, pos: &Pos) -> InstrSeq {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::empty(), env, pos, 1)
}

fn emit_await_assignment(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    lval: &tast::Expr,
    r: &tast::Expr,
) -> Result {
    match lval.1.as_lvar() {
        Some(tast::Lid(_, id)) if !emit_expr::is_local_this(env, &id) => {
            Ok(InstrSeq::gather(vec![
                emit_expr::emit_await(e, env, pos, r)?,
                emit_pos(pos),
                instr::popl(emit_expr::get_local(e, env, pos, local_id::get_name(&id))?),
            ]))
        }
        _ => {
            let awaited_instrs = emit_await(e, env, pos, r)?;
            scope::with_unnamed_local(e, |e, temp| {
                let rhs_instrs = instr::pushl(temp.clone());
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
                    InstrSeq::gather(vec![awaited_instrs, instr::popl(temp)]),
                    lhs,
                    InstrSeq::gather(vec![rhs, setop, instr::popc()]),
                ))
            })
        }
    }
}

fn emit_if(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    condition: &tast::Expr,
    consequence: &[tast::Stmt],
    alternative: &[tast::Stmt],
) -> Result {
    if alternative.is_empty() || (alternative.len() == 1 && alternative[0].1.is_noop()) {
        let done_label = e.label_gen_mut().next_regular();
        let consequence_instr = emit_stmts(e, env, consequence)?;
        Ok(InstrSeq::gather(vec![
            emit_expr::emit_jmpz(e, env, condition, &done_label)?.instrs,
            consequence_instr,
            instr::label(done_label),
        ]))
    } else {
        let alternative_label = e.label_gen_mut().next_regular();
        let done_label = e.label_gen_mut().next_regular();
        let consequence_instr = emit_stmts(e, env, consequence)?;
        let alternative_instr = emit_stmts(e, env, alternative)?;
        Ok(InstrSeq::gather(vec![
            emit_expr::emit_jmpz(e, env, condition, &alternative_label)?.instrs,
            consequence_instr,
            emit_pos(pos),
            instr::jmp(done_label.clone()),
            instr::label(alternative_label),
            alternative_instr,
            instr::label(done_label),
        ]))
    }
}
