// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub mod scope {
    use env::{emitter::Emitter, iterator::Iter};
    use instruction_sequence_rust::Instr;
    use label_rust as label;
    use local_rust as local;

    /// Run emit () in a new unnamed local scope, which produces three instruction
    /// blocks -- before, inner, after. If emit () registered any unnamed locals, the
    /// inner block will be wrapped in a try/catch that will unset these unnamed
    /// locals upon exception.
    pub fn with_unnamed_locals<F>(emitter: &mut Emitter, emit: F) -> Instr
    where
        F: FnOnce() -> (Instr, Instr, Instr),
    {
        let next_local = emitter.local_gen_mut();
        next_local.store_current_state();

        let (before, inner, after) = emit();

        if !next_local.state_has_changed() {
            Instr::gather(vec![before, inner, after])
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

    /// Run emit () in a new unnamed local and iterator scope, which produces three
    /// instruction blocks -- before, inner, after. If emit () registered any unnamed
    /// locals or iterators, the inner block will be wrapped in a try/catch that will
    /// unset these unnamed locals and free these iterators upon exception.
    pub fn with_unnamed_locals_and_iterators<F>(emitter: &mut Emitter, emit: F) -> Instr
    where
        F: FnOnce() -> (Instr, Instr, Instr),
    {
        let stateref = emitter.refined_state_mut();
        let (next_local, next_iterator) = (stateref.local_gen, stateref.iterator);
        next_local.store_current_state();
        next_iterator.store_current_state();

        let (before, inner, after) = emit();

        if !next_local.state_has_changed() && !next_iterator.state_has_changed() {
            Instr::gather(vec![before, inner, after])
        } else {
            let local_ids_to_unset = next_local.revert_state();
            let iters_to_free = next_iterator.revert_state();
            let unset_locals = unset_unnamed_locals(local_ids_to_unset);
            let free_iters = free_iterators(iters_to_free);
            wrap_inner_in_try_catch(
                stateref.label_gen,
                (before, inner, after),
                Instr::gather(vec![unset_locals, free_iters]),
            )
        }
    }

    /// An equivalent of with_unnamed_locals that allocates a single local and
    /// passes it to emit
    pub fn with_unnamed_local<F>(emitter: &mut Emitter, emit: F) -> Instr
    where
        F: FnOnce(local::Type) -> (Instr, Instr, Instr),
    {
        let next_local = emitter.local_gen_mut();
        next_local.store_current_state();

        let (before, inner, after) = emit(next_local.get_unnamed());

        if !next_local.state_has_changed() {
            Instr::gather(vec![before, inner, after])
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

    pub fn stash_top_in_unnamed_local<F>(emitter: &mut Emitter, emit: F) -> Instr
    where
        F: FnOnce() -> Instr,
    {
        let next_local = emitter.local_gen_mut();
        next_local.store_current_state();

        // Pop the top of the stack into an unnamed local, run emit (), then push the
        // stashed value to the top of the stack
        let tmp = next_local.get_unnamed();
        let (before, inner, after) = (
            Instr::make_instr_popl(tmp.clone()),
            emit(),
            Instr::make_instr_pushl(tmp.clone()),
        );

        if !next_local.state_has_changed() {
            Instr::gather(vec![before, inner, after])
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

    fn unset_unnamed_locals(ids: Vec<local::Id>) -> Instr {
        Instr::gather(
            ids.into_iter()
                .map(|id| Instr::make_instr_unsetl(local::Type::Unnamed(id)))
                .collect(),
        )
    }

    fn free_iterators(iters: Vec<Iter>) -> Instr {
        Instr::gather(
            iters
                .into_iter()
                .map(|i| Instr::make_instr_iterfree(i))
                .collect(),
        )
    }

    fn wrap_inner_in_try_catch(
        label_gen: &mut label::Gen,
        (before, inner, after): (Instr, Instr, Instr),
        catch_instrs: Instr,
    ) -> Instr {
        Instr::gather(vec![
            before,
            Instr::create_try_catch(label_gen, None, false, inner, catch_instrs),
            after,
        ])
    }
}
