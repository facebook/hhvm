// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use instruction_sequence_rust::InstrSeq;

struct State {
    // TODO(hrust)
}

impl State {
    fn init() -> Box<dyn std::any::Any> {
        Box::new(State {})
    }
}
env::lazy_emit_state!(adata_state, State, State::init);

pub fn rewrite_typed_value(_instrseq: &mut InstrSeq) {
    //TODO(hrust) implement
}

#[cfg(test)]
mod tests {
    use super::*;

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    fn ref_state_from_emiter(e: &Emitter) {
        let _: &State = e.emit_state();
    }

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    fn mut_state_from_emiter(e: &mut Emitter) {
        let _: &mut State = e.emit_state_mut();
    }
}
