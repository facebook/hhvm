// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::emitter::Emitter;
use env::LabelGen;
use error::Result;
use hhbc::Instruct;
use hhbc::IterId;
use hhbc::Label;
use hhbc::Local;
use hhbc::Opcode;
use hhbc::Pseudo;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;

/// Run emit () in a new unnamed local scope, which produces three instruction
/// blocks -- before, inner, after. If emit () registered any unnamed locals, the
/// inner block will be wrapped in a try/catch that will unset these unnamed
/// locals upon exception.
pub fn with_unnamed_locals<'arena, 'decl, F>(
    e: &mut Emitter<'arena, 'decl>,
    emit: F,
) -> Result<InstrSeq<'arena>>
where
    F: FnOnce(
        &mut Emitter<'arena, 'decl>,
    ) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>)>,
{
    let local_counter = e.local_gen().counter;
    e.local_gen_mut().dedicated.temp_map.push();

    let (before, inner, after) = emit(e)?;

    e.local_gen_mut().dedicated.temp_map.pop();
    if local_counter == e.local_gen().counter {
        Ok(InstrSeq::gather(vec![before, inner, after]))
    } else {
        let unset_locals = unset_unnamed_locals(local_counter.next, e.local_gen().counter.next);
        e.local_gen_mut().counter = local_counter;
        Ok(wrap_inner_in_try_catch(
            e.label_gen_mut(),
            (before, inner, after),
            unset_locals,
        ))
    }
}

/// Run emit () in a new unnamed local and iterator scope, which produces three
/// instruction blocks -- before, inner, after. If emit () registered any unnamed
/// locals or iterators, the inner block will be wrapped in a try/catch that will
/// unset these unnamed locals and free these iterators upon exception.
pub fn with_unnamed_locals_and_iterators<'arena, 'decl, F>(
    e: &mut Emitter<'arena, 'decl>,
    emit: F,
) -> Result<InstrSeq<'arena>>
where
    F: FnOnce(
        &mut Emitter<'arena, 'decl>,
    ) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>)>,
{
    let local_counter = e.local_gen().counter;
    e.local_gen_mut().dedicated.temp_map.push();
    let next_iterator = e.iterator().next;

    let (before, inner, after) = emit(e)?;

    e.local_gen_mut().dedicated.temp_map.pop();

    if local_counter == e.local_gen().counter && next_iterator == e.iterator().next {
        Ok(InstrSeq::gather(vec![before, inner, after]))
    } else {
        let unset_locals = unset_unnamed_locals(local_counter.next, e.local_gen().counter.next);
        e.local_gen_mut().counter = local_counter;
        let free_iters = free_iterators(next_iterator, e.iterator().next);
        e.iterator_mut().next = next_iterator;
        Ok(wrap_inner_in_try_catch(
            e.label_gen_mut(),
            (before, inner, after),
            InstrSeq::gather(vec![unset_locals, free_iters]),
        ))
    }
}

/// An equivalent of with_unnamed_locals that allocates a single local and
/// passes it to emit
pub fn with_unnamed_local<'arena, 'decl, F>(
    e: &mut Emitter<'arena, 'decl>,
    emit: F,
) -> Result<InstrSeq<'arena>>
where
    F: FnOnce(
        &mut Emitter<'arena, 'decl>,
        Local,
    ) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>)>,
{
    with_unnamed_locals(e, |e| {
        let tmp = e.local_gen_mut().get_unnamed();
        emit(e, tmp)
    })
}

pub fn stash_top_in_unnamed_local<'arena, 'decl, F>(
    e: &mut Emitter<'arena, 'decl>,
    emit: F,
) -> Result<InstrSeq<'arena>>
where
    F: FnOnce(&mut Emitter<'arena, 'decl>) -> Result<InstrSeq<'arena>>,
{
    with_unnamed_locals(e, |e| {
        let tmp = e.local_gen_mut().get_unnamed();
        Ok((instr::pop_l(tmp.clone()), emit(e)?, instr::push_l(tmp)))
    })
}

fn unset_unnamed_locals<'arena>(start: Local, end: Local) -> InstrSeq<'arena> {
    InstrSeq::gather(
        (start.idx..end.idx)
            .into_iter()
            .map(|idx| instr::unset_l(Local::new(idx as usize)))
            .collect(),
    )
}

fn free_iterators<'arena>(start: IterId, end: IterId) -> InstrSeq<'arena> {
    InstrSeq::gather(
        (start.idx..end.idx)
            .into_iter()
            .map(|idx| instr::iter_free(IterId { idx }))
            .collect(),
    )
}

fn wrap_inner_in_try_catch<'arena>(
    label_gen: &mut LabelGen,
    (before, inner, after): (InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>),
    catch_instrs: InstrSeq<'arena>,
) -> InstrSeq<'arena> {
    InstrSeq::gather(vec![
        before,
        create_try_catch(label_gen, None, false, inner, catch_instrs),
        after,
    ])
}

pub fn create_try_catch<'a>(
    label_gen: &mut LabelGen,
    opt_done_label: Option<Label>,
    skip_throw: bool,
    try_instrs: InstrSeq<'a>,
    catch_instrs: InstrSeq<'a>,
) -> InstrSeq<'a> {
    let done_label = match opt_done_label {
        Some(l) => l,
        None => label_gen.next_regular(),
    };
    InstrSeq::gather(vec![
        instr::instr(Instruct::Pseudo(Pseudo::TryCatchBegin)),
        try_instrs,
        instr::jmp(done_label),
        instr::instr(Instruct::Pseudo(Pseudo::TryCatchMiddle)),
        catch_instrs,
        if skip_throw {
            instr::empty()
        } else {
            instr::instr(Instruct::Opcode(Opcode::Throw))
        },
        instr::instr(Instruct::Pseudo(Pseudo::TryCatchEnd)),
        instr::label(done_label),
    ])
}
