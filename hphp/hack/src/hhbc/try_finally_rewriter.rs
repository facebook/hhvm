// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use super::*; // reimport from emit_statement

use ast_scope_rust as ast_scope;
use emit_pos_rust::emit_pos;
use env::iterator::Iter;

use bitflags::bitflags;

use std::{borrow::Cow, collections::BTreeMap};

type LabelMap<'a> = BTreeMap<label::Id, &'a hhbc_ast::Instruct>;

pub(super) struct JumpInstructions<'a>(LabelMap<'a>);
impl JumpInstructions<'_> {
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
                use hhbc_ast::InstructSpecialFlow::{Break, Continue, Goto};
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
                        _ => (),
                    },
                    &ISpecialFlow(Goto(ref l)) => {
                        acc.insert(jt_gen.get_id_for_label(Label::Named(l.clone())), i);
                    }
                    _ => (),
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

pub(super) fn emit_jump_to_label(l: Label, iters: Vec<Iter>) -> InstrSeq {
    if iters.is_empty() {
        InstrSeq::make_jmp(l)
    } else {
        InstrSeq::make_iter_break(l, iters)
    }
}

pub(super) fn emit_save_label_id(local_gen: &mut local::Gen, id: usize) -> InstrSeq {
    InstrSeq::gather(vec![
        InstrSeq::make_int(id as isize),
        InstrSeq::make_setl(local_gen.get_label().clone()),
        InstrSeq::make_popc(),
    ])
}

fn get_pos_for_error(env: &Env) -> Cow<Pos> {
    for item in env.scope.iter() {
        use ast_scope::ScopeItem;
        match item {
            ScopeItem::Function(fd) => return Pos::first_char_of_line(&fd.span),
            // For methods, it points to class not the method.. weird
            ScopeItem::Class(cd) => return Pos::first_char_of_line(&cd.span),
            ScopeItem::Method(_) | ScopeItem::Lambda(_) | ScopeItem::LongLambda(_) => (),
        }
    }
    Cow::Owned(Pos::make_none())
}

fn emit_goto(
    in_finally_epilogue: bool,
    label: String,
    env: &mut Env,
    local_gen: &mut local::Gen,
) -> Result<InstrSeq, emit_fatal::Error> {
    let in_using_opt = env
        .jump_targets_gen
        .get_labels_in_function()
        .get(&label)
        .copied();
    match in_using_opt {
        None => Err(emit_fatal::raise_fatal_parse(
            &get_pos_for_error(env),
            format!("'goto' to undefined label '{}'", label),
        )),
        Some(in_using) => {
            // CONSIDER: we don't need to assign state id for label
            // for cases when it is not necessary, i.e. when jump target is in the same
            // scope. HHVM does not do this today, do the same for compatibility reasons
            let jt_gen = &mut env.jump_targets_gen;
            match jt_gen.jump_targets().find_goto_target(&label) {
                jt::ResolvedGotoTarget::Label(iters) => {
                    let preamble = if !in_finally_epilogue {
                        InstrSeq::make_empty()
                    } else {
                        InstrSeq::make_unsetl(local_gen.get_label().clone())
                    };
                    Ok(InstrSeq::gather(vec![
                        preamble,
                        emit_jump_to_label(Label::Named(label), iters),
                    ]))
                }
                jt::ResolvedGotoTarget::Finally(jt::ResolvedGotoFinally {
                    rgf_finally_start_label,
                    rgf_iterators_to_release,
                }) => {
                    let preamble = if in_finally_epilogue {
                        InstrSeq::make_empty()
                    } else {
                        let label_id = jt_gen.get_id_for_label(Label::Named(label.clone()));
                        emit_save_label_id(local_gen, label_id)
                    };
                    Ok(InstrSeq::gather(vec![
                        preamble,
                        emit_jump_to_label(rgf_finally_start_label, rgf_iterators_to_release),
                        // emit goto as an indicator for try/finally rewriter to generate
                        // finally epilogue, try/finally rewriter will remove it.
                        InstrSeq::make_goto(label),
                    ]))
                }
                jt::ResolvedGotoTarget::GotoFromFinally => Err(emit_fatal::raise_fatal_parse(
                    &get_pos_for_error(env),
                    "Goto to a label outside a finally block is not supported".into(),
                )),
                jt::ResolvedGotoTarget::GotoInvalidLabel => Err(emit_fatal::raise_fatal_parse(
                    &get_pos_for_error(env),
                    String::from(if in_using {
                        "'goto' into or across using statement is disallowed"
                    } else {
                        "'goto' into loop or switch statement is disallowed"
                    }),
                )),
            }
        }
    }
}

pub(super) fn emit_return(
    _e: &Emitter,
    _verify_return: &Option<a::Hint>,
    _verify_out: &InstrSeq,
    _num_out: usize,
    _in_finally_epilogue: bool,
    _env: &Env,
) -> InstrSeq {
    unimplemented!("TODO(hrust) port reified_generics_helpers first")
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
        jt::ResolvedJumpTarget::NotFound => {
            emit_fatal::emit_fatal_for_break_continue(e, pos, level)
        }
        jt::ResolvedJumpTarget::ResolvedRegular(target_label, iterators_to_release) => {
            let preamble = if in_finally_epilogue && level == 1 {
                InstrSeq::make_unsetl(e.local_gen_mut().get_label().clone())
            } else {
                InstrSeq::make_empty()
            };
            InstrSeq::gather(vec![
                preamble,
                emit_pos(e, pos),
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
                InstrSeq::make_empty()
            };
            let adjusted_level = adjusted_level as isize;
            InstrSeq::gather(vec![
                preamble,
                emit_jump_to_label(finally_label, iterators_to_release),
                emit_pos(e, pos),
                // emit break/continue instr as an indicator for try/finally rewriter
                // to generate finally epilogue - try/finally rewriter will remove it.
                if is_break {
                    InstrSeq::make_break(adjusted_level)
                } else {
                    InstrSeq::make_continue(adjusted_level)
                },
            ])
        }
    }
}

fn emit_finally_epilogue(
    e: &Emitter,
    _env: &Env,
    _jump_instrs: (),
    _finally_end: Label,
) -> InstrSeq {
    let ctx = e.emit_state();
    let _verify_return = &ctx.verify_return;
    unimplemented!("TODO(hrust) blocked on porting emit_return")
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
