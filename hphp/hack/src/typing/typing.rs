// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::ast;
use typing_defs_rust::{tast, Ty};
use typing_env_rust::Genv;

fn dispatch_call<'a>(ast::Expr(_pos, e): &ast::Expr, env: &'a Genv) -> Ty<'a> {
    use oxidized::decl_defs::Ty_;
    match e {
        ast::Expr_::Id(x) => {
            let (_pos, sid) = (&x.0, &x.1);
            match env.provider.get_fun(sid) {
                None => unimplemented!(),
                Some(f) => match f.type_.1.as_ref() {
                    Ty_::Tfun(x) => Ty::from_oxidized(&x.ret.type_, env.builder),
                    x => unimplemented!("{:#?}", x),
                },
            }
        }
        x => unimplemented!("{:#?}", x),
    }
}

fn expr<'a>(ast::Expr(_pos, e): &ast::Expr, env: &'a Genv) -> tast::Expr<'a> {
    let (ty, e) = match e {
        ast::Expr_::Call(x) => {
            if let (ast::CallType::Cnormal, e, [], [], None) =
                (x.0, &x.1, &x.2[..], &&x.3[..], &x.4)
            {
                let ty = dispatch_call(e, env);
                let e = tast::Expr(
                    env.builder.null(env.builder.mk_rnone()),
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
    ast::Expr(ty, e)
}

fn markup<'a>(s: &ast::Pstring, e: &Option<ast::Expr>) -> tast::Stmt_<'a> {
    match e {
        None => tast::Stmt_::mk_markup(s.clone(), None),
        x => unimplemented!("{:#?}", x),
    }
}

fn stmt_<'a>(s: &ast::Stmt_, env: &'a Genv) -> tast::Stmt_<'a> {
    match s {
        ast::Stmt_::Noop => tast::Stmt_::Noop,
        ast::Stmt_::Markup(x) => markup(&x.0, &x.1),
        ast::Stmt_::Expr(x) => tast::Stmt_::mk_expr(expr(x, env)),
        x => unimplemented!("{:#?}", x),
    }
}

fn stmt<'a>(ast::Stmt(pos, s): &ast::Stmt, env: &'a Genv) -> tast::Stmt<'a> {
    tast::Stmt(pos.clone(), stmt_(s, env))
}

fn fun<'a>(f: &ast::Fun_, env: &'a Genv) -> tast::Fun_<'a> {
    let ast = f.body.ast.iter().map(|x| stmt(x, env)).collect();

    // We put empty vec below for all of those, since real conversion is unimplemented
    assert!(f.tparams.is_empty());
    assert!(f.params.is_empty());
    assert!(f.user_attributes.is_empty());
    assert!(f.file_attributes.is_empty());

    tast::Fun_ {
        span: f.span.clone(),
        annotation: f.annotation,
        mode: f.mode,
        ret: f.ret.clone(),
        name: f.name.clone(),
        tparams: vec![],
        where_constraints: f.where_constraints.clone(),
        body: tast::FuncBody {
            ast,
            annotation: f.body.annotation,
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

fn def<'a>(def: &ast::Def, env: &'a Genv) -> Option<tast::Def<'a>> {
    match def {
        ast::Def::Fun(x) => Some(tast::Def::mk_fun(fun(x, env))),
        ast::Def::Stmt(x) => Some(tast::Def::mk_stmt(stmt(x, env))),
        _ => unimplemented!(),
    }
}

pub fn program<'a>(ast: &ast::Program, env: &'a Genv) -> tast::Program<'a> {
    ast.iter().filter_map(|x| def(x, env)).collect()
}
