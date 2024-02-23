// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use hash::HashMap;
use hash::HashSet;
use hhbc::AdataId;
use hhbc::Local;
use hhbc::TypedValue;
use log::trace;

use crate::body::Body;
use crate::instr_ptr::InstrPtr;
use crate::state::State;
use crate::value::ValueBuilder;

#[derive(Default)]
pub(crate) struct WorkQueue<'arena, 'a> {
    queue: Vec<(State<'arena, 'a>, State<'arena, 'a>)>,
    processed: HashSet<(InstrPtr, InstrPtr)>,
}

impl<'arena, 'a> WorkQueue<'arena, 'a> {
    pub(crate) fn init_from_bodies(
        &mut self,
        value_builder: &mut ValueBuilder<'arena>,
        a: &'a Body<'arena>,
        a_adata: &'a HashMap<AdataId, &'arena TypedValue>,
        b: &'a Body<'arena>,
        b_adata: &'a HashMap<AdataId, &'arena TypedValue>,
    ) {
        let mut a_state = State::new(a, "A", a_adata);
        let mut b_state = State::new(b, "B", b_adata);

        // Also need to handle entrypoints for defaults!
        for (idx, (param_a, param_b)) in a
            .hhbc_body
            .params
            .iter()
            .zip(b.hhbc_body.params.iter())
            .enumerate()
        {
            // Initialize parameter values
            let value = value_builder.alloc();

            let local = Local::from_usize(idx);
            a_state.local_set(&local, value);
            b_state.local_set(&local, value);

            match (&param_a.default_value, &param_b.default_value) {
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

    pub(crate) fn add(&mut self, a: State<'arena, 'a>, b: State<'arena, 'a>) {
        if self.processed.insert((a.ip, b.ip)) {
            trace!("Added");
            self.queue.push((a, b));
        } else {
            trace!("Skipped");
        }
    }

    pub(crate) fn pop(&mut self) -> Option<(State<'arena, 'a>, State<'arena, 'a>)> {
        self.queue.pop()
    }
}
