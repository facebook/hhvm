use ffi::Maybe;
use ffi::Pair;
use hash::HashSet;
use hhbc::Local;
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
        value_builder: &mut ValueBuilder,
        a: &'a Body<'arena>,
        b: &'a Body<'arena>,
    ) {
        let mut a_state = State::new(a, "A");
        let mut b_state = State::new(b, "B");

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

            match (param_a.default_value, param_b.default_value) {
                (Maybe::Just(Pair(a_target, _)), Maybe::Just(Pair(b_target, _))) => {
                    // The text should have already been compared.
                    let a_state = a_state.clone_with_jmp(&a_target);
                    let b_state = b_state.clone_with_jmp(&b_target);
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
