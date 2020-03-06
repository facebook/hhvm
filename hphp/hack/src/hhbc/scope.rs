// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub mod scope {
    use env::{emitter::Emitter, iterator::Iter, local};
    use instruction_sequence_rust::{InstrSeq, Result};
    use label_rust as label;

    /// Run emit () in a new unnamed local scope, which produces three instruction
    /// blocks -- before, inner, after. If emit () registered any unnamed locals, the
    /// inner block will be wrapped in a try/catch that will unset these unnamed
    /// locals upon exception.
    pub fn with_unnamed_locals<F>(emitter: &mut Emitter, emit: F) -> Result
    where
        F: FnOnce(&mut Emitter) -> Result<(InstrSeq, InstrSeq, InstrSeq)>,
    {
        emitter.local_gen_mut().store_current_state();

        let (before, inner, after) = emit(emitter)?;

        if !emitter.local_gen().state_has_changed() {
            emitter.local_gen_mut().revert_state();
            Ok(InstrSeq::gather(vec![before, inner, after]))
        } else {
            let local_ids_to_unset = emitter.local_gen_mut().revert_state();
            let unset_locals = unset_unnamed_locals(local_ids_to_unset);
            Ok(wrap_inner_in_try_catch(
                emitter.label_gen_mut(),
                (before, inner, after),
                unset_locals,
            ))
        }
    }

    /// Run emit () in a new unnamed local and iterator scope, which produces three
    /// instruction blocks -- before, inner, after. If emit () registered any unnamed
    /// locals or iterators, the inner block will be wrapped in a try/catch that will
    /// unset these unnamed locals and free these iterators upon exception.
    pub fn with_unnamed_locals_and_iterators<F>(emitter: &mut Emitter, emit: F) -> Result
    where
        F: FnOnce(&mut Emitter) -> Result<(InstrSeq, InstrSeq, InstrSeq)>,
    {
        let stateref = emitter.refined_state_mut();
        let (next_local, next_iterator) = (stateref.local_gen, stateref.iterator);
        next_local.store_current_state();
        next_iterator.store_current_state();

        let (before, inner, after) = emit(emitter)?;

        let stateref = emitter.refined_state_mut();
        let (next_local, next_iterator) = (stateref.local_gen, stateref.iterator);
        if !next_local.state_has_changed() && !next_iterator.state_has_changed() {
            next_local.revert_state();
            Ok(InstrSeq::gather(vec![before, inner, after]))
        } else {
            let local_ids_to_unset = next_local.revert_state();
            let iters_to_free = next_iterator.revert_state();
            let unset_locals = unset_unnamed_locals(local_ids_to_unset);
            let free_iters = free_iterators(iters_to_free);
            Ok(wrap_inner_in_try_catch(
                stateref.label_gen,
                (before, inner, after),
                InstrSeq::gather(vec![unset_locals, free_iters]),
            ))
        }
    }

    /// An equivalent of with_unnamed_locals that allocates a single local and
    /// passes it to emit
    pub fn with_unnamed_local<F>(emitter: &mut Emitter, emit: F) -> Result
    where
        F: FnOnce(&mut Emitter, local::Type) -> Result<(InstrSeq, InstrSeq, InstrSeq)>,
    {
        let next_local = emitter.local_gen_mut();
        next_local.store_current_state();
        let local = next_local.get_unnamed();

        let (before, inner, after) = emit(emitter, local)?;

        if !emitter.local_gen().state_has_changed() {
            Ok(InstrSeq::gather(vec![before, inner, after]))
        } else {
            let local_ids_to_unset = emitter.local_gen_mut().revert_state();
            let unset_locals = unset_unnamed_locals(local_ids_to_unset);
            Ok(wrap_inner_in_try_catch(
                emitter.label_gen_mut(),
                (before, inner, after),
                unset_locals,
            ))
        }
    }

    pub fn stash_top_in_unnamed_local<F>(emitter: &mut Emitter, emit: F) -> InstrSeq
    where
        F: FnOnce() -> InstrSeq,
    {
        let next_local = emitter.local_gen_mut();
        next_local.store_current_state();

        // Pop the top of the stack into an unnamed local, run emit (), then push the
        // stashed value to the top of the stack
        let tmp = next_local.get_unnamed();
        let (before, inner, after) = (
            InstrSeq::make_popl(tmp.clone()),
            emit(),
            InstrSeq::make_pushl(tmp.clone()),
        );

        if !next_local.state_has_changed() {
            InstrSeq::gather(vec![before, inner, after])
        } else {
            let local_ids_to_unset = next_local.revert_state();
            let unset_locals = unset_unnamed_locals(local_ids_to_unset);
            wrap_inner_in_try_catch(
                emitter.label_gen_mut(),
                (before, inner, after),
                unset_locals,
            )
        }
    }

    fn unset_unnamed_locals(ids: Vec<local::Id>) -> InstrSeq {
        InstrSeq::gather(
            ids.into_iter()
                .map(|id| InstrSeq::make_unsetl(local::Type::Unnamed(id)))
                .collect(),
        )
    }

    fn free_iterators(iters: Vec<Iter>) -> InstrSeq {
        InstrSeq::gather(
            iters
                .into_iter()
                .map(|i| InstrSeq::make_iterfree(i.next))
                .collect(),
        )
    }

    fn wrap_inner_in_try_catch(
        label_gen: &mut label::Gen,
        (before, inner, after): (InstrSeq, InstrSeq, InstrSeq),
        catch_instrs: InstrSeq,
    ) -> InstrSeq {
        InstrSeq::gather(vec![
            before,
            InstrSeq::create_try_catch(label_gen, None, false, inner, catch_instrs),
            after,
        ])
    }
}
