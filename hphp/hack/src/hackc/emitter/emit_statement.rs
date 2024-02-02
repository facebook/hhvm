// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use emit_pos::emit_pos;
use emit_pos::emit_pos_then;
use env::emitter::Emitter;
use env::Env;
use error::Error;
use error::Result;
use hack_macros::hack_expr;
use hhbc::FCallArgs;
use hhbc::FCallArgsFlags;
use hhbc::IsTypeOp;
use hhbc::IterArgs;
use hhbc::Label;
use hhbc::Local;
use hhbc::MOpMode;
use hhbc::MemberKey;
use hhbc::QueryMOp;
use hhbc::ReadonlyOp;
use hhbc::SetRangeOp;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use lazy_static::lazy_static;
use naming_special_names_rust::special_idents;
use naming_special_names_rust::superglobals;
use oxidized::aast as a;
use oxidized::ast;
use oxidized::ast_defs;
use oxidized::local_id;
use oxidized::pos::Pos;
use regex::Regex;
use statement_state::StatementState;

use crate::emit_expression::emit_await;
use crate::emit_expression::emit_expr;
use crate::emit_expression::LValOp;
use crate::emit_expression::SetRange;
use crate::emit_expression::{self as emit_expr};
use crate::emit_fatal;
use crate::try_finally_rewriter as tfr;

// Expose a mutable ref to state for emit_body so that it can set it appropriately
pub(crate) fn set_state<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    state: StatementState<'arena>,
) {
    *e.statement_state_mut() = state;
}

// Wrapper functions

fn emit_return<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
) -> Result<InstrSeq<'arena>> {
    tfr::emit_return(e, false, env)
}

fn is_readonly_expr(e: &ast::Expr) -> bool {
    match &e.2 {
        ast::Expr_::ReadonlyExpr(_) => true,
        _ => false,
    }
}

fn set_bytes_kind(name: &str) -> Option<SetRange> {
    lazy_static! {
        static ref RE: Regex =
            Regex::new(r#"(?i)^HH\\set_bytes(_rev)?_([a-z0-9]+)(_vec)?$"#).unwrap();
    }
    RE.captures(name).and_then(|groups| {
        let op = if groups.get(1).is_some() {
            // == _rev
            SetRangeOp::Reverse
        } else {
            SetRangeOp::Forward
        };
        let kind = groups.get(2).unwrap().as_str();
        let vec = groups.get(3).is_some(); // == _vec
        if kind == "string" && !vec {
            Some(SetRange {
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
            Some(SetRange { size, vec, op })
        }
    })
}

fn is_single_await_all_block(lids: &[a::Lid], stmts: &[ast::Stmt]) -> bool {
    matches!((lids, stmts),
        ([lid], [a::Stmt(_, a::Stmt_::Awaitall(box (inits, _)))]) if inits.len() == 1 && &inits[0].0 == lid
    )
}

#[allow(clippy::todo)]
pub fn emit_stmt<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    stmt: &ast::Stmt,
) -> Result<InstrSeq<'arena>> {
    let pos = &stmt.0;
    match &stmt.1 {
        a::Stmt_::YieldBreak => emit_yieldbreak(e, env, pos),
        a::Stmt_::Expr(e_) => match &e_.2 {
            a::Expr_::Await(a) => emit_await_(e, env, e_, pos, a),
            a::Expr_::Call(c) => emit_call(e, env, e_, pos, c),
            a::Expr_::Binop(bop) => emit_binop(e, env, e_, pos, bop),
            _ => emit_expr::emit_ignored_expr(e, env, pos, e_),
        },
        a::Stmt_::Return(r_opt) => emit_return_(e, env, (**r_opt).as_ref(), pos),
        a::Stmt_::Block(box (Some(lids), a::Block(block)))
            if is_single_await_all_block(lids, block) =>
        {
            emit_block_awaitall_single(e, env, block)
        }
        a::Stmt_::Block(box (lids, b)) => emit_block_with_temps(env, e, lids, b),
        a::Stmt_::If(f) => emit_if(e, env, pos, &f.0, &f.1, &f.2),
        a::Stmt_::While(x) => emit_while(e, env, &x.0, &x.1),
        a::Stmt_::Using(x) => emit_using(e, env, x),
        a::Stmt_::Break => Ok(emit_break(e, env, pos)),
        a::Stmt_::Continue => Ok(emit_continue(e, env, pos)),
        a::Stmt_::Do(x) => emit_do(e, env, &x.0, &x.1),
        a::Stmt_::For(x) => emit_for(e, env, &x.0, x.1.as_ref(), &x.2, &x.3),
        a::Stmt_::Throw(x) => emit_throw(e, env, x, pos),
        a::Stmt_::Try(x) => emit_try(e, env, x, pos),
        a::Stmt_::Switch(x) => emit_switch(e, env, pos, &x.0, &x.1, &x.2),
        a::Stmt_::Foreach(x) => emit_foreach(e, env, pos, &x.0, &x.1, &x.2),
        a::Stmt_::Awaitall(x) => emit_awaitall(e, env, pos, &x.0, &x.1),
        // The lowerer converts concurrent statements to Awaitall when
        // elaboration is going to generate bytecode.
        a::Stmt_::Concurrent(_) => panic!("Concurrent statement in emit_statement"),
        a::Stmt_::Markup(x) => emit_markup(e, env, x, false),
        a::Stmt_::Fallthrough | a::Stmt_::Noop => Ok(instr::empty()),
        a::Stmt_::DeclareLocal(x) => emit_declare_local(e, env, pos, &x.0, &x.2),
        a::Stmt_::Match(..) => todo!("TODO(jakebailey): match statements"),
    }
}

fn emit_await_<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    e_: &ast::Expr,
    _pos: &Pos,
    a: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(vec![
        emit_await(e, env, &e_.1, a)?,
        instr::pop_c(),
    ]))
}

fn emit_binop<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    e_: &ast::Expr,
    pos: &Pos,
    bop: &ast::Binop,
) -> Result<InstrSeq<'arena>> {
    if let ast::Binop {
        bop: ast_defs::Bop::Eq(None),
        lhs,
        rhs,
    } = bop
    {
        if let Some(e_await) = rhs.2.as_await() {
            let await_pos = &rhs.1;
            if let Some(l) = lhs.2.as_list() {
                let awaited_instrs = emit_await(e, env, await_pos, e_await)?;
                let has_elements = l.iter().any(|e| !e.2.is_omitted());
                if has_elements {
                    scope::with_unnamed_local(e, |e, temp| {
                        let (init, assign) = emit_expr::emit_lval_op_list(
                            e,
                            env,
                            pos,
                            Some(&temp),
                            &[],
                            lhs,
                            false,
                            is_readonly_expr(rhs),
                        )?;
                        Ok((
                            InstrSeq::gather(vec![awaited_instrs, instr::pop_l(temp)]),
                            InstrSeq::gather(vec![init, assign]),
                            instr::empty(),
                        ))
                    })
                } else {
                    Ok(InstrSeq::gather(vec![awaited_instrs, instr::pop_c()]))
                }
            } else {
                emit_await_assignment(e, env, await_pos, lhs, e_await)
            }
        } else {
            emit_expr::emit_ignored_expr(e, env, pos, e_)
        }
    } else {
        emit_expr::emit_ignored_expr(e, env, pos, e_)
    }
}

fn emit_yieldbreak<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    _pos: &Pos,
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(vec![instr::null(), emit_return(e, env)?]))
}

fn emit_try<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    x: &(ast::Block, Vec<ast::Catch>, ast::FinallyBlock),
    pos: &Pos,
) -> Result<InstrSeq<'arena>> {
    let (try_block, catch_list, finally_block) = x;
    if catch_list.is_empty() {
        emit_try_finally(e, env, pos, try_block, finally_block)
    } else if finally_block.is_empty() {
        emit_try_catch(e, env, pos, try_block, &catch_list[..])
    } else {
        emit_try_catch_finally(e, env, pos, try_block, &catch_list[..], finally_block)
    }
}

fn emit_throw<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    x: &ast::Expr,
    pos: &Pos,
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(vec![
        emit_expr::emit_expr(e, env, x)?,
        emit_pos(pos),
        instr::throw(),
    ]))
}

fn emit_return_<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    r_opt: Option<&ast::Expr>,
    pos: &Pos,
) -> Result<InstrSeq<'arena>> {
    match r_opt {
        Some(r) => {
            let ret = emit_return(e, env)?;
            let expr_instr = if let Some(e_) = r.2.as_await() {
                emit_await(e, env, &r.1, e_)?
            } else {
                emit_expr(e, env, r)?
            };
            Ok(InstrSeq::gather(vec![expr_instr, emit_pos(pos), ret]))
        }
        None => Ok(InstrSeq::gather(vec![
            instr::null(),
            emit_pos(pos),
            emit_return(e, env)?,
        ])),
    }
}

fn emit_call<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    e_: &ast::Expr,
    _pos: &Pos,
    c: &ast::CallExpr,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    if let a::CallExpr {
        func: a::Expr(_, _, a::Expr_::Id(sid)),
        args,
        unpacked_arg: None,
        ..
    } = c
    {
        let ft = hhbc::FunctionName::from_ast_name(alloc, &sid.1);
        let fname = ft.unsafe_as_str();
        if fname.eq_ignore_ascii_case("unset") {
            Ok(InstrSeq::gather(
                args.iter()
                    .map(|ex| {
                        emit_expr::emit_unset_expr(e, env, error::expect_normal_paramkind(ex)?)
                    })
                    .collect::<Result<Vec<_>, _>>()?,
            ))
        } else if let Some(kind) = set_bytes_kind(fname) {
            emit_expr::emit_set_range_expr(e, env, &e_.1, fname, kind, args)
        } else {
            emit_expr::emit_ignored_expr(e, env, &e_.1, e_)
        }
    } else {
        emit_expr::emit_ignored_expr(e, env, &e_.1, e_)
    }
}

fn emit_case<'c, 'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    case: &'c ast::Case,
    addbreak: bool,
) -> Result<(InstrSeq<'arena>, (&'c ast::Expr, Label))> {
    let ast::Case(case_expr, body) = case;

    let label = e.label_gen_mut().next_regular();
    let mut res = vec![instr::label(label), emit_block(env, e, body)?];
    if addbreak {
        res.push(emit_break(e, env, &Pos::NONE));
    }
    Ok((InstrSeq::gather(res), (case_expr, label)))
}

fn emit_default_case<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    dfl: &ast::DefaultCase,
) -> Result<(InstrSeq<'arena>, Label)> {
    let ast::DefaultCase(_, body) = dfl;

    let label = e.label_gen_mut().next_regular();
    Ok((
        InstrSeq::gather(vec![instr::label(label), emit_block(env, e, body)?]),
        label,
    ))
}

fn emit_check_case<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    scrutinee_expr: &ast::Expr,
    (case_expr, case_handler_label): (&ast::Expr, Label),
) -> Result<InstrSeq<'arena>> {
    Ok(if scrutinee_expr.2.is_lvar() {
        InstrSeq::gather(vec![
            emit_expr::emit_two_exprs(e, env, &case_expr.1, scrutinee_expr, case_expr)?,
            instr::eq(),
            instr::jmp_nz(case_handler_label),
        ])
    } else {
        let next_case_label = e.label_gen_mut().next_regular();
        InstrSeq::gather(vec![
            instr::dup(),
            emit_expr::emit_expr(e, env, case_expr)?,
            emit_pos(&case_expr.1),
            instr::eq(),
            instr::jmp_z(next_case_label),
            instr::pop_c(),
            instr::jmp(case_handler_label),
            instr::label(next_case_label),
        ])
    })
}

fn emit_awaitall<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    el: &[(ast::Lid, ast::Expr)],
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    match el {
        [] => Ok(instr::empty()),
        [(lvar, expr)] => emit_awaitall_single(e, env, pos, lvar, expr, block),
        _ => emit_awaitall_multi(e, env, pos, el, block),
    }
}

// We don't have to special case this, but we do in order to make it bytecode
// preserving. Here we don't put the rhs of the single await inside of the
// with_unnamed_temps which is what the Block would otherwise cause. We could
// make this more uniform in the future.
fn emit_block_awaitall_single<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    match block {
        [a::Stmt(pos, a::Stmt_::Awaitall(box (init, block)))] => {
            let (lval, expr) = &init[0];
            let lvals = vec![lval.clone()];
            let a::Lid(_, id) = lval;
            let load_arg = emit_expr::emit_await(e, env, pos, expr)?;
            let mut load = instr::nop();
            let body = scope::with_unnamed_temps(e, &lvals, |e| {
                load = instr::pop_l(e.local_gen().get_unnamed_for_tempname(&id.1).clone());
                let body = emit_stmts(e, env, block)?;
                Ok(InstrSeq::gather(vec![body]))
            })?;
            Ok(InstrSeq::gather(vec![load_arg, load, body]))
        }
        _ => panic!(),
    }
}

fn emit_awaitall_single<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    lval: &ast::Lid,
    expr: &ast::Expr,
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let load_arg = emit_expr::emit_await(e, env, pos, expr)?;
    let load = instr::pop_l(e.local_gen().get_unnamed_for_tempname(&lval.1.1).clone());
    let body = emit_stmts(e, env, block)?;
    Ok(InstrSeq::gather(vec![load_arg, load, body]))
}

fn emit_awaitall_multi<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    el: &[(ast::Lid, ast::Expr)],
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let mut instrs = vec![];
    let mut locals = vec![];
    for (lvar, expr) in el.iter() {
        let local = e.local_gen().get_unnamed_for_tempname(&lvar.1.1).clone();
        instrs.push(emit_expr::emit_expr(e, env, expr)?);
        instrs.push(instr::pop_l(local));
        locals.push(local);
    }

    let load_args = InstrSeq::gather(instrs);
    let mut instrs = vec![];
    for l in locals.iter() {
        instrs.push({
            let label_done = e.label_gen_mut().next_regular();
            InstrSeq::gather(vec![
                instr::push_l(*l),
                instr::dup(),
                instr::is_type_c(IsTypeOp::Null),
                instr::jmp_nz(label_done),
                instr::wh_result(),
                instr::label(label_done),
                instr::pop_l(*l),
            ])
        });
    }

    let unpack = InstrSeq::gather(instrs);
    let await_all = InstrSeq::gather(vec![instr::await_all_list(locals), instr::pop_c()]);
    let block_instrs = emit_stmts(e, env, block)?;
    Ok(InstrSeq::gather(vec![
        load_args,
        emit_pos(pos),
        await_all,
        unpack,
        block_instrs,
    ]))
}

fn emit_using<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    using: &ast::UsingStmt,
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
                    ast::Block(vec![ast::Stmt(
                        expr.1.clone(),
                        ast::Stmt_::mk_using(ast::UsingStmt {
                            has_await: using.has_await,
                            is_block_scoped: using.is_block_scoped,
                            exprs: (expr.1.clone(), vec![expr.clone()]),
                            block,
                        }),
                    )])
                })
                .as_slice(),
        )
    } else {
        e.local_scope(|e| {
            let (local, preamble) = match &(using.exprs.1[0].2) {
                ast::Expr_::Binop(binop) => match (&binop.bop, (binop.lhs).2.as_lvar()) {
                    (ast_defs::Bop::Eq(None), Some(ast::Lid(_, id))) => (
                        e.named_local(local_id::get_name(id).into()),
                        InstrSeq::gather(vec![
                            emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                            emit_pos(&block_pos),
                            instr::pop_c(),
                        ]),
                    ),
                    _ => {
                        let l = e.local_gen_mut().get_unnamed();
                        (
                            l,
                            InstrSeq::gather(vec![
                                emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                                instr::set_l(l),
                                instr::pop_c(),
                            ]),
                        )
                    }
                },
                ast::Expr_::Lvar(lid) => (
                    e.named_local(local_id::get_name(&lid.1).into()),
                    InstrSeq::gather(vec![
                        emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                        emit_pos(&block_pos),
                        instr::pop_c(),
                    ]),
                ),
                _ => {
                    let l = e.local_gen_mut().get_unnamed();
                    (
                        l,
                        InstrSeq::gather(vec![
                            emit_expr::emit_expr(e, env, &(using.exprs.1[0]))?,
                            instr::set_l(l),
                            instr::pop_c(),
                        ]),
                    )
                }
            };
            let finally_start = e.label_gen_mut().next_regular();
            let finally_end = e.label_gen_mut().next_regular();
            let body = env.do_in_using_body(e, finally_start, &using.block, emit_block)?;
            let jump_instrs = tfr::JumpInstructions::collect(&body, &mut env.jump_targets_gen);
            let jump_instrs_is_empty = jump_instrs.is_empty();
            let finally_epilogue =
                tfr::emit_finally_epilogue(e, env, &using.exprs.1[0].1, jump_instrs, finally_end)?;
            let try_instrs = if jump_instrs_is_empty {
                body
            } else {
                tfr::cleanup_try_body(body)
            };

            let emit_finally = |e: &mut Emitter<'arena, 'decl>,
                                local: Local,
                                has_await: bool,
                                is_block_scoped: bool|
             -> InstrSeq<'arena> {
                let (epilogue, async_eager_label) = if has_await {
                    let after_await = e.label_gen_mut().next_regular();
                    (
                        InstrSeq::gather(vec![
                            instr::await_(),
                            instr::label(after_await),
                            instr::pop_c(),
                        ]),
                        Some(after_await),
                    )
                } else {
                    (instr::pop_c(), None)
                };
                let fn_name = hhbc::MethodName::from_raw_string(
                    alloc,
                    if has_await {
                        "__disposeAsync"
                    } else {
                        "__dispose"
                    },
                );
                InstrSeq::gather(vec![
                    instr::c_get_l(local),
                    instr::null_uninit(),
                    instr::f_call_obj_method_d(
                        FCallArgs::new(
                            FCallArgsFlags::default(),
                            1,
                            0,
                            vec![],
                            vec![],
                            async_eager_label,
                            env.call_context
                                .as_ref()
                                .map(|s| -> &str { alloc.alloc_str(s.as_ref()) }),
                        ),
                        fn_name,
                    ),
                    epilogue,
                    if is_block_scoped {
                        instr::unset_l(local)
                    } else {
                        instr::empty()
                    },
                ])
            };
            let exn_local = e.local_gen_mut().get_unnamed();
            let middle = if is_empty_block(&using.block) {
                instr::empty()
            } else {
                let finally_instrs = emit_finally(e, local, using.has_await, using.is_block_scoped);
                let catch_instrs = InstrSeq::gather(vec![
                    emit_pos(&block_pos),
                    make_finally_catch(e, exn_local, finally_instrs),
                    emit_pos(&using.exprs.1[0].1),
                ]);
                scope::create_try_catch(e.label_gen_mut(), None, true, try_instrs, catch_instrs)
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

fn block_pos(block: &[ast::Stmt]) -> Result<Pos> {
    if block.iter().all(|b| b.0.is_none()) {
        return Ok(Pos::NONE);
    }
    let mut first = 0;
    let mut last = block.len() - 1;
    loop {
        if !block[first].0.is_none() && !block[last].0.is_none() {
            return Pos::btw(&block[first].0, &block[last].0).map_err(Error::unrecoverable);
        }
        if block[first].0.is_none() {
            first += 1;
        }
        if block[last].0.is_none() {
            last -= 1;
        }
    }
}

fn emit_cases<'a, 'arena, 'decl>(
    env: &mut Env<'a, 'arena>,
    e: &mut Emitter<'arena, 'decl>,
    pos: &Pos,
    scrutinee_expr: &ast::Expr,
    cases: &[ast::Case],
    dfl: &Option<ast::DefaultCase>,
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, Label)> {
    let should_gen_break = |i: usize| -> bool { dfl.is_none() && (i + 1 == cases.len()) };

    let mut instr_cases = cases
        .iter()
        .enumerate()
        .map(|(i, case)| emit_case(e, env, case, should_gen_break(i)))
        .map(|x| x.map(|(instr_body, label)| (instr_body, Some(label))))
        .collect::<Result<Vec<_>>>()?;

    let default_label = match dfl {
        None => {
            // emit warning/exception for missing default
            let default_label = e.label_gen_mut().next_regular();
            instr_cases.push((
                InstrSeq::gather(vec![
                    instr::label(default_label),
                    emit_pos_then(pos, instr::throw_non_exhaustive_switch()),
                ]),
                None,
            ));
            default_label
        }
        Some(dfl) => {
            let (dinstrs, default_label) = emit_default_case(e, env, dfl)?;
            instr_cases.push((dinstrs, None));
            default_label
        }
    };

    let (case_body_instrs, case_exprs): (Vec<InstrSeq<'arena>>, Vec<_>) =
        instr_cases.into_iter().unzip();

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

fn emit_switch<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    scrutinee_expr: &ast::Expr,
    cl: &[ast::Case],
    dfl: &Option<ast::DefaultCase>,
) -> Result<InstrSeq<'arena>> {
    let (instr_init, instr_free) = if scrutinee_expr.2.is_lvar() {
        (instr::empty(), instr::empty())
    } else {
        (
            emit_expr::emit_expr(e, env, scrutinee_expr)?,
            instr::pop_c(),
        )
    };
    let break_label = e.label_gen_mut().next_regular();

    let (case_expr_instrs, case_body_instrs, default_label) =
        env.do_in_switch_body(e, break_label, cl, dfl, |env, e, cases, dfl| {
            emit_cases(env, e, pos, scrutinee_expr, cases, dfl)
        })?;
    Ok(InstrSeq::gather(vec![
        instr_init,
        case_expr_instrs,
        instr_free,
        instr::jmp(default_label),
        case_body_instrs,
        instr::label(break_label),
    ]))
}

fn is_empty_block(b: &[ast::Stmt]) -> bool {
    b.iter().all(|s| s.1.is_noop())
}

fn emit_try_catch_finally<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    r#try: &[ast::Stmt],
    catch: &[ast::Catch],
    finally: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let is_try_block_empty = false;
    let emit_try_block =
        |env: &mut Env<'a, 'arena>, e: &mut Emitter<'arena, 'decl>, finally_start: Label| {
            env.do_in_try_catch_body(e, finally_start, r#try, catch, |env, e, t, c| {
                emit_try_catch(e, env, pos, t, c)
            })
        };
    e.local_scope(|e| emit_try_finally_(e, env, pos, emit_try_block, finally, is_try_block_empty))
}

fn emit_try_finally<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    try_block: &[ast::Stmt],
    finally_block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let is_try_block_empty = is_empty_block(try_block);
    let emit_try_block =
        |env: &mut Env<'a, 'arena>, e: &mut Emitter<'arena, 'decl>, finally_start: Label| {
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
    'decl,
    E: Fn(&mut Env<'a, 'arena>, &mut Emitter<'arena, 'decl>, Label) -> Result<InstrSeq<'arena>>,
>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    emit_try_block: E,
    finally_block: &[ast::Stmt],
    is_try_block_empty: bool,
) -> Result<InstrSeq<'arena>> {
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
    let try_body_result = emit_try_block(env, e, finally_start);
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
    label_rewriter::rewrite_with_fresh_regular_labels(e, &mut finally_body_for_catch);

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

    let enclosing_span = env.scope.get_span_or_none();
    let try_instrs = if jump_instrs_is_empty {
        try_body
    } else {
        tfr::cleanup_try_body(try_body)
    };
    let catch_instrs = InstrSeq::gather(vec![
        emit_pos(&enclosing_span),
        make_finally_catch(e, exn_local, finally_body_for_catch),
    ]);
    let middle = scope::create_try_catch(e.label_gen_mut(), None, true, try_instrs, catch_instrs);

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

fn make_finally_catch<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    exn_local: Local,
    finally_body: InstrSeq<'arena>,
) -> InstrSeq<'arena> {
    let l2 = instr::unset_l(*e.local_gen_mut().get_retval());
    let l1 = instr::unset_l(*e.local_gen_mut().get_label());
    InstrSeq::gather(vec![
        instr::pop_l(exn_local),
        l1,
        l2,
        scope::create_try_catch(
            e.label_gen_mut(),
            None,
            false,
            finally_body,
            InstrSeq::gather(vec![instr::push_l(exn_local), instr::chain_faults()]),
        ),
        instr::push_l(exn_local),
        instr::throw(),
    ])
}

fn emit_try_catch<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    try_block: &[ast::Stmt],
    catch_list: &[ast::Catch],
) -> Result<InstrSeq<'arena>> {
    e.local_scope(|e| emit_try_catch_(e, env, pos, try_block, catch_list))
}

fn emit_try_catch_<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    try_block: &[ast::Stmt],
    catch_list: &[ast::Catch],
) -> Result<InstrSeq<'arena>> {
    if is_empty_block(try_block) {
        return Ok(instr::empty());
    };
    let end_label = e.label_gen_mut().next_regular();

    let catch_instrs = InstrSeq::gather(
        catch_list
            .iter()
            .map(|catch| emit_catch(e, env, pos, end_label, catch))
            .collect::<Result<Vec<_>>>()?,
    );
    let in_try = env.flags.contains(env::Flags::IN_TRY);
    env.flags.set(env::Flags::IN_TRY, true);
    let try_body = emit_stmts(e, env, try_block);
    env.flags.set(env::Flags::IN_TRY, in_try);

    let try_instrs = InstrSeq::gather(vec![try_body?, emit_pos(pos)]);
    Ok(scope::create_try_catch(
        e.label_gen_mut(),
        Some(end_label),
        false,
        try_instrs,
        catch_instrs,
    ))
}

fn emit_catch<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    end_label: Label,
    ast::Catch(catch_ty, catch_lid, catch_block): &ast::Catch,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    // Note that this is a "regular" label; we're not going to branch to
    // it directly in the event of an exception.
    let next_catch = e.label_gen_mut().next_regular();
    let class_id = hhbc::ClassName::from_ast_name_and_mangle(alloc, &catch_ty.1);
    let ast::Lid(_pos, catch_local_id) = catch_lid;
    Ok(InstrSeq::gather(vec![
        instr::dup(),
        instr::instance_of_d(class_id),
        instr::jmp_z(next_catch),
        instr::set_l(e.named_local(local_id::get_name(catch_local_id).into())),
        instr::pop_c(),
        emit_stmts(e, env, catch_block)?,
        emit_pos(pos),
        instr::jmp(end_label),
        instr::label(next_catch),
    ]))
}

fn emit_foreach<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    collection: &ast::Expr,
    iterator: &ast::AsExpr,
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    use ast::AsExpr as A;
    e.local_scope(|e| match iterator {
        A::AsV(_) | A::AsKv(_, _) => emit_foreach_(e, env, pos, collection, iterator, block),
        A::AwaitAsV(pos, _) | A::AwaitAsKv(pos, _, _) => {
            emit_foreach_await(e, env, pos, collection, iterator, block)
        }
    })
}

fn emit_foreach_<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    collection: &ast::Expr,
    iterator: &ast::AsExpr,
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let collection_instrs = emit_expr::emit_expr(e, env, collection)?;
    scope::with_unnamed_locals_and_iterators(e, |e| {
        let iter_id = e.iterator_mut().get();
        let loop_break_label = e.label_gen_mut().next_regular();
        let loop_continue_label = e.label_gen_mut().next_regular();
        let loop_head_label = e.label_gen_mut().next_regular();
        let (key_id, val_id, preamble) = emit_iterator_key_value_storage(e, env, iterator)?;
        let iter_args = IterArgs {
            iter_id,
            key_id: key_id.unwrap_or(Local::INVALID),
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
        let iter_init = InstrSeq::gather(vec![
            collection_instrs,
            emit_pos(&collection.1),
            instr::iter_init(iter_args.clone(), loop_break_label),
        ]);
        let iterate = InstrSeq::gather(vec![
            instr::label(loop_head_label),
            preamble,
            body,
            instr::label(loop_continue_label),
            emit_pos(pos),
            instr::iter_next(iter_args, loop_head_label),
        ]);
        let iter_done = instr::label(loop_break_label);
        Ok((iter_init, iterate, iter_done))
    })
}

fn emit_foreach_await<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    collection: &ast::Expr,
    iterator: &ast::AsExpr,
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let instr_collection = emit_expr::emit_expr(e, env, collection)?;
    scope::with_unnamed_local(e, |e, iter_temp_local| {
        let alloc = e.alloc;
        let input_is_async_iterator_label = e.label_gen_mut().next_regular();
        let next_label = e.label_gen_mut().next_regular();
        let exit_label = e.label_gen_mut().next_regular();
        let pop_and_exit_label = e.label_gen_mut().next_regular();
        let async_eager_label = e.label_gen_mut().next_regular();
        let next_meth = hhbc::MethodName::from_raw_string(alloc, "next");
        let iter_init = InstrSeq::gather(vec![
            instr_collection,
            instr::dup(),
            instr::instance_of_d(hhbc::ClassName::from_raw_string(alloc, "HH\\AsyncIterator")),
            instr::jmp_nz(input_is_async_iterator_label),
            emit_fatal::emit_fatal_runtime(
                alloc,
                pos,
                "Unable to iterate non-AsyncIterator asynchronously",
            ),
            instr::label(input_is_async_iterator_label),
            instr::pop_l(iter_temp_local),
        ]);
        let loop_body_instr =
            env.do_in_loop_body(e, exit_label, next_label, None, block, emit_block)?;
        let iterate = InstrSeq::gather(vec![
            instr::label(next_label),
            instr::c_get_l(iter_temp_local),
            instr::null_uninit(),
            instr::f_call_obj_method_d(
                FCallArgs::new(
                    FCallArgsFlags::default(),
                    1,
                    0,
                    vec![],
                    vec![],
                    Some(async_eager_label),
                    None,
                ),
                next_meth,
            ),
            instr::await_(),
            instr::label(async_eager_label),
            instr::dup(),
            instr::is_type_c(IsTypeOp::Null),
            instr::jmp_nz(pop_and_exit_label),
            emit_foreach_await_key_value_storage(e, env, iterator)?,
            loop_body_instr,
            emit_pos(pos),
            instr::jmp(next_label),
            instr::label(pop_and_exit_label),
            instr::pop_c(),
            instr::label(exit_label),
        ]);
        let iter_done = instr::empty();
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
fn emit_iterator_key_value_storage<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    iterator: &ast::AsExpr,
) -> Result<(Option<Local>, Local, InstrSeq<'arena>)> {
    use ast::AsExpr as A;
    fn get_id_of_simple_lvar_opt(lvar: &ast::Expr_) -> Result<Option<&str>> {
        if let Some(ast::Lid(pos, id)) = lvar.as_lvar() {
            let name = local_id::get_name(id);
            if name == special_idents::THIS {
                return Err(Error::fatal_parse(pos, "Cannot re-assign $this"));
            } else if !(superglobals::is_superglobal(name)) {
                return Ok(Some(name));
            }
        };
        Ok(None)
    }
    match iterator {
        A::AsKv(k, v) => Ok(
            match (
                get_id_of_simple_lvar_opt(&k.2)?,
                get_id_of_simple_lvar_opt(&v.2)?,
            ) {
                (Some(key_id), Some(val_id)) => (
                    Some(e.named_local(key_id.into())),
                    e.named_local(val_id.into()),
                    instr::empty(),
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
                    if !(k.2).is_list() {
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
        A::AsV(v) => Ok(match get_id_of_simple_lvar_opt(&v.2)? {
            Some(val_id) => (None, e.named_local(val_id.into()), instr::empty()),
            None => {
                let val_local = e.local_gen_mut().get_unnamed();
                let (val_preamble, val_load) = emit_iterator_lvalue_storage(e, env, v, val_local)?;
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
        _ => Err(Error::unrecoverable(
            "emit_iterator_key_value_storage with iterator using await",
        )),
    }
}

fn emit_iterator_lvalue_storage<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    lvalue: &ast::Expr,
    local: Local,
) -> Result<(Vec<InstrSeq<'arena>>, Vec<InstrSeq<'arena>>)> {
    match &lvalue.2 {
        ast::Expr_::Call(_) => Err(Error::fatal_parse(
            &lvalue.1,
            "Can't use return value in write context",
        )),
        ast::Expr_::List(es) => {
            let (preamble, load_values) = emit_load_list_elements(
                e,
                env,
                vec![instr::base_l(local, MOpMode::Warn, ReadonlyOp::Any)],
                es,
            )?;
            let load_values = vec![
                InstrSeq::gather(load_values.into_iter().rev().collect()),
                instr::unset_l(local),
            ];
            Ok((preamble, load_values))
        }
        _ => {
            let (lhs, rhs, set_op) = emit_expr::emit_lval_op_nonlist_steps(
                e,
                env,
                &lvalue.1,
                LValOp::Set,
                lvalue,
                instr::c_get_l(local),
                1,
                false,
                false, // TODO: Readonly iterator assignment
            )?;
            Ok((
                vec![lhs],
                vec![rhs, set_op, instr::pop_c(), instr::unset_l(local)],
            ))
        }
    }
}

fn emit_load_list_elements<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    path: Vec<InstrSeq<'arena>>,
    es: &[ast::Expr],
) -> Result<(Vec<InstrSeq<'arena>>, Vec<InstrSeq<'arena>>)> {
    let (preamble, load_value): (Vec<Vec<InstrSeq<'arena>>>, Vec<Vec<InstrSeq<'arena>>>) = es
        .iter()
        .enumerate()
        .map(|(i, x)| {
            emit_load_list_element(
                e,
                env,
                path.iter().map(InstrSeq::clone).collect::<Vec<_>>(),
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

fn emit_load_list_element<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    mut path: Vec<InstrSeq<'arena>>,
    i: usize,
    elem: &ast::Expr,
) -> Result<(Vec<InstrSeq<'arena>>, Vec<InstrSeq<'arena>>)> {
    let query_value = |path| {
        InstrSeq::gather(vec![
            InstrSeq::gather(path),
            instr::query_m(0, QueryMOp::CGet, MemberKey::EI(i as i64, ReadonlyOp::Any)),
        ])
    };
    Ok(match &elem.2 {
        ast::Expr_::Lvar(lid) => {
            let load_value = InstrSeq::gather(vec![
                query_value(path),
                instr::set_l(e.named_local(local_id::get_name(&lid.1).into())),
                instr::pop_c(),
            ]);
            (vec![], vec![load_value])
        }
        ast::Expr_::List(es) => {
            let instr_dim = instr::dim(MOpMode::Warn, MemberKey::EI(i as i64, ReadonlyOp::Any));
            path.push(instr_dim);
            emit_load_list_elements(e, env, path, es)?
        }
        _ => {
            let set_instrs = emit_expr::emit_lval_op_nonlist(
                e,
                env,
                &elem.1,
                LValOp::Set,
                elem,
                query_value(path),
                1,
                false,
                false, // TODO readonly load list elements
            )?;
            let load_value = InstrSeq::gather(vec![set_instrs, instr::pop_c()]);
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
fn emit_foreach_await_key_value_storage<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    iterator: &ast::AsExpr,
) -> Result<InstrSeq<'arena>> {
    use ast::AsExpr as A;

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
fn emit_foreach_await_lvalue_storage<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    lvalue: &ast::Expr,
    indices: &[isize],
    keep_on_stack: bool,
) -> Result<InstrSeq<'arena>> {
    scope::with_unnamed_local(e, |e, local| {
        let (init, assign) = emit_expr::emit_lval_op_list(
            e,
            env,
            &lvalue.1,
            Some(&local),
            indices,
            lvalue,
            false,
            false,
        )?;
        Ok((
            instr::pop_l(local),
            InstrSeq::gather(vec![init, assign]),
            if keep_on_stack {
                instr::push_l(local)
            } else {
                instr::empty()
            },
        ))
    })
}

fn emit_stmts<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    stl: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    Ok(InstrSeq::gather(
        stl.iter()
            .map(|s| emit_stmt(e, env, s))
            .collect::<Result<Vec<_>>>()?,
    ))
}

fn emit_block_with_temps<'a, 'arena, 'decl>(
    env: &mut Env<'a, 'arena>,
    emitter: &mut Emitter<'arena, 'decl>,
    lids: &Option<Vec<ast::Lid>>,
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    if let Some(lids) = lids {
        scope::with_unnamed_temps(emitter, lids, |e| emit_stmts(e, env, block))
    } else {
        emit_stmts(emitter, env, block)
    }
}

fn emit_block<'a, 'arena, 'decl>(
    env: &mut Env<'a, 'arena>,
    emitter: &mut Emitter<'arena, 'decl>,
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    emit_stmts(emitter, env, block)
}

fn emit_do<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    body: &[ast::Stmt],
    cond: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    let break_label = e.label_gen_mut().next_regular();
    let cont_label = e.label_gen_mut().next_regular();
    let start_label = e.label_gen_mut().next_regular();
    let jmpnz_instr = emit_expr::emit_jmpnz(e, env, cond, start_label)?.instrs;
    Ok(InstrSeq::gather(vec![
        instr::label(start_label),
        env.do_in_loop_body(e, break_label, cont_label, None, body, emit_block)?,
        instr::label(cont_label),
        jmpnz_instr,
        instr::label(break_label),
    ]))
}

fn emit_while<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    cond: &ast::Expr,
    body: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
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
    Ok(InstrSeq::gather(vec![
        i1,
        instr::label(start_label),
        i2,
        instr::label(cont_label),
        i3,
        instr::label(break_label),
    ]))
}

fn emit_for<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    e1: &[ast::Expr],
    e2: Option<&ast::Expr>,
    e3: &[ast::Expr],
    body: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    let break_label = e.label_gen_mut().next_regular();
    let cont_label = e.label_gen_mut().next_regular();
    let start_label = e.label_gen_mut().next_regular();
    fn emit_cond<'a, 'arena, 'decl>(
        emitter: &mut Emitter<'arena, 'decl>,
        env: &mut Env<'a, 'arena>,
        jmpz: bool,
        label: Label,
        cond: Option<&ast::Expr>,
    ) -> Result<InstrSeq<'arena>> {
        Ok(match cond {
            None => {
                if jmpz {
                    instr::empty()
                } else {
                    instr::jmp(label)
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
    let i4 = emit_expr::emit_ignored_exprs(e, env, &Pos::NONE, e3)?;
    let i3 = env.do_in_loop_body(e, break_label, cont_label, None, body, emit_block)?;
    let i2 = emit_cond(e, env, true, break_label, e2)?;
    let i1 = emit_expr::emit_ignored_exprs(e, env, &Pos::NONE, e1)?;
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

pub fn emit_dropthrough_return<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
) -> Result<InstrSeq<'arena>> {
    match e.statement_state().default_dropthrough.as_ref() {
        Some(instrs) => Ok(InstrSeq::clone(instrs)),
        None => {
            let ret = emit_return(e, env)?;
            let state = e.statement_state();
            Ok(InstrSeq::gather(vec![
                emit_pos(&(state.function_pos.last_char())),
                state.default_return_value.clone(),
                ret,
            ]))
        }
    }
}

pub fn emit_final_stmt<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    stmt: &ast::Stmt,
) -> Result<InstrSeq<'arena>> {
    match &stmt.1 {
        a::Stmt_::Throw(_) | a::Stmt_::Return(_) | a::Stmt_::YieldBreak => emit_stmt(e, env, stmt),
        a::Stmt_::Block(box (None, stmts)) => emit_final_stmts(env, e, stmts),
        _ => {
            let ret = emit_dropthrough_return(e, env)?;
            Ok(InstrSeq::gather(vec![emit_stmt(e, env, stmt)?, ret]))
        }
    }
}

pub fn emit_final_stmts<'a, 'arena, 'decl>(
    env: &mut Env<'a, 'arena>,
    e: &mut Emitter<'arena, 'decl>,
    block: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
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

pub fn emit_markup<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    (_, s): &ast::Pstring,
    check_for_hashbang: bool,
) -> Result<InstrSeq<'arena>> {
    let markup = if s.is_empty() {
        instr::empty()
    } else {
        lazy_static! {
            static ref HASHBANG_PAT: regex::Regex = regex::Regex::new(r"^#!.*\n").unwrap();
        }
        let tail = String::from(match HASHBANG_PAT.shortest_match(s) {
            Some(i) if check_for_hashbang => {
                // if markup text starts with #!
                // - extract a line with hashbang
                // - it will be emitted as a call to print_hashbang function
                // - emit remaining part of text as regular markup
                &s[i..]
            }
            _ => s,
        });
        if tail.is_empty() {
            instr::empty()
        } else {
            let call_expr = hack_expr!("echo #{str(tail)}");
            emit_expr::emit_ignored_expr(e, env, &Pos::NONE, &call_expr)?
        }
    };
    Ok(markup)
}

fn emit_break<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
) -> InstrSeq<'arena> {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::IS_BREAK, env, pos)
}

fn emit_continue<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
) -> InstrSeq<'arena> {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::empty(), env, pos)
}

fn emit_await_assignment<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    lval: &ast::Expr,
    r: &ast::Expr,
) -> Result<InstrSeq<'arena>> {
    match lval.2.as_lvar() {
        Some(ast::Lid(_, id)) if !emit_expr::is_local_this(env, id) => Ok(InstrSeq::gather(vec![
            emit_expr::emit_await(e, env, pos, r)?,
            emit_pos(pos),
            instr::pop_l(emit_expr::get_local(e, env, pos, local_id::get_name(id))?),
        ])),
        _ => {
            let awaited_instrs = emit_await(e, env, pos, r)?;
            scope::with_unnamed_local(e, |e, temp| {
                let rhs_instrs = instr::push_l(temp);
                let (lhs, rhs, setop) = emit_expr::emit_lval_op_nonlist_steps(
                    e,
                    env,
                    pos,
                    LValOp::Set,
                    lval,
                    rhs_instrs,
                    1,
                    false,
                    false, // unnamed local assignment does not need readonly check
                )?;
                Ok((
                    InstrSeq::gather(vec![awaited_instrs, instr::pop_l(temp)]),
                    lhs,
                    InstrSeq::gather(vec![rhs, setop, instr::pop_c()]),
                ))
            })
        }
    }
}

fn emit_if<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    condition: &ast::Expr,
    consequence: &[ast::Stmt],
    alternative: &[ast::Stmt],
) -> Result<InstrSeq<'arena>> {
    if alternative.is_empty() || (alternative.len() == 1 && alternative[0].1.is_noop()) {
        let done_label = e.label_gen_mut().next_regular();
        let consequence_instr = emit_stmts(e, env, consequence)?;
        Ok(InstrSeq::gather(vec![
            emit_expr::emit_jmpz(e, env, condition, done_label)?.instrs,
            consequence_instr,
            instr::label(done_label),
        ]))
    } else {
        let alternative_label = e.label_gen_mut().next_regular();
        let done_label = e.label_gen_mut().next_regular();
        let consequence_instr = emit_stmts(e, env, consequence)?;
        let alternative_instr = emit_stmts(e, env, alternative)?;
        Ok(InstrSeq::gather(vec![
            emit_expr::emit_jmpz(e, env, condition, alternative_label)?.instrs,
            consequence_instr,
            emit_pos(pos),
            instr::jmp(done_label),
            instr::label(alternative_label),
            alternative_instr,
            instr::label(done_label),
        ]))
    }
}

// Treat the declaration of a local as a binop assignment.
// TODO: clone less
// TODO: Enforce the type hint from the declaration?
fn emit_declare_local<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a, 'arena>,
    pos: &Pos,
    id: &ast::Lid,
    e_: &Option<ast::Expr>,
) -> Result<InstrSeq<'arena>> {
    if let Some(e_) = e_ {
        let bop = ast::Binop {
            bop: ast_defs::Bop::Eq(None),
            lhs: ast::Expr::new((), pos.clone(), ast::Expr_::mk_lvar(id.clone())),
            rhs: e_.clone(),
        };
        let e2 = ast::Expr::new((), pos.clone(), ast::Expr_::mk_binop(bop.clone()));
        emit_binop(e, env, &e2, pos, &bop)
    } else {
        Ok(InstrSeq::gather(vec![]))
    }
}
