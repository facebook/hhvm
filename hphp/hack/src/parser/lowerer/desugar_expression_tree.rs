use crate::lowerer::Env;
use bstr::BString;
use ocamlrep::rc::RcOc;
use oxidized::{
    aast,
    aast_visitor::{AstParams, NodeMut, VisitorMut},
    ast,
    ast::{ClassId, ClassId_, Expr, Expr_, Stmt, Stmt_},
    ast_defs::*,
    file_info,
    pos::Pos,
};

/// Convert an expression tree to
/// ```
/// # Outer thunk
/// (() ==> {
///   # Spliced assignments
///   return new ExprTree(
///     # Metadata
///     # AST as smart constructor calls function
///     function (VisitorType $v) { $v->... },
///   );
/// )();
/// ```
pub fn desugar(hint: &aast::Hint, e: &Expr, env: &Env) -> Expr {
    let (e, extracted_splices) = extract_and_replace_splices(e);
    let temp_pos = e.0.clone();

    // Create assignments of extracted splices
    // `$__1 = spliced_expr;`
    let mut thunk_body: Vec<Stmt> = extracted_splices
        .into_iter()
        .enumerate()
        .map(|(i, expr)| {
            Stmt::new(
                expr.0.clone(),
                Stmt_::Expr(Box::new(Expr::new(
                    expr.0.clone(),
                    Expr_::Binop(Box::new((Bop::Eq(None), temp_lvar(&expr.0, i), expr))),
                ))),
            )
        })
        .collect();

    // Make anonymous function of smart constructor calls
    let visitor_expr = wrap_return(rewrite_expr(&e), &temp_pos.clone());
    let visitor_body = ast::FuncBody {
        ast: vec![visitor_expr],
        annotation: (),
    };
    let param = ast::FunParam {
        annotation: hint.0.clone(),
        type_hint: ast::TypeHint((), Some(hint.clone())),
        is_variadic: false,
        pos: hint.0.clone(),
        name: "$v".into(),
        expr: None,
        callconv: None,
        user_attributes: vec![],
        visibility: None,
    };
    let visitor_fun_ = wrap_fun_(visitor_body, vec![param], temp_pos.clone(), env);
    let visitor_lambda = Expr::new(temp_pos.clone(), Expr_::mk_lfun(visitor_fun_, vec![]));

    // Make anonymous function for typing purposes
    let typing_fun = if env.codegen {
        // throw new Exception()
        Stmt::new(
            temp_pos.clone(),
            Stmt_::Throw(Box::new(Expr::new(
                temp_pos.clone(),
                Expr_::New(Box::new((
                    ClassId(
                        temp_pos.clone(),
                        ClassId_::CIexpr(Expr::new(
                            temp_pos.clone(),
                            Expr_::Id(Box::new(Id(temp_pos.clone(), "Exception".to_string()))),
                        )),
                    ),
                    vec![],
                    vec![],
                    None,
                    temp_pos.clone(),
                ))),
            ))),
        )
    } else {
        // The original expression to be inferred
        wrap_return(e, &temp_pos.clone())
    };
    let typing_fun_body = ast::FuncBody {
        ast: vec![typing_fun],
        annotation: (),
    };
    let typing_fun_ = wrap_fun_(typing_fun_body, vec![], temp_pos.clone(), env);
    let typing_lambda = Expr::new(temp_pos.clone(), Expr_::mk_lfun(typing_fun_, vec![]));

    // Make `return new ExprTree(...et_construct_args)`
    let et_construct_args = vec![visitor_lambda, typing_lambda];
    let new_et_expr = Expr::new(
        temp_pos.clone(),
        Expr_::New(Box::new((
            ClassId(
                temp_pos.clone(),
                ClassId_::CIexpr(Expr::new(
                    temp_pos.clone(),
                    Expr_::Id(Box::new(Id(temp_pos.clone(), "ExprTree".to_string()))),
                )),
            ),
            vec![],
            et_construct_args,
            None,
            temp_pos.clone(),
        ))),
    );
    let return_stmt = wrap_return(new_et_expr, &temp_pos.clone());

    // Add to the body of the thunk after the splice assignments
    thunk_body.push(return_stmt);

    // Create the thunk
    let thunk_func_body = ast::FuncBody {
        ast: thunk_body,
        annotation: (),
    };
    let thunk_fun_ = wrap_fun_(thunk_func_body, vec![], temp_pos.clone(), env);
    let thunk = Expr::new(temp_pos.clone(), Expr_::mk_lfun(thunk_fun_, vec![]));

    // Call the thunk
    Expr::new(
        temp_pos.clone(),
        Expr_::Call(Box::new((thunk, vec![], vec![], None))),
    )
}

/// Convert `foo` to `return foo;`.
fn wrap_return(e: Expr, pos: &Pos) -> Stmt {
    Stmt::new(pos.clone(), Stmt_::Return(Box::new(Some(e))))
}

/// Wrap a FuncBody into an anonymous Fun_
fn wrap_fun_(body: ast::FuncBody, params: Vec<ast::FunParam>, pos: Pos, env: &Env) -> ast::Fun_ {
    ast::Fun_ {
        span: pos,
        annotation: (),
        mode: file_info::Mode::Mstrict,
        ret: ast::TypeHint((), None),
        name: make_id(";anonymous"),
        tparams: vec![],
        where_constraints: vec![],
        variadic: aast::FunVariadicity::FVnonVariadic,
        params,
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
    }
}

/// Convert expression tree expressions to method calls.
fn rewrite_expr(e: &Expr) -> Expr {
    use aast::Expr_::*;
    match &e.1 {
        // Convert `1` to `$v->intLiteral(1)`.
        Int(_) => meth_call("intLiteral", vec![e.clone()], &e.0),
        // Convert `"foo"` to `$v->stringLiteral("foo")`.
        String(_) => meth_call("stringLiteral", vec![e.clone()], &e.0),
        // Convert `true` to `$v->boolLiteral(true)`.
        True => meth_call("boolLiteral", vec![e.clone()], &e.0),
        False => meth_call("boolLiteral", vec![e.clone()], &e.0),
        // Convert `null` to `$v->nullLiteral()`.
        Null => meth_call("nullLiteral", vec![], &e.0),
        // Convert `$x` to `$v->localVar("$x")` (note the quoting).
        Lvar(lid) => meth_call("localVar", vec![string_literal(&((lid.1).1))], &e.0),
        Binop(bop) => match &**bop {
            // Convert `... + ...` to `$v->plus($v->..., $v->...)`.
            (Bop::Plus, lhs, rhs) => {
                meth_call("plus", vec![rewrite_expr(&lhs), rewrite_expr(&rhs)], &e.0)
            }
            // Convert `... = ...` to `$v->assign($v->..., $v->...)`.
            (Bop::Eq(None), lhs, rhs) => {
                meth_call("assign", vec![rewrite_expr(&lhs), rewrite_expr(&rhs)], &e.0)
            }
            // Convert `... && ...` to `$v->ampamp($v->..., $v->...)`.
            (Bop::Ampamp, lhs, rhs) => {
                meth_call("ampamp", vec![rewrite_expr(&lhs), rewrite_expr(&rhs)], &e.0)
            }
            // Convert `... || ...` to `$v->barbar($v->..., $v->...)`.
            (Bop::Barbar, lhs, rhs) => {
                meth_call("barbar", vec![rewrite_expr(&lhs), rewrite_expr(&rhs)], &e.0)
            }
            _ => meth_call(
                "unsupportedSyntax",
                vec![string_literal("bad binary operator")],
                &e.0,
            ),
        },
        Unop(uop) => match &**uop {
            // Convert `!...` to `$v->exclamationMark($v->...)`.
            (Uop::Unot, operand) => {
                meth_call("exclamationMark", vec![rewrite_expr(&operand)], &e.0)
            }
            _ => meth_call(
                "unsupportedSyntax",
                vec![string_literal("bad unary operator")],
                &e.0,
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
                    meth_call("call", desugared_args, &e.0)
                }
                _ => meth_call(
                    "unsupportedSyntax",
                    vec![string_literal("invalid function call")],
                    &e.0,
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
                &e.0,
            )
        }
        // Convert `{ expr }` to `$v->splice( expr )`
        ETSplice(e) => meth_call("splice", vec![*e.clone()], &e.0),
        // Convert anything else to $v->unsupportedSyntax().
        // Type checking should prevent us hitting these cases.
        _ => meth_call(
            "unsupportedSyntax",
            vec![string_literal(&format!("{:#?}", &e.1))],
            &e.0,
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
            Some(e) => Some(meth_call("returnStatement", vec![rewrite_expr(&e)], &s.0)),
            // Convert `return;` to `$v->returnStatement(null)`.
            None => Some(meth_call("returnStatement", vec![null_literal()], &s.0)),
        },
        // Convert `if (...) {...} else {...}` to `$v-ifStatement($v->..., vec[...], vec[...])`.
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
                    &s.0,
                ))
            }
        },
        // Convert `while (...) {...}` to `$v->whileStatement($v->..., vec[...])`.
        While(w) => match &**w {
            (e, body) => {
                let body_stmts = rewrite_stmts(body);

                Some(meth_call(
                    "whileStatement",
                    vec![rewrite_expr(&e), vec_literal(body_stmts)],
                    &s.0,
                ))
            }
        },
        // Convert `break;` to `$v->breakStatement()`
        Break => Some(meth_call("breakStatement", vec![], &s.0)),
        // Convert `continue;` to `$v->continueStatement()`
        Continue => Some(meth_call("continueStatement", vec![], &s.0)),
        Noop => None,
        // Convert anything else to $v->unsupportedSyntax().
        // Type checking should prevent us hitting these cases.
        _ => Some(meth_call(
            "unsupportedSyntax",
            vec![string_literal(&format!("{:#?}", &s.1))],
            &s.0,
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
    let positions: Vec<_> = items.iter().map(|x| &x.0).collect();
    let position = merge_positions(&positions);
    let fields: Vec<_> = items.into_iter().map(|e| ast::Afield::AFvalue(e)).collect();
    Expr::new(
        position,
        Expr_::Collection(Box::new((make_id("vec"), None, fields))),
    )
}

fn make_id(name: &str) -> ast::Id {
    ast::Id(Pos::make_none(), name.into())
}

/// Build `$v->meth_name(args)`.
fn meth_call(meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    let receiver = Expr::mk_lvar(pos, "$v");
    let meth = Expr::new(
        pos.clone(),
        Expr_::Id(Box::new(ast::Id(pos.clone(), meth_name.into()))),
    );

    let c = Expr_::Call(Box::new((
        Expr::new(
            pos.clone(),
            Expr_::ObjGet(Box::new((receiver, meth, OgNullFlavor::OGNullthrows))),
        ),
        vec![],
        args,
        None,
    )));
    Expr::new(pos.clone(), c)
}

/// Join a slice of positions together into a single, larger position.
fn merge_positions(positions: &[&Pos]) -> Pos {
    positions
        .iter()
        .fold(None, |acc, pos| match acc {
            Some(res) => Some(Pos::merge(&res, pos).expect("Positions should be in the same file")),
            None => Some((*pos).clone()),
        })
        .unwrap_or(Pos::make_none())
}

/// Extracts all the expression tree splices and replaces them with
/// placeholder variables.
///
/// ```
/// $c = Code`__splice__($x->foo()) + __splice__($y);
/// $c_after = Code`$__splice_1 + $__splice_2`;
/// ```
///
/// Returns the updated Expr and a vec of the extracted spliced expr
/// representing `vec![$x->foo(), $y]`.
fn extract_and_replace_splices(e: &Expr) -> (Expr, Vec<Expr>) {
    let mut e_copy = e.clone();

    let mut visitor = SpliceExtractor {
        extracted_splices: vec![],
    };
    visitor.visit_expr(&mut (), &mut e_copy).unwrap();
    return (e_copy, visitor.extracted_splices);
}

struct SpliceExtractor {
    extracted_splices: Vec<Expr>,
}

impl<'ast> VisitorMut<'ast> for SpliceExtractor {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr_(&mut self, env: &mut (), e: &mut Expr_) -> Result<(), ()> {
        use aast::Expr_::*;
        match e {
            ETSplice(ex) => {
                let len = self.extracted_splices.len();
                self.extracted_splices.push((**ex).clone());
                *e = ETSplice(Box::new(temp_lvar(&ex.0, len)));
            }
            _ => e.recurse(env, self.object())?,
        }
        Ok(())
    }
}

fn temp_lvar(pos: &Pos, num: usize) -> Expr {
    Expr::mk_lvar(pos, &(format!("$__{}", num.to_string())))
}
