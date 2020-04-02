// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use closure_convert_rust as closure_convert;
use emit_adata_rust as emit_adata;
use emit_body_rust::{emit_body_with_default_args, make_body};
use emit_class_rust::emit_classes_from_program;
use emit_constant_rust::emit_constants_from_program;
use emit_fatal_rust::emit_fatal;
use emit_file_attributes_rust::emit_file_attributes_from_program;
use emit_function_rust::emit_functions_from_program;
use emit_record_def_rust::emit_record_defs_from_program;
use emit_symbol_refs_rust as emit_symbol_refs;
use emit_typedef_rust::emit_typedefs_from_program;
use env::{self, emitter::Emitter, Env};
use hhas_body_rust::HhasBody;
use hhas_program_rust::HhasProgram;
use hhbc_ast_rust::FatalOp;
use instruction_sequence_rust::{instr, Error, Result};
use options::Options;
use oxidized::{ast as Tast, namespace_env, pos::Pos};

extern crate bitflags;
use bitflags::bitflags;

// PUBLIC INTERFACE (ENTRY POINTS)

/// This is the entry point from hh_single_compile & fuzzer
pub fn emit_fatal_program<'p>(
    options: &Options,
    is_systemlib: bool,
    op: FatalOp,
    pos: &Pos,
    msg: impl AsRef<str>,
) -> Result<HhasProgram<'p>> {
    let mut emitter = Emitter::new(options.clone());
    emitter.context_mut().set_systemlib(is_systemlib);
    let body_instrs = emit_fatal(op, pos, msg);
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
    )?;
    Ok(HhasProgram {
        is_hh: true,
        main,
        ..HhasProgram::default()
    })
}

/// This is the entry point from hh_single_compile & coroutine
pub fn emit_program<'p>(
    emitter: &mut Emitter,
    flags: FromAstFlags,
    namespace: &namespace_env::Env,
    tast: &'p mut Tast::Program,
) -> Result<HhasProgram<'p>> {
    let result = emit_program_(emitter, flags, namespace, tast);
    match result {
        Err(Error::IncludeTimeFatalException(op, pos, msg)) => emit_fatal_program(
            emitter.options(),
            flags.contains(FromAstFlags::IS_SYSTEMLIB),
            op,
            &pos,
            msg,
        ),
        _ => result,
    }
}

fn emit_program_<'p>(
    emitter: &mut Emitter,
    flags: FromAstFlags,
    namespace: &namespace_env::Env,
    prog: &'p mut Tast::Program,
) -> Result<HhasProgram<'p>> {
    let hoist_kinds = closure_convert::convert_toplevel_prog(emitter, prog)?;
    emitter
        .context_mut()
        .set_systemlib(flags.contains(FromAstFlags::IS_SYSTEMLIB));

    let main = emit_main(emitter, flags, namespace, prog)?;
    let mut functions = emit_functions_from_program(emitter, &hoist_kinds, prog)?;
    let classes = emit_classes_from_program(emitter, &hoist_kinds, prog)?;
    let record_defs = emit_record_defs_from_program(emitter, prog)?;
    let typedefs = emit_typedefs_from_program(emitter, prog)?;
    let (constants, mut const_inits) = {
        let mut env = Env::default();
        // TODO(hrust), why clone is needed here? Try Rc,
        env.namespace = namespace.clone();
        emit_constants_from_program(emitter, &mut env, prog)?
    };
    functions.append(&mut const_inits);
    let file_attributes = emit_file_attributes_from_program(emitter, prog)?;
    let adata = emit_adata::take(emitter).adata;
    let symbol_refs = emit_symbol_refs::take(emitter).symbol_refs;

    Ok(HhasProgram {
        main,
        record_defs,
        classes,
        functions,
        typedefs,
        constants,
        adata,
        is_hh: flags.contains(FromAstFlags::IS_HH_FILE),
        file_attributes,
        symbol_refs,
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
        instr::null()
    } else {
        instr::int(1)
    };
    emit_body_with_default_args(emitter, namespace, prog, return_value)
}
