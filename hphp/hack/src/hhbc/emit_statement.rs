// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use crate::try_finally_rewriter as tfr;

use emit_expression_rust::{self as emit_expr, Setrange};
use emit_fatal_rust as emit_fatal;
use emit_pos_rust::emit_pos_then;
use env::{emitter::Emitter, Env};
use hhbc_ast_rust::*;
use hhbc_id_rust::{self as hhbc_id, Id};
use instruction_sequence_rust::{Error::Unrecoverable, InstrSeq, Result};
use oxidized::{aast as a, ast as tast, ast_defs, pos::Pos};

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

fn emit_return(e: &mut Emitter, env: &mut Env) -> InstrSeq {
    tfr::emit_return(e, false, env)
}

fn emit_def_inline<Ex, Fb, En, Hi>(e: &mut Emitter, def: a::Def<Ex, Fb, En, Hi>) -> Result {
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
            let Id(pos, name) = (*cd).name;
            let num = name.parse::<ClassNum>().unwrap();
            emit_pos_then(e, &pos, make_def_instr(num))
        }
        a::Def::Typedef(td) => {
            let Id(pos, name) = (*td).name;
            let num = name.parse::<TypedefNum>().unwrap();
            emit_pos_then(e, &pos, InstrSeq::make_deftypealias(num))
        }
        a::Def::RecordDef(rd) => {
            let Id(pos, name) = (*rd).name;
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
        static ref RE: Regex = Regex::new("(?i)^hh\\set_bytes(_rev|)_([a-z0-9]+)(_vec|)$").unwrap();
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

fn emit_stmt(e: &mut Emitter, env: &mut Env, stmt: &tast::Stmt) -> Result {
    let pos = &stmt.0;
    match &stmt.1 {
        a::Stmt_::Expr(e_) => match &(*e_).1 {
            a::Expr_::YieldBreak => {
                return Ok(InstrSeq::gather(vec![
                    InstrSeq::make_null(),
                    emit_return(e, env),
                ]))
            }
            a::Expr_::Call(c) => {
                if let (_, a::Expr(_, a::Expr_::Id(sid)), _, exprs, None) = &**c {
                    let expr = &(**c).1;
                    let ft = hhbc_id::function::Type::from_ast_name(&(*sid).1);
                    let fname = ft.to_raw_string();
                    return if fname.eq_ignore_ascii_case("unset") {
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
                                    let (ast_defs::ParamKind::Pinout, ex) = &**cc;
                                    (&exprs.as_slice()[..exprs.len() - 1], Some(ex))
                                }
                                _ => (exprs.as_slice(), None),
                            };
                            emit_expr::emit_set_range_expr(e, env, pos, fname, kind, args, arg)
                        } else {
                            emit_expr::emit_ignored_expr(e, env, pos, expr)
                        }
                    };
                }
            }
            _ => (),
        },
        _ => (),
    }
    unimplemented!("TODO(hrust)")
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
