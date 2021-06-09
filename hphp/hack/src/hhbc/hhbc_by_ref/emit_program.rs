// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use decl_provider::DeclProvider;
use hhbc_by_ref_emit_adata as emit_adata;
use hhbc_by_ref_emit_class::emit_classes_from_program;
use hhbc_by_ref_emit_constant::emit_constants_from_program;
use hhbc_by_ref_emit_file_attributes::emit_file_attributes_from_program;
use hhbc_by_ref_emit_function::emit_functions_from_program;
use hhbc_by_ref_emit_record_def::emit_record_defs_from_program;
use hhbc_by_ref_emit_symbol_refs as emit_symbol_refs;
use hhbc_by_ref_emit_typedef::emit_typedefs_from_program;
use hhbc_by_ref_env::{self, emitter::Emitter, Env};
use hhbc_by_ref_hhas_program::HhasProgram;
use hhbc_by_ref_hhbc_ast::FatalOp;
use hhbc_by_ref_instruction_sequence::{Error, Result};
use ocamlrep::rc::RcOc;
use oxidized::{ast as Tast, namespace_env, pos::Pos};

// PUBLIC INTERFACE (ENTRY POINTS)

/// This is the entry point from hh_single_compile & fuzzer
pub fn emit_fatal_program<'a, 'arena>(
    op: FatalOp,
    pos: &Pos,
    msg: impl AsRef<str>,
) -> Result<HhasProgram<'a, 'arena>> {
    Ok(HhasProgram {
        fatal: Some((op, pos.clone(), msg.as_ref().into())),
        ..HhasProgram::default()
    })
}

/// This is the entry point from hh_single_compile
pub fn emit_program<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl, D>,
    flags: FromAstFlags,
    namespace: RcOc<namespace_env::Env>,
    tast: &'a Tast::Program,
) -> Result<HhasProgram<'a, 'arena>> {
    let result = emit_program_(alloc, emitter, flags, namespace, tast);
    match result {
        Err(Error::IncludeTimeFatalException(op, pos, msg)) => emit_fatal_program(op, &pos, msg),
        _ => result,
    }
}

fn emit_program_<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl, D>,
    _flags: FromAstFlags,
    namespace: RcOc<namespace_env::Env>,
    prog: &'a Tast::Program,
) -> Result<HhasProgram<'a, 'arena>> {
    let mut functions = emit_functions_from_program(alloc, emitter, prog)?;
    let classes = emit_classes_from_program(alloc, emitter, prog)?;
    let record_defs = emit_record_defs_from_program(alloc, emitter, prog)?;
    let typedefs = emit_typedefs_from_program(alloc, emitter, prog)?;
    let (constants, mut const_inits) = {
        let mut env = Env::default(alloc, namespace);
        emit_constants_from_program(emitter, &mut env, prog)?
    };
    functions.append(&mut const_inits);
    let file_attributes = emit_file_attributes_from_program(alloc, emitter, prog)?;
    let adata = emit_adata::take(alloc, emitter).adata;
    let symbol_refs = emit_symbol_refs::take(alloc, emitter).symbol_refs;
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
