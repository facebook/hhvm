// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::{self, emitter::Emitter};
use options::Options;

extern crate bitflags;
use bitflags::bitflags;

// PUBLIC INTERFACE (ENTRY POINTS)

/// This is the entry point from hh_single_compile & fuzzer
pub fn emit_fatal_program(options: Options, is_systemlib: bool) {
    // TODO(hrust) implement the rest
    let mut emitter = Emitter::new(options, env::GlobalState::default());
    emitter.context_mut().set_systemlib(is_systemlib);
}

/// This is the entry point from hh_single_compile & coroutine
#[allow(unused_variables)]
pub fn from_ast(options: Options, is_systemlib: bool, flags: FromAstFlags) {
    // TODO(hrust) real code in closure_convert.rs
    fn closure_convert() -> ((), env::GlobalState) {
        ((), env::GlobalState::default())
    }

    let (_ast_defs, global_state) = closure_convert();
    let mut emitter = Emitter::new(options, global_state);
    emitter.context_mut().set_systemlib(is_systemlib);

    let prog = emit_main(&mut emitter, flags);
    // TODO(hrust) implement rest, passing &mut Emitter
    match prog {
        Ok(ret) => ret,
        // TODO(hrust) emit_fatal::IncludeTimeFatalException
        Err(_) => emit_fatal_program(emitter.into_options(), is_systemlib),
    }
}

bitflags! {
    pub struct FromAstFlags: u8 {
        const IS_HH_FILE =        0b0001;
        const IS_EVALED =         0b0010;
        const FOR_DEBUGGER_EVAL = 0b0100;
    }
}

// IMPLEMENTATION DETAILS

#[allow(unused_variables)]
fn emit_main(emitter: &mut Emitter, flags: FromAstFlags) -> Result<(), ()> {
    // TODO(hrust) implement
    Err(())
}
