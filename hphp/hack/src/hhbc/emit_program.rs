// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::{self, emitter::Emitter};
use hhas_program_rust::HhasProgram;
use instruction_sequence_rust::Error;
use options::Options;
use oxidized::ast as Tast;
use std::result::Result;

extern crate bitflags;
use bitflags::bitflags;

// PUBLIC INTERFACE (ENTRY POINTS)

/// This is the entry point from hh_single_compile & fuzzer
pub fn emit_fatal_program<'p>(
    options: Options,
    is_systemlib: bool,
) -> Result<HhasProgram<'p>, Error> {
    // TODO(hrust) implement the rest
    let mut emitter = Emitter::new(options, env::GlobalState::default());
    emitter.context_mut().set_systemlib(is_systemlib);
    Ok(HhasProgram::default())
}

/// This is the entry point from hh_single_compile & coroutine
#[allow(unused_variables)]
pub fn emit_program<'p>(
    options: Options,
    flags: FromAstFlags,
    tast: Tast::Program,
) -> Result<HhasProgram<'p>, Error> {
    let result = emit_program_(options.clone(), flags, tast);
    match result {
        Err(Error::IncludeTimeFatalException(op, msg)) => {
            emit_fatal_program(options, flags.contains(FromAstFlags::IS_SYSTEMLIB))
        }
        _ => result,
    }
}

fn emit_program_<'p>(
    options: Options,
    flags: FromAstFlags,
    _tast: Tast::Program,
) -> Result<HhasProgram<'p>, Error> {
    // TODO(hrust) real code in closure_convert.rs
    fn closure_convert() -> ((), env::GlobalState) {
        ((), env::GlobalState::default())
    }

    let (_ast_defs, global_state) = closure_convert();
    let mut emitter = Emitter::new(options, global_state);
    emitter
        .context_mut()
        .set_systemlib(flags.contains(FromAstFlags::IS_SYSTEMLIB));

    emit_main(&mut emitter, flags);

    Ok(HhasProgram::default())
}

bitflags! {
    pub struct FromAstFlags: u8 {
        const IS_HH_FILE =        0b0001;
        const IS_EVALED =         0b0010;
        const FOR_DEBUGGER_EVAL = 0b0100;
        const IS_SYSTEMLIB =      0b1000;
    }
}

// IMPLEMENTATION DETAILS

#[allow(unused_variables)]
fn emit_main(emitter: &mut Emitter, flags: FromAstFlags) -> Result<(), ()> {
    // TODO(hrust) implement
    Err(())
}
