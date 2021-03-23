// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::emit_statement::Level;
use crate::reified_generics_helpers as reified;

use emit_expression_rust as emit_expression;
use emit_fatal_rust as emit_fatal;
use emit_pos_rust::emit_pos;
use env::{emitter::Emitter, jump_targets as jt, Env};
use hhbc_ast_rust::{self as hhbc_ast, Instruct};
use instruction_sequence::{instr, InstrSeq, Result};
use label::Label;
use label_rust as label;
use oxidized::pos::Pos;

use bitflags::bitflags;
use indexmap::IndexSet;
use std::collections::BTreeMap;

type LabelMap<'a> = BTreeMap<label::Id, &'a Instruct>;

pub(super) struct JumpInstructions<'a>(LabelMap<'a>);
impl JumpInstructions<'_> {
    pub(super) fn is_empty(&self) -> bool {
        self.0.is_empty()
    }

    /// Collects list of Ret* and non rewritten Break/Continue instructions inside try body.
    pub(super) fn collect<'a>(is: &'a InstrSeq, jt_gen: &mut jt::Gen) -> JumpInstructions<'a> {
        fn get_label_id(jt_gen: &mut jt::Gen, is_break: bool, level: Level) -> label::Id {
            use jt::ResolvedJumpTarget::*;
            match jt_gen.jump_targets().get_target_for_level(is_break, level) {
                ResolvedRegular(target_label, _)
                | ResolvedTryFinally(jt::ResolvedTryFinally { target_label, .. }) => {
                    jt_gen.get_id_for_label(target_label)
                }
                _ => panic!("impossible"),
            }
        }
        JumpInstructions(is.fold_left(
            &mut |mut acc: LabelMap<'a>, i| {
                use hhbc_ast::Instruct::*;
                use hhbc_ast::InstructControlFlow::{RetC, RetCSuspended, RetM};
                use hhbc_ast::InstructSpecialFlow::{Break, Continue};
                match i {
                    &ISpecialFlow(Break(level)) => {
                        acc.insert(get_label_id(jt_gen, true, level as Level), i);
                    }
                    &ISpecialFlow(Continue(level)) => {
                        acc.insert(get_label_id(jt_gen, false, level as Level), i);
                    }
                    &IContFlow(ref cont_flow) => match cont_flow {
                        RetC | RetCSuspended | RetM(_) => {
                            acc.insert(jt_gen.get_id_for_return(), i);
                        }
                        _ => {}
                    },
                    _ => {}
                };
                acc
            },
            LabelMap::new(),
        ))
    }
}

/// Delete Ret*, Break/Continue/Jmp(Named) instructions from the try body
pub(super) fn cleanup_try_body(is: &InstrSeq) -> InstrSeq {
    use hhbc_ast::Instruct::*;
    use hhbc_ast::InstructControlFlow::{RetC, RetCSuspended, RetM};
    is.filter_map(&mut |i| match i {
        &ISpecialFlow(_) => None,
        &IContFlow(ref cont_flow) => match cont_flow {
            RetC | RetCSuspended | RetM(_) => None,
            _ => Some(i.clone()),
        },
        _ => Some(i.clone()),
    })
}

pub(super) fn emit_jump_to_label(l: Label, iters: Vec<iterator::Id>) -> InstrSeq {
    if iters.is_empty() {
        instr::jmp(l)
    } else {
        instr::iter_break(l, iters)
    }
}

pub(super) fn emit_save_label_id(local_gen: &mut local::Gen, id: usize) -> InstrSeq {
    InstrSeq::gather(vec![
        instr::int(id as isize),
        instr::setl(local_gen.get_label().clone()),
        instr::popc(),
    ])
}

pub(super) fn emit_return(e: &mut Emitter, in_finally_epilogue: bool, env: &mut Env) -> Result {
    // check if there are try/finally region
    let jt_gen = &env.jump_targets_gen;
    match jt_gen.jump_targets().get_closest_enclosing_finally_label() {
        None => {
            // no finally blocks, but there might be some iterators that should be
            // released before exit - do it
            let ctx = e.emit_statement_state();
            let num_out = ctx.num_out;
            let verify_out = ctx.verify_out.clone();
            let verify_return = ctx.verify_return.clone();
            let release_iterators_instr = InstrSeq::gather(
                jt_gen
                    .jump_targets()
                    .iterators()
                    .map(|i| instr::iterfree(*i))
                    .collect(),
            );
            let mut instrs = Vec::with_capacity(5);
            if in_finally_epilogue {
                let load_retval_instr = instr::cgetl(e.local_gen_mut().get_retval().clone());
                instrs.push(load_retval_instr);
            }
            let verify_return_instr = verify_return.map_or_else(
                || Ok(instr::empty()),
                |h| {
                    use reified::ReificationLevel;
                    let h = reified::convert_awaitable(env, h.clone());
                    let h = reified::remove_erased_generics(env, h);
                    match reified::has_reified_type_constraint(env, &h) {
                        ReificationLevel::Unconstrained => Ok(instr::empty()),
                        ReificationLevel::Not => Ok(instr::verify_ret_type_c()),
                        ReificationLevel::Maybe => Ok(InstrSeq::gather(vec![
                            emit_expression::get_type_structure_for_hint(
                                e,
                                &[],
                                &IndexSet::new(),
                                &h,
                            )?,
                            instr::verify_ret_type_ts(),
                        ])),
                        ReificationLevel::Definitely => {
                            let check = InstrSeq::gather(vec![
                                instr::dup(),
                                instr::istypec(hhbc_ast::IstypeOp::OpNull),
                            ]);
                            reified::simplify_verify_type(
                                e,
                                env,
                                &Pos::make_none(),
                                check,
                                &h,
                                instr::verify_ret_type_ts(),
                            )
                        }
                    }
                },
            )?;
            instrs.extend(vec![
                verify_return_instr,
                verify_out,
                release_iterators_instr,
                if num_out != 0 {
                    instr::retm(num_out + 1)
                } else {
                    instr::retc()
                },
            ]);
            Ok(InstrSeq::gather(instrs))
        }
        // ret is in finally block and there might be iterators to release -
        // jump to finally block via Jmp
        Some((target_label, iterators_to_release)) => {
            let preamble = if in_finally_epilogue {
                instr::empty()
            } else {
                let jt_gen = &mut env.jump_targets_gen;
                let save_state = emit_save_label_id(e.local_gen_mut(), jt_gen.get_id_for_return());
                let save_retval = InstrSeq::gather(vec![
                    instr::setl(e.local_gen_mut().get_retval().clone()),
                    instr::popc(),
                ]);
                InstrSeq::gather(vec![save_state, save_retval])
            };
            Ok(InstrSeq::gather(vec![
                preamble,
                emit_jump_to_label(target_label, iterators_to_release),
                // emit ret instr as an indicator for try/finally rewriter to generate
                // finally epilogue, try/finally rewriter will remove it.
                instr::retc(),
            ]))
        }
    }
}

bitflags! {
    pub(super) struct EmitBreakOrContinueFlags: u8 {
        const IS_BREAK =            0b01;
        const IN_FINALLY_EPILOGUE = 0b10;
    }
}

pub(super) fn emit_break_or_continue(
    e: &mut Emitter,
    flags: EmitBreakOrContinueFlags,
    env: &mut Env,
    pos: &Pos,
    level: Level,
) -> InstrSeq {
    let jt_gen = &mut env.jump_targets_gen;
    let in_finally_epilogue = flags.contains(EmitBreakOrContinueFlags::IN_FINALLY_EPILOGUE);
    let is_break = flags.contains(EmitBreakOrContinueFlags::IS_BREAK);
    match jt_gen.jump_targets().get_target_for_level(is_break, level) {
        jt::ResolvedJumpTarget::NotFound => emit_fatal::emit_fatal_for_break_continue(pos, level),
        jt::ResolvedJumpTarget::ResolvedRegular(target_label, iterators_to_release) => {
            let preamble = if in_finally_epilogue && level == 1 {
                instr::unsetl(e.local_gen_mut().get_label().clone())
            } else {
                instr::empty()
            };
            InstrSeq::gather(vec![
                preamble,
                emit_pos(pos),
                emit_jump_to_label(target_label, iterators_to_release),
            ])
        }
        jt::ResolvedJumpTarget::ResolvedTryFinally(jt::ResolvedTryFinally {
            target_label,
            finally_label,
            iterators_to_release,
            adjusted_level,
        }) => {
            let preamble = if !in_finally_epilogue {
                let label_id = jt_gen.get_id_for_label(target_label.clone());
                emit_save_label_id(e.local_gen_mut(), label_id)
            } else {
                instr::empty()
            };
            let adjusted_level = adjusted_level as isize;
            InstrSeq::gather(vec![
                preamble,
                emit_jump_to_label(finally_label, iterators_to_release),
                emit_pos(pos),
                // emit break/continue instr as an indicator for try/finally rewriter
                // to generate finally epilogue - try/finally rewriter will remove it.
                if is_break {
                    instr::break_(adjusted_level)
                } else {
                    instr::continue_(adjusted_level)
                },
            ])
        }
    }
}

pub(super) fn emit_finally_epilogue(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    jump_instrs: JumpInstructions,
    finally_end: Label,
) -> Result {
    fn emit_instr(e: &mut Emitter, env: &mut Env, pos: &Pos, i: &Instruct) -> Result {
        use hhbc_ast::Instruct::*;
        use hhbc_ast::InstructControlFlow::{RetC, RetCSuspended, RetM};
        use hhbc_ast::InstructSpecialFlow::{Break, Continue};
        let fail = || {
            panic!("unexpected instruction: only Ret* or Break/Continue/Jmp(Named) are expected")
        };
        match i {
            &IContFlow(ref cont_flow) => match cont_flow {
                RetC | RetCSuspended | RetM(_) => emit_return(e, true, env),
                _ => fail(),
            },
            &ISpecialFlow(Break(level)) => Ok(emit_break_or_continue(
                e,
                EmitBreakOrContinueFlags::IS_BREAK | EmitBreakOrContinueFlags::IN_FINALLY_EPILOGUE,
                env,
                pos,
                level as Level,
            )),
            &ISpecialFlow(Continue(level)) => Ok(emit_break_or_continue(
                e,
                EmitBreakOrContinueFlags::IN_FINALLY_EPILOGUE,
                env,
                pos,
                level as Level,
            )),
            _ => fail(),
        }
    }
    Ok(if jump_instrs.0.is_empty() {
        instr::empty()
    } else if jump_instrs.0.len() == 1 {
        let (_, instr) = jump_instrs.0.iter().next().unwrap();
        InstrSeq::gather(vec![
            emit_pos(pos),
            instr::issetl(e.local_gen_mut().get_label().clone()),
            instr::jmpz(finally_end),
            emit_instr(e, env, pos, instr)?,
        ])
    } else {
        // mimic HHVM behavior:
        // in some cases ids can be non-consequtive - this might happen i.e. return statement
        //  appear in the block and it was assigned a high id before.
        //  ((3, Return), (1, Break), (0, Continue))
        //  In thid case generate switch as
        //  switch  (id) {
        //     L0 -> handler for continue
        //     L1 -> handler for break
        //     FinallyEnd -> empty
        //     L3 -> handler for return
        //  }
        //
        // This function builds a list of labels and jump targets for switch.
        // It is possible that cases ids are not consequtive
        // [L1,L2,L4]. Vector of labels in switch should be dense so we need to
        // fill holes with a label that points to the end of finally block
        // [End, L1, L2, End, L4]
        let (max_id, _) = jump_instrs.0.iter().next_back().unwrap();
        let (mut labels, mut bodies) = (vec![], vec![]);
        let mut n: isize = *max_id as isize;
        // lst is already sorted - BTreeMap/IMap bindings took care of it
        // TODO: add is_sorted assert to make sure this behavior is preserved for labels
        for (id, instr) in jump_instrs.0.into_iter().rev() {
            let mut done = false;
            while !done {
                // NOTE(hrust) looping is equivalent to recursing without consuming instr
                done = (id as isize) == n;
                let (label, body) = if done {
                    let label = e.label_gen_mut().next_regular();
                    let body = InstrSeq::gather(vec![
                        instr::label(label.clone()),
                        emit_instr(e, env, pos, instr)?,
                    ]);
                    (label, body)
                } else {
                    (finally_end.clone(), instr::empty())
                };
                labels.push(label);
                bodies.push(body);
                n -= 1;
            }
        }
        // NOTE(hrust): base case when empty and n >= 0
        for _ in 0..=n {
            labels.push(finally_end.clone());
            bodies.push(instr::empty());
        }
        InstrSeq::gather(vec![
            emit_pos(pos),
            instr::issetl(e.local_gen_mut().get_label().clone()),
            instr::jmpz(finally_end),
            instr::cgetl(e.local_gen_mut().get_label().clone()),
            instr::switch(labels.into_iter().rev().collect()),
            InstrSeq::gather(bodies.into_iter().rev().collect()),
        ])
    })
}

// TODO: This codegen is unnecessarily complex.  Basically we are generating
//
// IsSetL temp
// JmpZ   finally_end
// CGetL  temp
// Switch Unbounded 0 <L4 L5>
// L5:
// UnsetL temp
// Jmp LContinue
// L4:
// UnsetL temp
// Jmp LBreak
//
// Two problems immediately come to mind. First, why is the unset in every case,
// instead of after the CGetL?  Surely the unset doesn't modify the stack.
// Second, now we have a jump-to-jump situation.
//
// Would it not make more sense to instead say
//
// IsSetL temp
// JmpZ   finally_end
// CGetL  temp
// UnsetL temp
// Switch Unbounded 0 <LBreak LContinue>
//
// ?
