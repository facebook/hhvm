// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use closure_convert_rust as closure_convert;
use emit_fatal_rust as emit_fatal;
use env::emitter::Emitter;
use instruction_sequence::{unrecoverable, Result};
use oxidized::ast as Tast;
use rewrite_xml::rewrite_xml;

fn debugger_eval_should_modify(tast: &Tast::Program) -> Result<bool> {
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
        .map(|x| !x.1.is_markup())
        .unwrap_or(true)
    {
        return Err(unrecoverable(
            "Lowered AST did not start with a Markup statement",
        ));
    }

    if tast.len() != 2 {
        Ok(false)
    } else {
        Ok(tast[1].as_stmt().map(|x| x.1.is_expr()).unwrap_or(false))
    }
}

pub fn rewrite_program<'p>(emitter: &mut Emitter, prog: &'p mut Tast::Program) -> Result<()> {
    let for_debugger_eval = emitter.for_debugger_eval && debugger_eval_should_modify(prog)?;
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

    closure_convert::convert_toplevel_prog(emitter, prog)?;

    emitter.for_debugger_eval = for_debugger_eval;

    rewrite_xml(emitter, prog)?;

    Ok(())
}
