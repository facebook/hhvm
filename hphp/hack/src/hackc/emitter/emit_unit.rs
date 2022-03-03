// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use emit_class::emit_classes_from_program;
use emit_constant::emit_constants_from_program;
use emit_file_attributes::emit_file_attributes_from_program;
use emit_function::emit_functions_from_program;
use emit_typedef::emit_typedefs_from_program;
use env::{self, emitter::Emitter, Env};
use ffi::{Maybe::*, Slice, Str};
use hackc_unit::HackCUnit;
use hhas_symbol_refs::HhasSymbolRefs;
use hhbc_ast::FatalOp;
use instruction_sequence::{Error, Result};
use ocamlrep::rc::RcOc;
use oxidized::{ast, namespace_env, pos::Pos};

// PUBLIC INTERFACE (ENTRY POINTS)

/// This is the entry point from hh_single_compile & fuzzer
pub fn emit_fatal_unit<'arena>(
    alloc: &'arena bumpalo::Bump,
    op: FatalOp,
    pos: &Pos,
    msg: impl AsRef<str> + 'arena,
) -> Result<HackCUnit<'arena>> {
    Ok(HackCUnit {
        fatal: Just((op, pos.clone().into(), Str::new_str(alloc, msg.as_ref())).into()),
        ..HackCUnit::default()
    })
}

/// This is the entry point from hh_single_compile
pub fn emit_unit<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    flags: FromAstFlags,
    namespace: RcOc<namespace_env::Env>,
    tast: &'a ast::Program,
) -> Result<HackCUnit<'arena>> {
    let result = emit_unit_(emitter, flags, namespace, tast);
    match result {
        Err(Error::IncludeTimeFatalException(op, pos, msg)) => {
            emit_fatal_unit(emitter.alloc, op, &pos, msg)
        }
        _ => result,
    }
}

fn emit_unit_<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    _flags: FromAstFlags,
    namespace: RcOc<namespace_env::Env>,
    prog: &'a ast::Program,
) -> Result<HackCUnit<'arena>> {
    let prog = prog.as_slice();
    let mut functions = emit_functions_from_program(emitter, prog)?;
    let classes = emit_classes_from_program(emitter.alloc, emitter, prog)?;
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

    Ok(HackCUnit {
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
