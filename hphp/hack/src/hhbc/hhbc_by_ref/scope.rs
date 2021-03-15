// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub mod scope {
    use hhbc_by_ref_env::emitter::Emitter;
    use hhbc_by_ref_instruction_sequence::{instr, InstrSeq, Result};
    use hhbc_by_ref_iterator as iterator;
    use hhbc_by_ref_label as label;
    use hhbc_by_ref_local as local;

    /// Run emit () in a new unnamed local scope, which produces three instruction
    /// blocks -- before, inner, after. If emit () registered any unnamed locals, the
    /// inner block will be wrapped in a try/catch that will unset these unnamed
    /// locals upon exception.
    pub fn with_unnamed_locals<'arena, F>(
        alloc: &'arena bumpalo::Bump,
        e: &mut Emitter<'arena>,
        emit: F,
    ) -> Result<InstrSeq<'arena>>
    where
        F: FnOnce(
            &'arena bumpalo::Bump,
            &mut Emitter<'arena>,
        ) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>)>,
    {
        let local_counter = e.local_gen().counter;
        e.local_gen_mut().dedicated.temp_map.push();

        let (before, inner, after) = emit(alloc, e)?;

        e.local_gen_mut().dedicated.temp_map.pop();
        if local_counter == e.local_gen().counter {
            Ok(InstrSeq::gather(alloc, vec![before, inner, after]))
        } else {
            let unset_locals =
                unset_unnamed_locals(alloc, local_counter.0, e.local_gen().counter.0);
            e.local_gen_mut().counter = local_counter;
            Ok(wrap_inner_in_try_catch(
                alloc,
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
    pub fn with_unnamed_locals_and_iterators<'arena, F>(
        alloc: &'arena bumpalo::Bump,
        e: &mut Emitter<'arena>,
        emit: F,
    ) -> Result<InstrSeq<'arena>>
    where
        F: FnOnce(
            &'arena bumpalo::Bump,
            &mut Emitter<'arena>,
        ) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>)>,
    {
        let local_counter = e.local_gen().counter;
        e.local_gen_mut().dedicated.temp_map.push();
        let next_iterator = e.iterator().next;

        let (before, inner, after) = emit(alloc, e)?;

        e.local_gen_mut().dedicated.temp_map.pop();

        if local_counter == e.local_gen().counter && next_iterator == e.iterator().next {
            Ok(InstrSeq::gather(alloc, vec![before, inner, after]))
        } else {
            let unset_locals =
                unset_unnamed_locals(alloc, local_counter.0, e.local_gen().counter.0);
            e.local_gen_mut().counter = local_counter;
            let free_iters = free_iterators(alloc, next_iterator, e.iterator().next);
            e.iterator_mut().next = next_iterator;
            Ok(wrap_inner_in_try_catch(
                alloc,
                e.label_gen_mut(),
                (before, inner, after),
                InstrSeq::gather(alloc, vec![unset_locals, free_iters]),
            ))
        }
    }

    /// An equivalent of with_unnamed_locals that allocates a single local and
    /// passes it to emit
    pub fn with_unnamed_local<'arena, F>(
        alloc: &'arena bumpalo::Bump,
        e: &mut Emitter<'arena>,
        emit: F,
    ) -> Result<InstrSeq<'arena>>
    where
        F: FnOnce(
            &'arena bumpalo::Bump,
            &mut Emitter<'arena>,
            local::Type<'arena>,
        ) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>, InstrSeq<'arena>)>,
    {
        with_unnamed_locals(alloc, e, |alloc, e| {
            let tmp = e.local_gen_mut().get_unnamed();
            emit(alloc, e, tmp)
        })
    }

    pub fn stash_top_in_unnamed_local<'arena, F>(
        alloc: &'arena bumpalo::Bump,
        e: &mut Emitter<'arena>,
        emit: F,
    ) -> Result<InstrSeq<'arena>>
    where
        F: FnOnce(&'arena bumpalo::Bump, &mut Emitter<'arena>) -> Result<InstrSeq<'arena>>,
    {
        with_unnamed_locals(alloc, e, |alloc, e| {
            let tmp = e.local_gen_mut().get_unnamed();
            Ok((
                instr::popl(alloc, tmp.clone()),
                emit(alloc, e)?,
                instr::pushl(alloc, tmp),
            ))
        })
    }

    #[allow(clippy::needless_lifetimes)]
    fn unset_unnamed_locals<'arena>(
        alloc: &'arena bumpalo::Bump,
        start: local::Id,
        end: local::Id,
    ) -> InstrSeq<'arena> {
        InstrSeq::gather(
            alloc,
            (start..end)
                .into_iter()
                .map(|id| instr::unsetl(alloc, local::Type::Unnamed(id)))
                .collect(),
        )
    }

    #[allow(clippy::needless_lifetimes)]
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
}
