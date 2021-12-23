// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use emit_class::emit_classes_from_program;
use emit_constant::emit_constants_from_program;
use emit_file_attributes::emit_file_attributes_from_program;
use emit_function::emit_functions_from_program;
use emit_record_def::emit_record_defs_from_program;
use emit_typedef::emit_typedefs_from_program;
use env::{self, emitter::Emitter, Env};
use ffi::{Maybe::*, Slice, Str};
use hhbc_by_ref_hhas_program::HhasProgram;
use hhbc_by_ref_hhas_symbol_refs::HhasSymbolRefs;
use hhbc_by_ref_hhbc_ast::FatalOp;
use instruction_sequence::{Error, Result};
use ocamlrep::rc::RcOc;
use oxidized::{ast, namespace_env, pos::Pos};

// PUBLIC INTERFACE (ENTRY POINTS)

/// This is the entry point from hh_single_compile & fuzzer
pub fn emit_fatal_program<'arena>(
    alloc: &'arena bumpalo::Bump,
    op: FatalOp,
    pos: &Pos,
    msg: impl AsRef<str> + 'arena,
) -> Result<HhasProgram<'arena>> {
    Ok(HhasProgram {
        fatal: Just((op, pos.clone().into(), Str::new_str(alloc, msg.as_ref())).into()),
        ..HhasProgram::default()
    })
}

/// This is the entry point from hh_single_compile
pub fn emit_program<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    flags: FromAstFlags,
    namespace: RcOc<namespace_env::Env>,
    tast: &'a ast::Program,
) -> Result<HhasProgram<'arena>> {
    let result = emit_program_(emitter, flags, namespace, tast);
    match result {
        Err(Error::IncludeTimeFatalException(op, pos, msg)) => {
            emit_fatal_program(emitter.alloc, op, &pos, msg)
        }
        _ => result,
    }
}

fn emit_program_<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    _flags: FromAstFlags,
    namespace: RcOc<namespace_env::Env>,
    prog: &'a ast::Program,
) -> Result<HhasProgram<'arena>> {
    let mut functions = emit_functions_from_program(emitter, prog)?;
    let classes = emit_classes_from_program(emitter.alloc, emitter, prog)?;
    let record_defs = emit_record_defs_from_program(emitter, prog)?;
    let typedefs = emit_typedefs_from_program(emitter, prog)?;
    let (constants, mut const_inits) = {
        let mut env = Env::default(emitter.alloc, namespace);
        emit_constants_from_program(emitter, &mut env, prog)?
    };
    functions.append(&mut const_inits);
    let file_attributes = emit_file_attributes_from_program(emitter, prog)?;
    let adata = emit_adata::take(emitter).adata;
    let symbol_refs =
        HhasSymbolRefs::from_symbol_refs_state(emitter.alloc, emit_symbol_refs::take(emitter));
    let fatal = Nothing;

    Ok(HhasProgram {
        record_defs: Slice::fill_iter(emitter.alloc, record_defs.into_iter()),
        classes: Slice::fill_iter(emitter.alloc, classes.into_iter()),
        functions: Slice::fill_iter(emitter.alloc, functions.into_iter()),
        typedefs: Slice::fill_iter(emitter.alloc, typedefs.into_iter()),
        constants: Slice::fill_iter(emitter.alloc, constants.into_iter()),
        adata: Slice::fill_iter(emitter.alloc, adata.into_iter()),
        file_attributes: Slice::fill_iter(emitter.alloc, file_attributes.into_iter()),
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
