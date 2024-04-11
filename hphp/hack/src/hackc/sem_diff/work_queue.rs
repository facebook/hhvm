// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use hash::HashSet;
use hhbc::Local;
use hhbc::ParamEntry;
use log::trace;

use crate::body::Body;
use crate::instr_ptr::InstrPtr;
use crate::state::State;
use crate::value::ValueBuilder;

#[derive(Default)]
pub(crate) struct WorkQueue<'a> {
    queue: Vec<(State<'a>, State<'a>)>,
    processed: HashSet<(InstrPtr, InstrPtr)>,
}

impl<'a> WorkQueue<'a> {
    pub(crate) fn init_from_bodies(
        &mut self,
        value_builder: &mut ValueBuilder<'a>,
        a: &'a Body<'a>,
        b: &'a Body<'a>,
    ) {
        let mut a_state = State::new(a, "A");
        let mut b_state = State::new(b, "B");

        // Also need to handle entrypoints for defaults!
        for (idx, (ParamEntry { dv: dv_a, .. }, ParamEntry { dv: dv_b, .. })) in a
            .hhbc_body
            .repr
            .params
            .iter()
            .zip(b.hhbc_body.repr.params.iter())
            .enumerate()
        {
            // Initialize parameter values
            let value = value_builder.alloc();

            let local = Local::new(idx);
            a_state.local_set(&local, value);
            b_state.local_set(&local, value);

            match (dv_a, dv_b) {
                (Maybe::Just(a), Maybe::Just(b)) => {
                    // The text should have already been compared.
                    let a_state = a_state.clone_with_jmp(&a.label);
                    let b_state = b_state.clone_with_jmp(&b.label);
                    self.add(a_state, b_state);
                }
                (Maybe::Nothing, Maybe::Nothing) => {}
                _ => {
                    // We should have already checked that the params at least
                    // both have a default or not.
                    unreachable!()
                }
            }
        }

        self.add(a_state, b_state);
    }

    pub(crate) fn add(&mut self, a: State<'a>, b: State<'a>) {
        if self.processed.insert((a.ip, b.ip)) {
            trace!("Added");
            self.queue.push((a, b));
        } else {
            trace!("Skipped");
        }
    }

    pub(crate) fn pop(&mut self) -> Option<(State<'a>, State<'a>)> {
        self.queue.pop()
    }
}
