// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(box_patterns)]

mod emit_adata;
mod emit_attribute;
mod emit_body;
mod emit_class;
mod emit_constant;
mod emit_expression;
mod emit_fatal;
mod emit_file_attributes;
mod emit_function;
mod emit_memoize_function;
mod emit_memoize_helpers;
mod emit_memoize_method;
mod emit_method;
mod emit_module;
mod emit_native_opcode;
mod emit_param;
mod emit_property;
mod emit_statement;
mod emit_type_constant;
mod emit_typedef;
mod emit_xhp;
mod generator;
mod opt_body;
mod reified_generics_helpers;
mod try_finally_rewriter;
mod xhp_attribute;

use std::borrow::Borrow;
use std::collections::VecDeque;
use std::sync::Arc;

use decl_provider::DeclProvider;
use decl_provider::TypeDecl;
use emit_class::emit_classes_from_program;
use emit_constant::emit_constants_from_program;
use emit_file_attributes::emit_file_attributes_from_program;
use emit_function::emit_functions_from_program;
use emit_module::emit_module_use_from_program;
use emit_module::emit_modules_from_program;
use emit_typedef::emit_typedefs_from_program;
use env::emitter::Emitter;
use env::Env;
use error::Error;
use error::ErrorKind;
use error::Result;
use ffi::Maybe::*;
use hhbc::Fatal;
use hhbc::FatalOp;
use hhbc::StringId;
use hhbc::Unit;
use oxidized::ast;
use oxidized::namespace_env;
use oxidized::pos::Pos;
use oxidized_by_ref::typing_defs;
use oxidized_by_ref::typing_defs_core::Exact;

// PUBLIC INTERFACE (ENTRY POINTS)

/// This is the entry point from hh_single_compile & fuzzer
pub fn emit_fatal_unit(op: FatalOp, pos: Pos, msg: impl Into<String>) -> Result<Unit> {
    Ok(Unit {
        fatal: Just(Fatal {
            op,
            loc: pos.into(),
            message: msg.into().into_bytes().into(),
        }),
        adata: Default::default(),
        functions: Default::default(),
        classes: Default::default(),
        modules: Default::default(),
        typedefs: Default::default(),
        file_attributes: Default::default(),
        module_use: Default::default(),
        symbol_refs: Default::default(),
        constants: Default::default(),
        missing_symbols: Default::default(),
        error_symbols: Default::default(),
        valid_utf8: true,
        invalid_utf8_offset: 0,
    })
}

/// This is the entry point from hh_single_compile
pub fn emit_unit<'a, 'd>(
    emitter: &mut Emitter<'d>,
    namespace: Arc<namespace_env::Env>,
    tast: &'a ast::Program,
    invalid_utf8_offset: Option<usize>,
) -> Result<Unit> {
    let result = emit_unit_(emitter, namespace, tast, invalid_utf8_offset);
    match result {
        Err(e) => match e.into_kind() {
            ErrorKind::IncludeTimeFatalException(op, pos, msg) => emit_fatal_unit(op, pos, msg),
            ErrorKind::Unrecoverable(x) => Err(Error::unrecoverable(x)),
        },
        _ => result,
    }
}

fn record_error(
    sym: StringId,
    e: decl_provider::Error,
    missing: &mut Vec<StringId>,
    error: &mut Vec<StringId>,
) {
    match e {
        decl_provider::Error::NotFound => {
            missing.push(sym);
        }
        decl_provider::Error::Bincode(_) => {
            error.push(sym);
        }
    }
}

fn scan_types<'d, F>(p: &dyn DeclProvider<'d>, q: &mut VecDeque<(StringId, u64)>, mut efunc: F)
where
    F: FnMut(StringId, decl_provider::Error),
{
    use typing_defs::Ty_;
    let mut seen = hhbc::StringIdSet::default();
    seen.extend(q.iter().map(|(s, _)| *s));
    while let Some((name, idx)) = q.pop_front() {
        match p.type_decl(name.as_str(), idx) {
            Ok(TypeDecl::Class(class_decl)) => {
                class_decl
                    .extends
                    .iter()
                    .chain(class_decl.implements.iter())
                    .chain(class_decl.req_class.iter())
                    .chain(class_decl.req_extends.iter())
                    .chain(class_decl.req_implements.iter())
                    .chain(class_decl.uses.iter())
                    .for_each(|ty| {
                        if let Ty_::Tclass(((_, cn), Exact::Exact, _ty_args)) = ty.1 {
                            let cn = hhbc::intern(*cn);
                            if seen.insert(cn) {
                                q.push_back((cn, idx + 1))
                            }
                        }
                    });
            }
            Ok(TypeDecl::Typedef(_)) => {}
            Err(e) => efunc(name, e),
        }
    }
}

fn emit_unit_<'a, 'd>(
    emitter: &mut Emitter<'d>,
    namespace: Arc<namespace_env::Env>,
    prog: &'a ast::Program,
    invalid_utf8_offset: Option<usize>,
) -> Result<Unit> {
    let prog = prog.as_slice();
    let mut functions = emit_functions_from_program(emitter, prog)?;
    let classes = emit_classes_from_program(emitter, prog)?;
    let modules = emit_modules_from_program(emitter, prog)?;
    let typedefs = emit_typedefs_from_program(emitter, prog)?;
    let (constants, mut const_inits) = {
        let mut env = Env::default(namespace);
        emit_constants_from_program(emitter, &mut env, prog)?
    };
    functions.append(&mut const_inits);
    let file_attributes = emit_file_attributes_from_program(emitter, prog)?;
    let adata = emitter.adata_state.take_adata();
    let module_use = emit_module_use_from_program(prog);
    let symbol_refs = emitter.finish_symbol_refs();
    let fatal = Nothing;

    let mut missing_syms = Vec::new();
    let mut error_syms = Vec::new();

    if let Some(p) = &emitter.decl_provider {
        if emitter.options().hhbc.stress_shallow_decl_deps {
            for cns in &symbol_refs.constants {
                if let Err(e) = p.const_decl(cns.as_str()) {
                    record_error(cns.as_string_id(), e, &mut missing_syms, &mut error_syms);
                }
            }
            for fun in &symbol_refs.functions {
                if let Err(e) = p.func_decl(fun.as_str()) {
                    record_error(fun.as_string_id(), e, &mut missing_syms, &mut error_syms);
                }
            }
            for cls in &symbol_refs.classes {
                if let Err(e) = p.type_decl(cls.as_str(), 0) {
                    record_error(cls.as_string_id(), e, &mut missing_syms, &mut error_syms);
                }
            }
        }
        if emitter.options().hhbc.stress_folded_decl_deps {
            let mut q = VecDeque::new();
            classes.iter().for_each(|c| {
                if let Just(b) = c.base {
                    q.push_back((b.as_string_id(), 0u64));
                }
                c.uses
                    .iter()
                    .chain(c.implements.iter())
                    .for_each(|i| q.push_back((i.as_string_id(), 0u64)));
                c.requirements
                    .iter()
                    .for_each(|r| q.push_back((r.name.as_string_id(), 0u64)));
            });
            scan_types(
                p.borrow(),
                &mut q,
                |sym: StringId, e: decl_provider::Error| {
                    record_error(sym, e, &mut missing_syms, &mut error_syms);
                },
            );
        }
    }

    Ok(Unit {
        classes: classes.into(),
        modules: modules.into(),
        functions: functions.into(),
        typedefs: typedefs.into(),
        constants: constants.into(),
        adata: adata.into(),
        file_attributes: file_attributes.into(),
        module_use,
        symbol_refs,
        fatal,
        missing_symbols: missing_syms.into(),
        error_symbols: error_syms.into(),
        valid_utf8: invalid_utf8_offset.is_none(),
        invalid_utf8_offset: invalid_utf8_offset.unwrap_or(0),
    })
}

#[derive(Clone, Copy, Debug)]
enum TypeRefinementInHint {
    Allowed,
    Disallowed,
}
