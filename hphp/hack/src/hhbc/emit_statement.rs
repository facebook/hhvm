// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use emit_fatal_rust as emit_fatal;
use hhbc_ast_rust as hhbc_ast;
use instruction_sequence_rust as instruction_sequence;
use label_rust as label;
use local_rust as local;

use env::{emitter::Emitter, jump_targets as jt, Env};
use instruction_sequence::InstrSeq;
use label::Label;
use oxidized::{aast as a, pos::Pos};

/// Context for code generation. It would be more elegant to pass this
/// around in an environment parameter.
// TODO(hrust) restrict visibility further: pub(crate) -> pub(in crate::emit_statement)
pub(crate) struct State {
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

type Level = usize;

fn get_level<Ex, Fb, En, Hi>(
    pos: Pos,
    op: &str,
    ex: a::Expr_<Ex, Fb, En, Hi>,
) -> Result<a::BreakContinueLevel, emit_fatal::Error> {
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

/// Wrapper functions
mod try_finally_rewriter;
use try_finally_rewriter as tfr;

fn emit_return(e: &Emitter, env: &Env) -> InstrSeq {
    let ctx = e.emit_state();
    tfr::emit_return(
        e,
        &ctx.verify_return,
        &ctx.verify_out,
        ctx.num_out,
        false,
        env,
    )
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
