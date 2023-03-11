// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;

use env::emitter::Emitter;
use error::Error;
use error::Result;
use hack_macro::hack_stmt;
use hhbc::decl_vars;
use ocamlrep::rc::RcOc;
use oxidized::ast;
use oxidized::ast::Def;
use oxidized::ast::FunDef;
use oxidized::ast::FunKind;
use oxidized::ast::Fun_;
use oxidized::ast::FuncBody;
use oxidized::ast::Id;
use oxidized::ast::Pos;
use oxidized::ast::Stmt;
use oxidized::ast::Stmt_;
use oxidized::ast::TypeHint;
use oxidized::ast::UserAttribute;
use oxidized::file_info::Mode;
use oxidized::local_id;
use oxidized::namespace_env;
use relative_path::Prefix;
use relative_path::RelativePath;
use rewrite_xml::rewrite_xml;

fn debugger_eval_should_modify(tast: &[ast::Def]) -> Result<bool> {
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
        return Err(Error::unrecoverable(
            "Lowered AST did not start with a Markup statement",
        ));
    }

    if tast.len() != 2 {
        Ok(false)
    } else {
        Ok(tast[1].as_stmt().map_or(false, |x| x.1.is_expr()))
    }
}

pub fn rewrite_program<'p, 'arena, 'emitter, 'decl>(
    emitter: &'emitter mut Emitter<'arena, 'decl>,
    prog: &'p mut ast::Program,
    namespace_env: RcOc<namespace_env::Env>,
) -> Result<()> {
    let for_debugger_eval =
        emitter.for_debugger_eval && debugger_eval_should_modify(prog.as_slice())?;
    if !emitter.for_debugger_eval {
        let contains_toplevel_code = prog.iter().find_map(|d| {
            if let Some(ast::Stmt(pos, s_)) = d.as_stmt() {
                if s_.is_markup() { None } else { Some(pos) }
            } else {
                None
            }
        });
        if let Some(pos) = contains_toplevel_code {
            return Err(Error::fatal_parse(pos, "Found top-level code"));
        }
    }

    if emitter.options().compiler_flags.constant_folding {
        constant_folder::fold_program(prog, emitter)
            .map_err(|e| Error::unrecoverable(format!("{}", e)))?;
    }

    if emitter.for_debugger_eval {
        extract_debugger_main(&namespace_env, &mut prog.0).map_err(Error::unrecoverable)?;
    }

    prog.0 = flatten_ns(prog.0.drain(..));

    closure_convert::convert_toplevel_prog(emitter, &mut prog.0, namespace_env)?;

    emitter.for_debugger_eval = for_debugger_eval;

    rewrite_xml(emitter, prog)?;

    Ok(())
}

fn extract_debugger_main(
    empty_namespace: &RcOc<namespace_env::Env>,
    all_defs: &mut Vec<Def>,
) -> std::result::Result<(), String> {
    let (stmts, mut defs): (Vec<Def>, Vec<Def>) = all_defs.drain(..).partition(|x| x.is_stmt());
    let mut vars = decl_vars::vars_from_ast(&[], &stmts)?
        .into_iter()
        .collect::<Vec<_>>();
    let mut stmts = stmts
        .into_iter()
        .filter_map(|x| x.as_stmt_into())
        .collect::<Vec<_>>();
    let stmts =
        if defs.is_empty() && stmts.len() == 2 && stmts[0].1.is_markup() && stmts[1].1.is_expr() {
            let Stmt(p, s) = stmts.pop().unwrap();
            let e = s.as_expr_into().unwrap();
            let m = stmts.pop().unwrap();
            vec![m, Stmt::new(p, Stmt_::mk_return(Some(e)))]
        } else {
            stmts
        };
    let p = || Pos::NONE;
    let mut unsets: ast::Block = vars
        .iter()
        .map(|name| {
            let name = local_id::make_unscoped(name);
            hack_stmt!("if (#{lvar(clone(name))} is __uninitSentinel) { unset(#{lvar(name)}); }")
        })
        .collect();
    let sets: Vec<_> = vars
        .iter()
        .map(|name: &String| {
            let name = local_id::make_unscoped(name);
            hack_stmt!(
                pos = p(),
                r#"if (\__SystemLib\__debugger_is_uninit(#{lvar(clone(name))})) {
                       #{lvar(name)} = new __uninitSentinel();
                     }
                "#
            )
        })
        .collect();
    vars.push("$__debugger$this".into());
    vars.push("$__debugger_exn$output".into());
    let params: Vec<_> = vars
        .iter()
        .map(|var| closure_convert::make_fn_param(p(), &local_id::make_unscoped(var), false, true))
        .collect();
    let exnvar = local_id::make_unscoped("$__debugger_exn$output");
    let catch = hack_stmt!(
        pos = p(),
        r#"
            try {
              #{stmts*};
            } catch (Throwable #{lvar(exnvar)}) {
              /* no-op */
            } finally {
              #{sets*};
            }
        "#
    );
    unsets.push(catch);
    let body = unsets;
    let pos = Pos::from_line_cols_offset(
        RcOc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(""))),
        1,
        0..0,
        0,
    );
    let f = Fun_ {
        span: pos,
        annotation: (),
        readonly_this: None, // TODO(readonly): readonly_this in closure_convert
        readonly_ret: None,  // TODO(readonly): readonly_ret in closure_convert
        ret: TypeHint((), None),
        params,
        ctxs: None,        // TODO(T70095684)
        unsafe_ctxs: None, // TODO(T70095684)
        body: FuncBody { fb_ast: body },
        fun_kind: FunKind::FSync,
        user_attributes: ast::UserAttributes(vec![UserAttribute {
            name: Id(Pos::NONE, "__DebuggerMain".into()),
            params: vec![],
        }]),
        external: false,
        doc_comment: None,
    };
    let fd = FunDef {
        namespace: RcOc::clone(empty_namespace),
        file_attributes: vec![],
        mode: Mode::Mstrict,
        name: Id(Pos::NONE, "include".into()),
        fun: f,
        // TODO(T116039119): Populate value with presence of internal attribute
        internal: false,
        module: None,
        tparams: vec![],
        where_constraints: vec![],
    };
    let mut new_defs = vec![Def::mk_fun(fd)];
    new_defs.append(&mut defs);
    *all_defs = new_defs;
    Ok(())
}

fn flatten_ns(defs: impl Iterator<Item = Def> + ExactSizeIterator) -> Vec<Def> {
    fn helper(out: &mut Vec<Def>, defs: impl Iterator<Item = Def> + ExactSizeIterator) {
        out.reserve(defs.len());
        for def in defs {
            match def {
                Def::Namespace(ns) => {
                    helper(out, ns.1.into_iter());
                }
                _ => out.push(def),
            }
        }
    }

    let mut out = Vec::with_capacity(defs.len());
    helper(&mut out, defs);
    out
}
