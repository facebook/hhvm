// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use closure_convert_rust::HoistKind;
use emit_body_rust::{emit_body_with_default_args, make_body};
use emit_class_rust::emit_classes_from_program;
use emit_fatal_rust::emit_fatal;
use emit_file_attributes_rust::emit_file_attributes_from_program;
use emit_function_rust::emit_functions_from_program;
use emit_record_def_rust::emit_record_defs_from_program;
use env::{self, emitter::Emitter};
use hhas_body_rust::HhasBody;
use hhas_program_rust::HhasProgram;
use hhbc_ast_rust::FatalOp;
use instruction_sequence_rust::{Error, InstrSeq, Result};
use options::Options;
use oxidized::{ast as Tast, namespace_env, pos::Pos}; // use std::result::Result;

extern crate bitflags;
use bitflags::bitflags;

// PUBLIC INTERFACE (ENTRY POINTS)

/// This is the entry point from hh_single_compile & fuzzer
pub fn emit_fatal_program<'p>(
    options: Options,
    is_systemlib: bool,
    op: FatalOp,
    msg: impl AsRef<str>,
) -> Result<HhasProgram<'p>> {
    let mut emitter = Emitter::new(options);
    emitter.context_mut().set_systemlib(is_systemlib);
    let body_instrs = emit_fatal(&emitter, op, &Pos::make_none(), msg);
    let main = make_body(
        &mut emitter,
        body_instrs,
        vec![],
        false,
        false,
        vec![],
        vec![],
        None,
        None,
        None,
    );
    Ok(HhasProgram {
        is_hh: true,
        main,
        ..HhasProgram::default()
    })
}

/// This is the entry point from hh_single_compile & coroutine
pub fn emit_program<'p>(
    options: Options,
    flags: FromAstFlags,
    namespace: &namespace_env::Env,
    tast: &'p Tast::Program,
) -> Result<HhasProgram<'p>> {
    let result = emit_program_(options.clone(), flags, namespace, tast);
    match result {
        Err(Error::IncludeTimeFatalException(op, msg)) => {
            emit_fatal_program(options, flags.contains(FromAstFlags::IS_SYSTEMLIB), op, msg)
        }
        _ => result,
    }
}

fn emit_program_<'p>(
    options: Options,
    flags: FromAstFlags,
    namespace: &namespace_env::Env,
    prog: &'p Tast::Program,
) -> Result<HhasProgram<'p>> {
    // TODO(hrust) real code in closure_convert.rs
    fn closure_convert(
        prog: &Tast::Program,
    ) -> (Vec<(HoistKind, &Tast::Def)>, global_state::GlobalState) {
        (
            prog.iter()
                .map(|d| (HoistKind::TopLevel, d))
                .collect::<Vec<_>>(),
            global_state::GlobalState::default(),
        )
    }

    let (ast_defs, _global_state) = closure_convert(prog);
    let mut emitter = Emitter::new(options);
    emitter
        .context_mut()
        .set_systemlib(flags.contains(FromAstFlags::IS_SYSTEMLIB));

    let main = emit_main(&mut emitter, flags, namespace, prog)?;
    let functions = emit_functions_from_program(&mut emitter, ast_defs)?;
    let record_defs = emit_record_defs_from_program(&mut emitter, prog)?;
    let classes = emit_classes_from_program(&mut emitter, prog)?;
    let file_attributes = emit_file_attributes_from_program(&mut emitter, prog)?;

    Ok(HhasProgram {
        main,
        record_defs,
        classes,
        functions,
        is_hh: flags.contains(FromAstFlags::IS_HH_FILE),
        file_attributes,
        ..HhasProgram::default()
    })
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
fn emit_main<'a>(
    emitter: &mut Emitter,
    flags: FromAstFlags,
    namespace: &namespace_env::Env,
    prog: &'a Tast::Program,
) -> Result<HhasBody<'a>> {
    let return_value = if flags.contains(FromAstFlags::IS_EVALED) {
        InstrSeq::make_null()
    } else {
        InstrSeq::make_int(1)
    };
    emit_body_with_default_args(emitter, namespace, prog, return_value)
}
