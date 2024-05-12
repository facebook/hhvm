// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::BTreeMap;

use bitflags::bitflags;
use emit_pos::emit_pos;
use env::emitter::Emitter;
use env::jump_targets as jt;
use env::Env;
use env::LocalGen;
use error::Result;
use hhbc::Instruct;
use hhbc::IsTypeOp;
use hhbc::IterId;
use hhbc::Label;
use hhbc::Opcode;
use hhbc::Pseudo;
use indexmap::IndexSet;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::pos::Pos;

use super::TypeRefinementInHint;
use crate::emit_expression;
use crate::emit_fatal;
use crate::reified_generics_helpers as reified;

type LabelMap<'i> = BTreeMap<jt::StateId, &'i Instruct>;

pub(super) struct JumpInstructions<'i>(LabelMap<'i>);
impl<'i> JumpInstructions<'i> {
    pub(super) fn is_empty(&self) -> bool {
        self.0.is_empty()
    }

    /// Collects list of Ret* and non rewritten Break/Continue instructions inside try body.
    pub(super) fn collect(instr_seq: &'i InstrSeq, jt_gen: &mut jt::Gen) -> JumpInstructions<'i> {
        fn get_label_id(jt_gen: &mut jt::Gen, is_break: bool) -> jt::StateId {
            use jt::ResolvedJumpTarget;
            match jt_gen.jump_targets().get_target(is_break) {
                ResolvedJumpTarget::ResolvedRegular(target_label, _)
                | ResolvedJumpTarget::ResolvedTryFinally(jt::ResolvedTryFinally {
                    target_label,
                    ..
                }) => jt_gen.get_id_for_label(target_label),
                ResolvedJumpTarget::NotFound => unreachable!(),
            }
        }
        JumpInstructions(instr_seq.iter().fold(LabelMap::new(), |mut acc, instr| {
            match *instr {
                Instruct::Pseudo(Pseudo::Break) => {
                    acc.insert(get_label_id(jt_gen, true), instr);
                }
                Instruct::Pseudo(Pseudo::Continue) => {
                    acc.insert(get_label_id(jt_gen, false), instr);
                }
                Instruct::Opcode(Opcode::RetC | Opcode::RetCSuspended | Opcode::RetM(_)) => {
                    acc.insert(jt_gen.get_id_for_return(), instr);
                }
                _ => {}
            };
            acc
        }))
    }
}

/// Delete Ret*, Break, and Continue instructions from the try body
pub(super) fn cleanup_try_body(mut is: InstrSeq) -> InstrSeq {
    is.retain(|instr| {
        !matches!(
            instr,
            Instruct::Pseudo(Pseudo::Continue | Pseudo::Break)
                | Instruct::Opcode(Opcode::RetC | Opcode::RetCSuspended | Opcode::RetM(_))
        )
    });
    is
}

pub(super) fn emit_jump_to_label(l: Label, iters: Vec<IterId>) -> InstrSeq {
    if iters.is_empty() {
        instr::jmp(l)
    } else {
        instr::iter_break(l, iters)
    }
}

pub(super) fn emit_save_label_id(local_gen: &mut LocalGen, id: jt::StateId) -> InstrSeq {
    InstrSeq::gather(vec![
        instr::int(id.0.into()),
        instr::set_l(*local_gen.get_label()),
        instr::pop_c(),
    ])
}

pub(super) fn emit_return<'a>(
    e: &mut Emitter<'_>,
    in_finally_epilogue: bool,
    env: &mut Env<'a>,
) -> Result<InstrSeq> {
    // check if there are try/finally region
    let jt_gen = &env.jump_targets_gen;
    match jt_gen.jump_targets().get_closest_enclosing_finally_label() {
        None => {
            // no finally blocks, but there might be some iterators that should be
            // released before exit - do it
            let ctx = e.statement_state();
            let num_out = ctx.num_out;
            let verify_out = ctx.verify_out.clone();
            let verify_return = ctx.verify_return.clone();
            let release_iterators_instr = InstrSeq::gather(
                jt_gen
                    .jump_targets()
                    .iterators()
                    .map(instr::iter_free)
                    .collect(),
            );
            let mut instrs = Vec::with_capacity(5);
            if in_finally_epilogue {
                let load_retval_instr = instr::c_get_l(e.local_gen_mut().get_retval().clone());
                instrs.push(load_retval_instr);
            }
            let verify_return_instr = verify_return.map_or_else(
                || Ok(instr::empty()),
                |h| {
                    use reified::ReificationLevel;
                    let h = reified::convert_awaitable(env, h);
                    let h = reified::remove_erased_generics(env, h);
                    match reified::has_reified_type_constraint(env, &h) {
                        ReificationLevel::Unconstrained => Ok(instr::empty()),
                        ReificationLevel::Not => Ok(instr::verify_ret_type_c()),
                        ReificationLevel::Maybe => Ok(InstrSeq::gather(vec![
                            emit_expression::get_type_structure_for_hint(
                                e,
                                &[],
                                &IndexSet::new(),
                                TypeRefinementInHint::Allowed,
                                &h,
                            )?,
                            instr::verify_ret_type_ts(),
                        ])),
                        ReificationLevel::Definitely => {
                            let check = InstrSeq::gather(vec![
                                instr::dup(),
                                instr::is_type_c(IsTypeOp::Null),
                            ]);
                            reified::simplify_verify_type(
                                e,
                                env,
                                &Pos::NONE,
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
                    instr::ret_m(num_out as u32 + 1)
                } else {
                    instr::ret_c()
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
                    instr::set_l(e.local_gen_mut().get_retval().clone()),
                    instr::pop_c(),
                ]);
                InstrSeq::gather(vec![save_state, save_retval])
            };
            Ok(InstrSeq::gather(vec![
                preamble,
                emit_jump_to_label(target_label, iterators_to_release),
                // emit ret instr as an indicator for try/finally rewriter to generate
                // finally epilogue, try/finally rewriter will remove it.
                instr::ret_c(),
            ]))
        }
    }
}

bitflags! {
    #[derive(PartialEq, Eq, PartialOrd, Ord, Hash, Debug, Clone, Copy)]
    pub(super) struct EmitBreakOrContinueFlags: u8 {
        const IS_BREAK =            0b01;
        const IN_FINALLY_EPILOGUE = 0b10;
    }
}

pub(super) fn emit_break_or_continue<'a>(
    e: &mut Emitter<'_>,
    flags: EmitBreakOrContinueFlags,
    env: &mut Env<'a>,
    pos: &Pos,
) -> InstrSeq {
    let jt_gen = &mut env.jump_targets_gen;
    let in_finally_epilogue = flags.contains(EmitBreakOrContinueFlags::IN_FINALLY_EPILOGUE);
    let is_break = flags.contains(EmitBreakOrContinueFlags::IS_BREAK);
    match jt_gen.jump_targets().get_target(is_break) {
        jt::ResolvedJumpTarget::NotFound => emit_fatal::emit_fatal_for_break_continue(pos),
        jt::ResolvedJumpTarget::ResolvedRegular(target_label, iterators_to_release) => {
            let preamble = if in_finally_epilogue {
                instr::unset_l(e.local_gen_mut().get_label().clone())
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
        }) => {
            let preamble = if !in_finally_epilogue {
                let label_id = jt_gen.get_id_for_label(target_label.clone());
                emit_save_label_id(e.local_gen_mut(), label_id)
            } else {
                instr::empty()
            };
            InstrSeq::gather(vec![
                preamble,
                emit_jump_to_label(finally_label, iterators_to_release),
                emit_pos(pos),
                // emit break/continue instr as an indicator for try/finally rewriter
                // to generate finally epilogue - try/finally rewriter will remove it.
                if is_break {
                    instr::break_()
                } else {
                    instr::continue_()
                },
            ])
        }
    }
}

pub(super) fn emit_finally_epilogue<'a, 'd>(
    e: &mut Emitter<'d>,
    env: &mut Env<'a>,
    pos: &Pos,
    jump_instrs: JumpInstructions<'_>,
    finally_end: Label,
) -> Result<InstrSeq> {
    fn emit_instr<'a, 'd>(
        e: &mut Emitter<'d>,
        env: &mut Env<'a>,
        pos: &Pos,
        i: &Instruct,
    ) -> Result<InstrSeq> {
        let fail = || {
            panic!("unexpected instruction: only Ret* or Break/Continue/Jmp(Named) are expected")
        };
        match *i {
            Instruct::Opcode(Opcode::RetC | Opcode::RetCSuspended | Opcode::RetM(_)) => {
                emit_return(e, true, env)
            }
            Instruct::Pseudo(Pseudo::Break) => Ok(emit_break_or_continue(
                e,
                EmitBreakOrContinueFlags::IS_BREAK | EmitBreakOrContinueFlags::IN_FINALLY_EPILOGUE,
                env,
                pos,
            )),
            Instruct::Pseudo(Pseudo::Continue) => Ok(emit_break_or_continue(
                e,
                EmitBreakOrContinueFlags::IN_FINALLY_EPILOGUE,
                env,
                pos,
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
            instr::isset_l(e.local_gen_mut().get_label().clone()),
            instr::jmp_z(finally_end),
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
        let mut limit = max_id.0 + 1;
        // jump_instrs is already sorted - BTreeMap/IMap bindings took care of it
        // TODO: add is_sorted assert to make sure this behavior is preserved for labels
        for (id, instr) in jump_instrs.0.into_iter().rev() {
            loop {
                limit -= 1;
                // Looping is equivalent to recursing without consuming instr
                if id.0 == limit {
                    let label = e.label_gen_mut().next_regular();
                    let body = InstrSeq::gather(vec![
                        instr::label(label.clone()),
                        emit_instr(e, env, pos, instr)?,
                    ]);
                    labels.push(label);
                    bodies.push(body);
                    break;
                } else {
                    labels.push(finally_end);
                };
            }
        }
        // Base case when empty and limit > 0
        for _ in 0..limit {
            labels.push(finally_end);
        }
        InstrSeq::gather(vec![
            emit_pos(pos),
            instr::isset_l(e.local_gen_mut().get_label().clone()),
            instr::jmp_z(finally_end),
            instr::c_get_l(e.local_gen_mut().get_label().clone()),
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
