// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub mod typing_phase;

use oxidized::{ast, pos::Pos};
use typing_defs_rust::{tast, FuncBodyAnn, SavedEnv, Ty, Ty_};
use typing_env_rust::Env;

fn dispatch_call<'a>(ast::Expr(_pos, e): &ast::Expr, env: &mut Env<'a>) -> Ty<'a> {
    let bld = env.builder();
    match e {
        ast::Expr_::Id(x) => {
            let (_pos, sid) = (&x.0, &x.1);
            match env.provider().get_fun(sid) {
                None => unimplemented!(),
                Some(f) => {
                    let fty = &f.type_;
                    let ety_env = bld.env_with_self();
                    let fty = typing_phase::localize(&ety_env, env, fty);
                    match fty.get_node() {
                        Ty_::Tfun(x) => x.return_,
                        x => unimplemented!("{:#?}", x),
                    }
                }
            }
        }
        x => unimplemented!("{:#?}", x),
    }
}

fn expr<'a>(ast::Expr(pos, e): &ast::Expr, env: &mut Env<'a>) -> tast::Expr<'a> {
    let (ty, e) = match e {
        ast::Expr_::Call(x) => {
            if let (ast::CallType::Cnormal, e, [], [], None) =
                (x.0, &x.1, &x.2[..], &&x.3[..], &x.4)
            {
                let ty = dispatch_call(e, env);
                let e = tast::Expr(
                    (
                        Pos::make_none(),
                        env.builder().null(env.builder().mk_rnone()),
                    ),
                    tast::Expr_::mk_null(),
                );
                (
                    ty,
                    tast::Expr_::mk_call(tast::CallType::Cnormal, e, vec![], vec![], None),
                )
            } else {
                unimplemented!("{:#?}", x);
            }
        }
        x => unimplemented!("{:#?}", x),
    };
    ast::Expr((pos.clone(), ty), e)
}

fn markup<'a>(s: &ast::Pstring, e: &Option<ast::Expr>) -> tast::Stmt_<'a> {
    match e {
        None => tast::Stmt_::mk_markup(s.clone(), None),
        x => unimplemented!("{:#?}", x),
    }
}

fn stmt_<'a>(s: &ast::Stmt_, env: &mut Env<'a>) -> tast::Stmt_<'a> {
    match s {
        ast::Stmt_::Noop => tast::Stmt_::Noop,
        ast::Stmt_::Markup(x) => markup(&x.0, &x.1),
        ast::Stmt_::Expr(x) => tast::Stmt_::mk_expr(expr(x, env)),
        x => unimplemented!("{:#?}", x),
    }
}

fn stmt<'a>(ast::Stmt(pos, s): &ast::Stmt, env: &mut Env<'a>) -> tast::Stmt<'a> {
    tast::Stmt(pos.clone(), stmt_(s, env))
}

fn fun<'a>(f: &ast::Fun_, env: &mut Env<'a>) -> tast::Fun_<'a> {
    let ast = f.body.ast.iter().map(|x| stmt(x, env)).collect();

    // We put empty vec below for all of those, since real conversion is unimplemented
    assert!(f.tparams.is_empty());
    assert!(f.params.is_empty());
    assert!(f.user_attributes.is_empty());
    assert!(f.file_attributes.is_empty());

    let ret = tast::TypeHint(env.builder().null(env.builder().mk_rnone()), None);

    tast::Fun_ {
        span: f.span.clone(),
        annotation: SavedEnv,
        mode: f.mode,
        ret,
        name: f.name.clone(),
        tparams: vec![],
        where_constraints: f.where_constraints.clone(),
        body: tast::FuncBody {
            ast,
            annotation: FuncBodyAnn,
        },
        fun_kind: f.fun_kind,
        variadic: tast::FunVariadicity::FVnonVariadic,
        params: vec![],
        user_attributes: vec![],
        file_attributes: vec![],
        external: f.external,
        namespace: f.namespace.clone(),
        doc_comment: f.doc_comment.clone(),
        static_: f.static_,
    }
}

fn def<'a>(def: &ast::Def, env: &mut Env<'a>) -> Option<tast::Def<'a>> {
    match def {
        ast::Def::Fun(x) => Some(tast::Def::mk_fun(fun(x, env))),
        ast::Def::Stmt(x) => Some(tast::Def::mk_stmt(stmt(x, env))),
        _ => unimplemented!(),
    }
}

pub fn program<'a>(ast: &ast::Program, env: &mut Env<'a>) -> tast::Program<'a> {
    ast.iter().filter_map(|x| def(x, env)).collect()
}
