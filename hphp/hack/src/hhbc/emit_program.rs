// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_adata_rust as emit_adata;
use emit_class_rust::emit_classes_from_program;
use emit_constant_rust::emit_constants_from_program;
use emit_file_attributes_rust::emit_file_attributes_from_program;
use emit_function_rust::emit_functions_from_program;
use emit_record_def_rust::emit_record_defs_from_program;
use emit_symbol_refs_rust as emit_symbol_refs;
use emit_typedef_rust::emit_typedefs_from_program;
use env::{self, emitter::Emitter, Env};
use hhas_program::HhasProgram;
use hhbc_ast_rust::FatalOp;
use instruction_sequence::{Error, Result};
use ocamlrep::rc::RcOc;
use oxidized::{ast as Tast, namespace_env, pos::Pos};

extern crate bitflags;
use bitflags::bitflags;

// PUBLIC INTERFACE (ENTRY POINTS)

/// This is the entry point from hh_single_compile & fuzzer
pub fn emit_fatal_program<'p>(
    op: FatalOp,
    pos: &Pos,
    msg: impl AsRef<str>,
) -> Result<HhasProgram<'p>> {
    Ok(HhasProgram {
        fatal: Some((op, pos.clone(), msg.as_ref().to_string())),
        ..HhasProgram::default()
    })
}

/// This is the entry point from hh_single_compile
pub fn emit_program<'p>(
    emitter: &mut Emitter,
    flags: FromAstFlags,
    namespace: RcOc<namespace_env::Env>,
    tast: &'p Tast::Program,
) -> Result<HhasProgram<'p>> {
    let result = emit_program_(emitter, flags, namespace, tast);
    match result {
        Err(Error::IncludeTimeFatalException(op, pos, msg)) => emit_fatal_program(op, &pos, msg),
        _ => result,
    }
}

fn emit_program_<'p>(
    emitter: &mut Emitter,
    _flags: FromAstFlags,
    namespace: RcOc<namespace_env::Env>,
    prog: &'p Tast::Program,
) -> Result<HhasProgram<'p>> {
    let mut functions = emit_functions_from_program(emitter, prog)?;
    let classes = emit_classes_from_program(emitter, prog)?;
    let record_defs = emit_record_defs_from_program(emitter, prog)?;
    let typedefs = emit_typedefs_from_program(emitter, prog)?;
    let (constants, mut const_inits) = {
        let mut env = Env::default(namespace);
        emit_constants_from_program(emitter, &mut env, prog)?
    };
    functions.append(&mut const_inits);
    let file_attributes = emit_file_attributes_from_program(emitter, prog)?;
    let adata = emit_adata::take(emitter).adata;
    let symbol_refs = emit_symbol_refs::take(emitter).symbol_refs;
    let fatal = None;

    Ok(HhasProgram {
        record_defs,
        classes,
        functions,
        typedefs,
        constants,
        adata,
        file_attributes,
        symbol_refs,
        fatal,
    })
}

bitflags! {
    pub struct FromAstFlags: u8 {
        const IS_EVALED =         0b0001;
        const FOR_DEBUGGER_EVAL = 0b0010;
        const IS_SYSTEMLIB =      0b0100;
    }
}
