// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::DeclProvider;
use hhbc_by_ref_closure_convert as closure_convert;
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_instruction_sequence::{unrecoverable, Result};
use hhbc_by_ref_rewrite_xml::rewrite_xml;
use ocamlrep::rc::RcOc;
use oxidized::{ast as Tast, namespace_env};

fn debugger_eval_should_modify(tast: &[Tast::Def]) -> Result<bool> {
    /*
    The AST currently always starts with a Markup statement, so a
    length of 2 means there was 1 user def (statement, function,
    etc.); we assert that the first thing is a Markup statement, and
    we only want to modify if there was exactly one user def (both 0
    user defs and > 1 user def are valid situations where we pass the
    program through unmodififed)
    */
    if tast
        .first()
        .and_then(|x| x.as_stmt())
        .map_or(true, |x| !x.1.is_markup())
    {
        return Err(unrecoverable(
            "Lowered AST did not start with a Markup statement",
        ));
    }

    if tast.len() != 2 {
        Ok(false)
    } else {
        Ok(tast[1].as_stmt().map_or(false, |x| x.1.is_expr()))
    }
}

pub fn rewrite_program<'p, 'arena, 'emitter, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &'emitter mut Emitter<'arena, 'decl, D>,
    prog: &'p mut Tast::Program,
    namespace_env: RcOc<namespace_env::Env>,
) -> Result<()> {
    let for_debugger_eval =
        emitter.for_debugger_eval && debugger_eval_should_modify(prog.as_slice())?;
    if !emitter.for_debugger_eval {
        let contains_toplevel_code = prog.iter().find_map(|d| {
            if let Some(Tast::Stmt(pos, s_)) = d.as_stmt() {
                if s_.is_markup() { None } else { Some(pos) }
            } else {
                None
            }
        });
        if let Some(pos) = contains_toplevel_code {
            return Err(emit_fatal::raise_fatal_parse(pos, "Found top-level code"));
        }
    }

    let local_alloc = bumpalo::Bump::new();
    closure_convert::convert_toplevel_prog(&local_alloc, emitter, prog, namespace_env)?;

    emitter.for_debugger_eval = for_debugger_eval;

    rewrite_xml(alloc, emitter, prog)?;

    Ok(())
}
