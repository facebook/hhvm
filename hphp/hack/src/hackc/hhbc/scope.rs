// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::emitter::Emitter;
use instruction_sequence::{instr, InstrSeq, Result};
use local::Local;

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
        Ok(InstrSeq::gather(e.alloc, vec![before, inner, after]))
    } else {
        let unset_locals = unset_unnamed_locals(e.alloc, local_counter.0, e.local_gen().counter.0);
        e.local_gen_mut().counter = local_counter;
        Ok(wrap_inner_in_try_catch(
            e.alloc,
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
        Ok(InstrSeq::gather(e.alloc, vec![before, inner, after]))
    } else {
        let unset_locals = unset_unnamed_locals(e.alloc, local_counter.0, e.local_gen().counter.0);
        e.local_gen_mut().counter = local_counter;
        let free_iters = free_iterators(e.alloc, next_iterator, e.iterator().next);
        e.iterator_mut().next = next_iterator;
        let alloc = e.alloc;
        Ok(wrap_inner_in_try_catch(
            e.alloc,
            e.label_gen_mut(),
            (before, inner, after),
            InstrSeq::gather(alloc, vec![unset_locals, free_iters]),
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
        Local<'arena>,
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
        Ok((
            instr::popl(e.alloc, tmp.clone()),
            emit(e)?,
            instr::pushl(e.alloc, tmp),
        ))
    })
}

fn unset_unnamed_locals<'arena>(
    alloc: &'arena bumpalo::Bump,
    start: local::Id,
    end: local::Id,
) -> InstrSeq<'arena> {
    InstrSeq::gather(
        alloc,
        (start..end)
            .into_iter()
            .map(|id| instr::unsetl(alloc, Local::Unnamed(id)))
            .collect(),
    )
}

fn free_iterators<'arena>(
    alloc: &'arena bumpalo::Bump,
    start: iterator::Id,
    end: iterator::Id,
) -> InstrSeq<'arena> {
    InstrSeq::gather(
        alloc,
        (start.0..end.0)
            .into_iter()
            .map(|i| instr::iterfree(alloc, iterator::Id(i)))
            .collect(),
    )
}

fn wrap_inner_in_try_catch<'arena>(
    alloc: &'arena bumpalo::Bump,
    label_gen: &mut label::Gen,
    (before, inner, after): (InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>),
    catch_instrs: InstrSeq<'arena>,
) -> InstrSeq<'arena> {
    InstrSeq::gather(
        alloc,
        vec![
            before,
            InstrSeq::create_try_catch(alloc, label_gen, None, false, inner, catch_instrs),
            after,
        ],
    )
}
