use crate::lowerer::Env;
use bstr::BString;
use naming_special_names_rust::{collections, typehints};
use ocamlrep::rc::RcOc;
use oxidized::{
    aast, ast,
    ast::{Expr, Expr_, Stmt, Stmt_},
    ast_defs::*,
    file_info,
    pos::Pos,
};

/// Convert an expression tree body to `(dynamic $v) ==> { return $v->...; }`.
pub fn desugar(e: &Expr, env: &Env) -> Expr {
    let visitor_expr = rewrite_expr(&e);

    let body = ast::FuncBody {
        ast: vec![wrap_return(visitor_expr)],
        annotation: (),
    };

    let param = ast::FunParam {
        annotation: Pos::make_none(),
        type_hint: ast::TypeHint(
            (),
            Some(aast::Hint(
                Pos::make_none(),
                // We can't use Hdynamic here, because naming expects
                // to convert an Happly to an Hdynamic later.
                Box::new(aast::Hint_::Happly(make_id(typehints::DYNAMIC), vec![])),
            )),
        ),
        is_variadic: false,
        pos: Pos::make_none(),
        name: "$v".into(),
        expr: None,
        callconv: None,
        user_attributes: vec![],
        visibility: None,
    };
    let visitor_fun = ast::Fun_ {
        span: e.0.clone(),
        annotation: (),
        mode: file_info::Mode::Mstrict,
        ret: ast::TypeHint((), None),
        name: make_id(";anonymous"),
        tparams: vec![],
        where_constraints: vec![],
        variadic: aast::FunVariadicity::FVnonVariadic,
        params: vec![param],
        body,
        fun_kind: ast::FunKind::FSync,
        cap: ast::TypeHint((), None),        // TODO(T70095684)
        unsafe_cap: ast::TypeHint((), None), // TODO(T70095684)
        user_attributes: vec![],
        file_attributes: vec![],
        external: false,
        doc_comment: None,
        namespace: RcOc::clone(&env.empty_ns_env),
        static_: false,
    };
    let visitor_lambda = Expr_::mk_lfun(visitor_fun, vec![]);
    let pos = Pos::make_none();
    Expr::new(pos, visitor_lambda)
}

/// Convert `foo` to `return foo;`.
fn wrap_return(e: Expr) -> Stmt {
    Stmt::new(Pos::make_none(), Stmt_::Return(Box::new(Some(e))))
}

/// Convert expression tree expressions to method calls.
fn rewrite_expr(e: &Expr) -> Expr {
    use aast::Expr_::*;
    match &e.1 {
        // Convert `1` to `$v->intLiteral(1)`.
        Int(_) => meth_call("intLiteral", vec![e.clone()]),
        // Convert `"foo"` to `$v->stringLiteral("foo")`.
        String(_) => meth_call("stringLiteral", vec![e.clone()]),
        // Convert `$x` to `$v->localVar("$x")` (note the quoting).
        Lvar(lid) => meth_call("localVar", vec![string_literal(&((lid.1).1))]),
        Binop(bop) => match &**bop {
            // Convert `... + ...` to `$v->plus($v->..., $v->...)`.
            (Bop::Plus, lhs, rhs) => {
                meth_call("plus", vec![rewrite_expr(&lhs), rewrite_expr(&rhs)])
            }
            _ => meth_call(
                "unsupportedSyntax",
                vec![string_literal("bad binary operator")],
            ),
        },
        // Convert `foo(...)` to `$v->call('foo', vec[...])`.
        Call(call) => match &**call {
            (recv, _, args, _) => match &recv.1 {
                Id(sid) => {
                    let fn_name = string_literal(&*sid.1);
                    let desugared_args = vec![
                        fn_name,
                        vec_literal(args.iter().map(rewrite_expr).collect()),
                    ];
                    meth_call("call", desugared_args)
                }
                _ => meth_call(
                    "unsupportedSyntax",
                    vec![string_literal("invalid function call")],
                ),
            },
        },
        // Convert `($x) ==> { ... }` to `$v->lambdaLiteral(vec["$x"], vec[...])`.
        Lfun(lf) => {
            let fun_ = &lf.0;
            let param_names = fun_
                .params
                .iter()
                .map(|p| string_literal(&p.name))
                .collect();
            let body_stmts = rewrite_stmts(&fun_.body.ast);

            meth_call(
                "lambdaLiteral",
                vec![vec_literal(param_names), vec_literal(body_stmts)],
            )
        }
        // Convert anything else to $v->unsupportedSyntax().
        // Type checking should prevent us hitting these cases.
        _ => meth_call(
            "unsupportedSyntax",
            vec![string_literal(&format!("{:#?}", &e.1))],
        ),
    }
}

/// Convert expression tree statements to method calls.
fn rewrite_stmts(stmts: &[Stmt]) -> Vec<Expr> {
    stmts.iter().filter_map(rewrite_stmt).collect()
}

fn rewrite_stmt(s: &Stmt) -> Option<Expr> {
    use aast::Stmt_::*;

    match &s.1 {
        Expr(e) => Some(rewrite_expr(&e)),
        Return(e) => match &**e {
            // Convert `return ...;` to `$v->returnStatement($v->...)`.
            Some(e) => Some(meth_call("returnStatement", vec![rewrite_expr(&e)])),
            // Convert `return;` to `$v->returnStatement(null)`.
            None => Some(meth_call("returnStatement", vec![null_literal()])),
        },
        // Convert `if (...) { ... } else { ...}` to `$v-ifStatement($v->..., vec[...], vec[...])`.
        If(if_stmt) => match &**if_stmt {
            (e, then_block, else_block) => {
                let then_stmts = rewrite_stmts(then_block);
                let else_stmts = rewrite_stmts(else_block);

                Some(meth_call(
                    "ifStatement",
                    vec![
                        rewrite_expr(&e),
                        vec_literal(then_stmts),
                        vec_literal(else_stmts),
                    ],
                ))
            }
        },
        Noop => None,
        // Convert anything else to $v->unsupportedSyntax().
        // Type checking should prevent us hitting these cases.
        _ => Some(meth_call(
            "unsupportedSyntax",
            vec![string_literal(&format!("{:#?}", &s.1))],
        )),
    }
}

fn null_literal() -> Expr {
    Expr::new(Pos::make_none(), Expr_::Null)
}

fn string_literal(s: &str) -> Expr {
    Expr::new(Pos::make_none(), Expr_::String(BString::from(s)))
}

fn vec_literal(items: Vec<Expr>) -> Expr {
    let fields: Vec<_> = items.into_iter().map(|e| ast::Afield::AFvalue(e)).collect();
    Expr::new(
        Pos::make_none(),
        Expr_::Collection(Box::new((make_id(collections::VEC), None, fields))),
    )
}

fn make_id(name: &str) -> ast::Id {
    ast::Id(Pos::make_none(), name.into())
}

/// Build `$v->meth_name(args)`.
fn meth_call(meth_name: &str, args: Vec<Expr>) -> Expr {
    let receiver = Expr::mk_lvar(&Pos::make_none(), "$v");
    let meth = Expr::new(Pos::make_none(), Expr_::Id(Box::new(make_id(meth_name))));

    let c = Expr_::Call(Box::new((
        Expr::new(
            Pos::make_none(),
            Expr_::ObjGet(Box::new((receiver, meth, OgNullFlavor::OGNullthrows))),
        ),
        vec![],
        args,
        None,
    )));
    Expr::new(Pos::make_none(), c)
}
