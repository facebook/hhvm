// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code, unused_variables)]

use crate::try_finally_rewriter as tfr;

use emit_expression_rust::{self as emit_expr, emit_await, emit_expr, LValOp, Setrange};
use emit_fatal_rust as emit_fatal;
use emit_pos_rust::{emit_pos, emit_pos_then};
use env::{emitter::Emitter, Env};
use hhbc_ast_rust::*;
use hhbc_id_rust::{self as hhbc_id, Id};
use instruction_sequence_rust::{Error::Unrecoverable, InstrSeq, Result};
use label_rust::Label;
use naming_special_names_rust::special_functions;
use oxidized::{aast as a, ast as tast, ast_defs, pos::Pos};
use scope_rust::scope;

use lazy_static::lazy_static;
use regex::Regex;

/// Context for code generation. It would be more elegant to pass this
/// around in an environment parameter.
pub(in crate) struct State {
    pub(crate) verify_return: Option<a::Hint>,
    pub(crate) default_return_value: InstrSeq,
    pub(crate) default_dropthrough: Option<InstrSeq>,
    pub(crate) verify_out: InstrSeq,
    pub(crate) function_pos: Pos,
    pub(crate) num_out: usize,
}

impl State {
    fn init() -> Box<dyn std::any::Any> {
        Box::new(State {
            verify_return: None,
            default_return_value: InstrSeq::make_null(),
            default_dropthrough: None,
            verify_out: InstrSeq::Empty,
            num_out: 0,
            function_pos: Pos::make_none(),
        })
    }
}
env::lazy_emit_state!(statement_state, State, State::init);

// Expose a mutable ref to state for emit_body so that it can set it appropriately
pub(crate) fn set_state(e: &mut Emitter, state: State) {
    *e.emit_state_mut() = state;
}

pub(crate) type Level = usize;

fn get_level<Ex, Fb, En, Hi>(
    pos: Pos,
    op: &str,
    ex: a::Expr_<Ex, Fb, En, Hi>,
) -> Result<a::BreakContinueLevel> {
    if let a::Expr_::Int(ref s) = ex {
        match s.parse::<isize>() {
            Ok(level) if level > 0 => return Ok(a::BreakContinueLevel::LevelOk(Some(level))),
            Ok(level) if level <= 0 => {
                return Err(emit_fatal::raise_fatal_parse(
                    &pos,
                    format!("'{}' operator accepts only positive numbers", op),
                ))
            }
            _ => (),
        }
    }
    let msg = format!("'{}' with non-constant operand is not supported", op);
    Err(emit_fatal::raise_fatal_parse(&pos, msg))
}

// Wrapper functions

fn emit_return(e: &mut Emitter, env: &mut Env) -> Result {
    tfr::emit_return(e, false, env)
}

fn emit_def_inline<Ex, Fb, En, Hi>(e: &mut Emitter, def: &a::Def<Ex, Fb, En, Hi>) -> Result {
    use ast_defs::Id;
    Ok(match def {
        a::Def::Class(cd) => {
            let make_def_instr = |num| {
                if e.context().systemlib() {
                    InstrSeq::make_defclsnop(num)
                } else {
                    InstrSeq::make_defcls(num)
                }
            };
            let Id(pos, name) = &(*cd).name;
            let num = name.parse::<ClassNum>().unwrap();
            emit_pos_then(e, &pos, make_def_instr(num))
        }
        a::Def::Typedef(td) => {
            let Id(pos, name) = &(*td).name;
            let num = name.parse::<TypedefNum>().unwrap();
            emit_pos_then(e, &pos, InstrSeq::make_deftypealias(num))
        }
        a::Def::RecordDef(rd) => {
            let Id(pos, name) = &(*rd).name;
            let num = name.parse::<ClassNum>().unwrap();
            emit_pos_then(e, &pos, InstrSeq::make_defrecord(num))
        }
        _ => {
            return Err(Unrecoverable(
                "Define inline: Invalid inline definition".into(),
            ))
        }
    })
}

fn set_bytes_kind(name: &str) -> Option<Setrange> {
    lazy_static! {
        static ref RE: Regex =
            Regex::new(r#"(?i)^hh\\set_bytes(_rev)_([a-z0-9]+)(_vec)$"#).unwrap();
    }
    RE.captures(name).map_or(None, |groups| {
        let op = if !groups.get(1).unwrap().as_str().is_empty() {
            // == _rev
            SetrangeOp::Reverse
        } else {
            SetrangeOp::Forward
        };
        let kind = groups.get(2).unwrap().as_str();
        let vec = !groups.get(3).unwrap().as_str().is_empty(); // == _vec
        if kind == "string" && !vec {
            Some(Setrange {
                size: 1,
                vec: false,
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
        a::Stmt_::Expr(e_) => match &e_.1 {
            a::Expr_::YieldBreak => Ok(InstrSeq::gather(vec![
                InstrSeq::make_null(),
                emit_return(e, env)?,
            ])),
            a::Expr_::YieldFrom(e_) => Ok(InstrSeq::gather(vec![
                emit_yield_from_delegates(e, env, pos, e_)?,
                emit_pos(e, pos),
                InstrSeq::make_popc(),
            ])),
            a::Expr_::Await(a) => Ok(InstrSeq::gather(vec![
                emit_await(e, env, &e_.0, a)?,
                InstrSeq::make_popc(),
            ])),
            a::Expr_::Call(c) => {
                if let (_, a::Expr(_, a::Expr_::Id(sid)), _, exprs, None) = c.as_ref() {
                    let expr = &c.1;
                    let ft = hhbc_id::function::Type::from_ast_name(&sid.1);
                    let fname = ft.to_raw_string();
                    if fname.eq_ignore_ascii_case("unset") {
                        Ok(InstrSeq::gather(
                            exprs
                                .iter()
                                .map(|ex| emit_expr::emit_unset_expr(env, ex))
                                .collect::<std::result::Result<Vec<_>, _>>()?,
                        ))
                    } else {
                        if let Some(kind) = set_bytes_kind(fname) {
                            let (args, arg) = match exprs.last() {
                                Some(a::Expr(_, a::Expr_::Callconv(cc))) => {
                                    let (ast_defs::ParamKind::Pinout, ex) = cc.as_ref();
                                    (&exprs.as_slice()[..exprs.len() - 1], Some(ex))
                                }
                                _ => (exprs.as_slice(), None),
                            };
                            emit_expr::emit_set_range_expr(e, env, pos, fname, kind, args, arg)
                        } else {
                            emit_expr::emit_ignored_expr(e, env, pos, e_)
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
                                            InstrSeq::make_popl(temp.clone()),
                                        ]),
                                        emit_expr::emit_lval_op_list(
                                            e,
                                            env,
                                            pos,
                                            Some(temp.clone()),
                                            &[],
                                            e_lhs,
                                            false,
                                        )?
                                        .into(),
                                        InstrSeq::make_unsetl(temp),
                                    ))
                                })
                            } else {
                                Ok(InstrSeq::gather(vec![
                                    awaited_instrs,
                                    InstrSeq::make_popc(),
                                ]))
                            }
                        } else {
                            emit_await_assignment(e, env, await_pos, e_lhs, e_await)
                        }
                    } else if let Some(e_yeild) = e_rhs.1.as_yield_from() {
                        e.local_scope(|e| {
                            let temp = e.local_gen_mut().get_unnamed();
                            let rhs_instrs = InstrSeq::make_pushl(temp.clone());
                            Ok(InstrSeq::gather(vec![
                                emit_yield_from_delegates(e, env, pos, e_yeild)?,
                                InstrSeq::make_setl(temp),
                                InstrSeq::make_popc(),
                                emit_expr::emit_lval_op_nonlist(
                                    e,
                                    env,
                                    pos,
                                    LValOp::Set,
                                    e_lhs,
                                    rhs_instrs,
                                    1,
                                    false,
                                )?,
                                InstrSeq::make_popc(),
                            ]))
                        })
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
                let expr_instr = if let Some(e_) = r.1.as_await() {
                    emit_await(e, env, &r.0, e_)?
                } else if let Some(_) = r.1.as_yield_from() {
                    emit_yield_from_delegates(e, env, pos, &r)?
                } else {
                    emit_expr(e, env, &r)?
                };
                Ok(InstrSeq::gather(vec![
                    expr_instr,
                    emit_pos(e, pos),
                    emit_return(e, env)?,
                ]))
            }
            None => Ok(InstrSeq::gather(vec![
                InstrSeq::make_null(),
                emit_pos(e, pos),
                emit_return(e, env)?,
            ])),
        },
        a::Stmt_::GotoLabel(l) => Ok(InstrSeq::make_label(Label::Named(l.1.clone()))),
        a::Stmt_::Goto(l) => tfr::emit_goto(false, l.1.clone(), env, e.local_gen_mut()),
        a::Stmt_::Block(b) => emit_stmts(e, env, &b),
        a::Stmt_::If(f) => emit_if(e, env, pos, &f.0, &f.1, &f.2),
        a::Stmt_::While(_) => unimplemented!("TODO(hrust)"),
        a::Stmt_::Using(_) => unimplemented!("TODO(hrust)"),
        a::Stmt_::Break => Ok(emit_break(e, env, pos)),
        a::Stmt_::Continue => Ok(emit_continue(e, env, pos)),
        a::Stmt_::Do(x) => emit_do(
            e,
            env,
            &tast::Stmt(pos.clone(), tast::Stmt_::mk_block(x.0.clone())),
            &x.1,
        ),
        a::Stmt_::For(x) => emit_for(e, env, pos, &x.0, &x.1, &x.2, &x.3),
        a::Stmt_::Throw(_) => unimplemented!("TODO(hrust)"),
        a::Stmt_::Try(_) => unimplemented!("TODO(hrust)"),
        a::Stmt_::Switch(_) => unimplemented!("TODO(hrust)"),
        a::Stmt_::Foreach(_) => unimplemented!("TODO(hrust)"),
        a::Stmt_::DefInline(def) => emit_def_inline(e, &**def),
        a::Stmt_::Awaitall(_) => unimplemented!("TODO(hrust)"),
        a::Stmt_::Markup(x) => emit_markup(e, env, (&x.0, &x.1), false),
        a::Stmt_::Fallthrough | a::Stmt_::Noop => Ok(InstrSeq::Empty),
    }
}

fn emit_stmts(e: &mut Emitter, env: &Env, stl: &[tast::Stmt]) -> Result {
    // TODO(shiqicao):
    //     Ocaml's env is immutable, we need to match the same behavior
    //     during porting. Simulating immutability will change every `&Env`
    //     to `&mut Env`. The following is a hack to build.
    let mut new_env: Env = env.clone();
    Ok(InstrSeq::gather(
        stl.iter()
            .map(|s| emit_stmt(e, &mut new_env, s))
            .collect::<Result<Vec<_>>>()?,
    ))
}

fn emit_stmt_wrapper(env: &mut Env, emitter: &mut Emitter, stmt: &tast::Stmt) -> Result {
    emit_stmt(emitter, env, stmt)
}

fn emit_do(e: &mut Emitter, env: &mut Env, body: &tast::Stmt, cond: &tast::Expr) -> Result {
    let break_label = e.label_gen_mut().next_regular();
    let cont_label = e.label_gen_mut().next_regular();
    let start_label = e.label_gen_mut().next_regular();
    Ok(InstrSeq::gather(vec![
        InstrSeq::make_label(start_label.clone()),
        env.do_in_loop_body(
            e,
            break_label.clone(),
            cont_label.clone(),
            None,
            body,
            emit_stmt_wrapper,
        )?,
        InstrSeq::make_label(cont_label),
        emit_expr::emit_jmpnz(e, env, cond, &start_label)?.instrs,
        InstrSeq::make_label(break_label),
    ]))
}

fn emit_for(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    e1: &tast::Expr,
    e2: &tast::Expr,
    e3: &tast::Expr,
    body: &tast::Block,
) -> Result {
    let break_label = e.label_gen_mut().next_regular();
    let cont_label = e.label_gen_mut().next_regular();
    let start_label = e.label_gen_mut().next_regular();
    fn emit_cond(
        emitter: &mut Emitter,
        env: &mut Env,
        pos: &Pos,
        jmpz: bool,
        label: &Label,
        cond: &tast::Expr,
    ) -> Result {
        fn final_(
            emitter: &mut Emitter,
            env: &mut Env,
            pos: &Pos,
            jmpz: bool,
            label: &Label,
            cond: &tast::Expr,
        ) -> Result {
            Ok(if jmpz {
                emit_expr::emit_jmpz(emitter, env, cond, label)
            } else {
                emit_expr::emit_jmpnz(emitter, env, cond, label)
            }?
            .instrs)
        };
        fn expr_list(
            emitter: &mut Emitter,
            env: &mut Env,
            pos: &Pos,
            jmpz: bool,
            label: &Label,
            fst: &tast::Expr,
            tl: &[tast::Expr],
        ) -> Result<Vec<InstrSeq>> {
            match tl.split_first() {
                None => Ok(vec![final_(
                    emitter,
                    env,
                    pos,
                    jmpz,
                    label,
                    &tast::Expr(
                        Pos::make_none(),
                        tast::Expr_::mk_expr_list(vec![fst.clone()]),
                    ),
                )?]),
                Some((snd, tl)) => {
                    let mut res = vec![emit_expr::emit_ignored_expr(emitter, env, pos, fst)?];
                    res.extend_from_slice(
                        expr_list(emitter, env, pos, jmpz, label, snd, tl)?.as_slice(),
                    );
                    Ok(res)
                }
            }
        };
        match cond.1.as_expr_list() {
            Some(es) => Ok(match es.split_first() {
                None => {
                    if jmpz {
                        InstrSeq::Empty
                    } else {
                        InstrSeq::make_jmp(label.clone())
                    }
                }
                Some((hd, tl)) => {
                    InstrSeq::gather(expr_list(emitter, env, pos, jmpz, label, hd, tl)?)
                }
            }),
            None => final_(emitter, env, pos, jmpz, label, cond),
        }
    };
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
    Ok(InstrSeq::gather(vec![
        emit_expr::emit_ignored_expr(e, env, pos, e1)?,
        emit_cond(e, env, pos, true, &break_label, e2)?,
        InstrSeq::make_label(start_label.clone()),
        env.do_in_loop_body(
            e,
            break_label.clone(),
            cont_label.clone(),
            None,
            &tast::Stmt(pos.clone(), tast::Stmt_::mk_block(body.clone())),
            emit_stmt_wrapper,
        )?,
        InstrSeq::make_label(cont_label),
        emit_expr::emit_ignored_expr(e, env, pos, e3)?,
        emit_cond(e, env, pos, false, &start_label, e2)?,
        InstrSeq::make_label(break_label),
    ]))
}

fn emit_yield_from_delegates(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    expr: &tast::Expr,
) -> Result {
    let iter_num = e.iterator_mut().get();
    let loop_label = e.label_gen_mut().next_regular();
    Ok(InstrSeq::gather(vec![
        emit_expr(e, env, expr)?,
        emit_pos(e, pos),
        InstrSeq::make_cont_assign_delegate(iter_num),
        InstrSeq::create_try_catch(
            e.label_gen_mut(),
            None,
            false,
            InstrSeq::gather(vec![
                InstrSeq::make_null(),
                InstrSeq::make_label(loop_label.clone()),
                InstrSeq::make_cont_enter_delegate(),
                InstrSeq::make_yield_from_delegate(iter_num, loop_label),
            ]),
            InstrSeq::make_cont_unset_delegate_free(iter_num),
        ),
        InstrSeq::make_cont_unset_delegate_ignore(iter_num),
    ]))
}

pub fn emit_dropthrough_return(e: &mut Emitter, env: &mut Env) -> Result {
    let return_instrs = emit_return(e, env)?;
    let state = e.emit_state();
    match state.default_dropthrough.as_ref() {
        Some(instrs) => Ok(instrs.clone()),
        None => Ok(emit_pos_then(
            e,
            &(state.function_pos.last_char()),
            InstrSeq::gather(vec![state.default_return_value.clone(), return_instrs]),
        )),
    }
}

pub fn emit_final_stmt(e: &mut Emitter, env: &mut Env, stmt: &tast::Stmt) -> Result {
    match &stmt.1 {
        a::Stmt_::Throw(_) | a::Stmt_::Return(_) | a::Stmt_::Goto(_) => emit_stmt(e, env, stmt),
        a::Stmt_::Expr(expr) if expr.1.is_yield_break() => emit_stmt(e, env, stmt),
        a::Stmt_::Block(stmts) => emit_final_stmts(e, env, stmts),
        _ => Ok(InstrSeq::gather(vec![
            emit_stmt(e, env, stmt)?,
            emit_dropthrough_return(e, env)?,
        ])),
    }
}

fn emit_final_stmts(e: &mut Emitter, env: &mut Env, block: &[tast::Stmt]) -> Result {
    match block {
        [] => emit_dropthrough_return(e, env),
        [s] => emit_final_stmt(e, env, s),
        [s, ..] => Ok(InstrSeq::gather(vec![
            emit_stmt(e, env, s)?,
            emit_final_stmts(e, env, &block[1..])?,
        ])),
    }
}

pub fn emit_markup(
    e: &mut Emitter,
    env: &mut Env,
    ((_, s), echo_expr_opt): (&tast::Pstring, &Option<tast::Expr>),
    check_for_hashbang: bool,
) -> Result {
    let mut emit_ignored_call_expr = |fname: String, expr: tast::Expr| {
        let call_expr = tast::Expr(
            Pos::make_none(),
            tast::Expr_::mk_call(
                tast::CallType::Cnormal,
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
            Ok(InstrSeq::Empty)
        } else {
            emit_ignored_call_expr(
                fname,
                tast::Expr(Pos::make_none(), tast::Expr_::mk_string(expr_str)),
            )
        }
    };
    let markup = if s.is_empty() {
        InstrSeq::Empty
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
    let echo = match echo_expr_opt {
        None => InstrSeq::Empty,
        Some(echo_expr) => {
            emit_ignored_call_expr(special_functions::ECHO.into(), echo_expr.clone())?
        }
    };
    Ok(InstrSeq::gather(vec![markup, echo]))
}

fn emit_break(e: &mut Emitter, env: &mut Env, pos: &Pos) -> InstrSeq {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::IS_BREAK, env, pos, 1)
}

fn emit_continue(e: &mut Emitter, env: &mut Env, pos: &Pos) -> InstrSeq {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::empty(), env, pos, 1)
}

fn emit_temp_break(e: &mut Emitter, env: &mut Env, pos: &Pos, level: Level) -> InstrSeq {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::IS_BREAK, env, pos, level)
}

fn emit_temp_continue(e: &mut Emitter, env: &mut Env, pos: &Pos, level: Level) -> InstrSeq {
    use tfr::EmitBreakOrContinueFlags as Flags;
    tfr::emit_break_or_continue(e, Flags::empty(), env, pos, level)
}

fn emit_await_assignment(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    lval: &tast::Expr,
    r: &tast::Expr,
) -> Result {
    unimplemented!()
}

fn emit_if(
    e: &mut Emitter,
    env: &Env,
    pos: &Pos,
    condition: &tast::Expr,
    consequence: &[tast::Stmt],
    alternative: &[tast::Stmt],
) -> Result {
    if alternative.is_empty() || (alternative.len() == 1 && alternative[0].1.is_noop()) {
        let done_label = e.label_gen_mut().next_regular();
        Ok(InstrSeq::gather(vec![
            emit_expr::emit_jmpz(e, env, condition, &done_label)?.instrs,
            emit_stmts(e, env, consequence)?,
            InstrSeq::make_label(done_label),
        ]))
    } else {
        let alternative_label = e.label_gen_mut().next_regular();
        let done_label = e.label_gen_mut().next_regular();
        let consequence_instr = emit_stmts(e, env, consequence)?;
        let alternative_instr = emit_stmts(e, env, alternative)?;
        Ok(InstrSeq::gather(vec![
            emit_expr::emit_jmpz(e, env, condition, &alternative_label)?.instrs,
            consequence_instr,
            emit_pos(e, pos),
            InstrSeq::make_jmp(done_label.clone()),
            InstrSeq::make_label(alternative_label),
            alternative_instr,
            InstrSeq::make_label(done_label),
        ]))
    }
}
