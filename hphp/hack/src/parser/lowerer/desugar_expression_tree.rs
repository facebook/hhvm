use crate::lowerer::Env;
use bstr::BString;
use ocamlrep::rc::RcOc;
use oxidized::{
    aast,
    aast_visitor::{AstParams, NodeMut, VisitorMut},
    ast,
    ast::{ClassId, ClassId_, Expr, Expr_, Hint_, Stmt, Stmt_},
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
pub fn desugar<TF>(hint: &aast::Hint, e: &Expr, env: &Env<TF>) -> Expr {
    let visitor_name = {
        if let Hint_::Happly(id, _) = &*hint.1 {
            &id.1
        } else {
            ""
        }
    };

    let e = virtualize_expr(visitor_name.to_string(), e);
    let (e, extracted_splices) = extract_and_replace_splices(&e);
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
            Stmt_::Throw(Box::new(new_obj(&temp_pos, "\\Exception", vec![]))),
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

    // Make `return new ExprTree(...)`
    let return_stmt = wrap_return(
        new_obj(
            &temp_pos,
            "\\ExprTree",
            vec![
                exprpos(&temp_pos),
                Expr::new(temp_pos.clone(), Expr_::Id(Box::new(make_id("__FILE__")))),
                visitor_lambda,
                typing_lambda,
            ],
        ),
        &temp_pos.clone(),
    );

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
fn wrap_fun_<TF>(
    body: ast::FuncBody,
    params: Vec<ast::FunParam>,
    pos: Pos,
    env: &Env<TF>,
) -> ast::Fun_ {
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

/// Preprocess expression
///   Virtualizes literals
///   Reduces operators to method calls
///   TODO: Transforms function calls
fn virtualize_expr(visitor_name: String, e: &Expr) -> Expr {
    let mut e_copy = e.clone();

    let mut visitor = Virtualizer { visitor_name };
    visitor.visit_expr(&mut (), &mut e_copy).unwrap();
    e_copy
}

struct Virtualizer {
    visitor_name: String,
}

impl<'ast> VisitorMut<'ast> for Virtualizer {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &mut Expr) -> Result<(), ()> {
        fn dummy_expr() -> Expr {
            Expr::new(Pos::make_none(), Expr_::Null)
        }

        fn virtualize_binop(lhs: &mut Expr, meth_name: &str, rhs: &mut Expr, pos: &Pos) -> Expr {
            let lhs = std::mem::replace(lhs, dummy_expr());
            let rhs = std::mem::replace(rhs, dummy_expr());
            meth_call(lhs, meth_name, vec![rhs], pos)
        }

        use aast::Expr_::*;

        let pos = e.0.clone();

        let mk_splice = |e: Expr| -> Expr { Expr::new(pos.clone(), Expr_::ETSplice(Box::new(e))) };

        match &mut e.1 {
            // Convert `1` to `__splice__(Visitor::intLiteral(1))`.
            Int(_) => {
                *e = mk_splice(static_meth_call(
                    &self.visitor_name,
                    "intLiteral",
                    vec![e.clone()],
                    &pos,
                ))
            }
            // Convert `"foo"` to `__splice__(Visitor::stringLiteral("foo"))`
            String(_) => {
                *e = mk_splice(static_meth_call(
                    &self.visitor_name,
                    "stringLiteral",
                    vec![e.clone()],
                    &pos,
                ))
            }
            // Convert `true` to `__splice__(Visitor::boolLiteral(true))`
            True | False => {
                *e = mk_splice(static_meth_call(
                    &self.visitor_name,
                    "boolLiteral",
                    vec![e.clone()],
                    &pos,
                ))
            }
            // Convert `null` to `__splice__(Visitor::nullLiteral())`
            Null => {
                *e = mk_splice(static_meth_call(
                    &self.visitor_name,
                    "nullLiteral",
                    vec![],
                    &pos,
                ))
            }
            // Do not want to recurse into splices
            ETSplice(_) => {}
            Binop(ref mut bop) => {
                let (ref op, ref mut lhs, ref mut rhs) = **bop;
                // Recurse down the left and right hand sides
                lhs.accept(env, self.object())?;
                rhs.accept(env, self.object())?;

                match op {
                    // Convert `... + ...` to `$lhs->__plus(vec[$rhs])`.
                    Bop::Plus => *e = virtualize_binop(lhs, "__plus", rhs, &e.0),
                    // Convert `... && ...` to `$lhs->__ampamp(vec[rhs])`.
                    Bop::Ampamp => *e = virtualize_binop(lhs, "__ampamp", rhs, &e.0),
                    // Convert `... || ...` to `$lhs->__barbar(vec[$rhs])`.
                    Bop::Barbar => *e = virtualize_binop(lhs, "__barbar", rhs, &e.0),
                    // Assignment is special and not virtualized
                    Bop::Eq(None) => {}
                    // The rest should be parser errors from expression_tree_check
                    _ => {}
                }
            }
            _ => e.recurse(env, self.object())?,
        }
        Ok(())
    }
}

/// Convert expression tree expressions to method calls.
fn rewrite_expr(e: &Expr) -> Expr {
    use aast::Expr_::*;

    let pos = exprpos(&e.0);
    match &e.1 {
        // Convert `$x` to `$v->localVar(new ExprPos(...), "$x")` (note the quoting).
        Lvar(lid) => v_meth_call("localVar", vec![pos, string_literal(&((lid.1).1))], &e.0),
        // Convert `... = ...` to `$v->assign(new ExprPos(...), $v->..., $v->...)`.
        Binop(bop) => match &**bop {
            (Bop::Eq(None), lhs, rhs) => v_meth_call(
                "assign",
                vec![pos, rewrite_expr(&lhs), rewrite_expr(&rhs)],
                &e.0,
            ),
            _ => v_meth_call(
                "unsupportedSyntax",
                vec![string_literal("bad binary operator")],
                &e.0,
            ),
        },
        Unop(uop) => match &**uop {
            // Convert `!...` to `$v->exclamationMark(new ExprPos(...), $v->...)`.
            (Uop::Unot, operand) => {
                v_meth_call("exclamationMark", vec![pos, rewrite_expr(&operand)], &e.0)
            }
            _ => v_meth_call(
                "unsupportedSyntax",
                vec![string_literal("bad unary operator")],
                &e.0,
            ),
        },
        Call(call) => {
            let (recv, _, args, _) = &**call;
            match &recv.1 {
                // Convert `foo(...)` to `$v->call(new ExprPos(...), 'foo', vec[...])`.
                Id(sid) => {
                    let fn_name = string_literal(&*sid.1);
                    let desugared_args = vec![
                        pos,
                        fn_name,
                        vec_literal(args.iter().map(rewrite_expr).collect()),
                    ];
                    v_meth_call("call", desugared_args, &e.0)
                }
                // Convert `$foo->bar(args)` to
                // `$v->methCall(new ExprPos(...), $foo, 'bar', vec[args])`
                ObjGet(objget) => {
                    let (receiver, meth, _) = &**objget;
                    match &meth.1 {
                        Id(sid) => {
                            let fn_name = string_literal(&*sid.1);
                            let desugared_args = vec![
                                pos,
                                rewrite_expr(&receiver),
                                fn_name,
                                vec_literal(args.iter().map(rewrite_expr).collect()),
                            ];
                            v_meth_call("methCall", desugared_args, &e.0)
                        }
                        _ => v_meth_call(
                            "unsupportedSyntax",
                            vec![string_literal("invalid function call")],
                            &e.0,
                        ),
                    }
                }
                _ => v_meth_call(
                    "unsupportedSyntax",
                    vec![string_literal("invalid function call")],
                    &e.0,
                ),
            }
        }
        // Convert `($x) ==> { ... }` to `$v->lambdaLiteral(new ExprPos(...), vec["$x"], vec[...])`.
        Lfun(lf) => {
            let fun_ = &lf.0;
            let param_names = fun_
                .params
                .iter()
                .map(|p| string_literal(&p.name))
                .collect();
            let body_stmts = rewrite_stmts(&fun_.body.ast);

            v_meth_call(
                "lambdaLiteral",
                vec![pos, vec_literal(param_names), vec_literal(body_stmts)],
                &e.0,
            )
        }
        // Convert `{ expr }` to `$v->splice(new ExprPos(...), expr )`
        ETSplice(e) => v_meth_call("splice", vec![pos, *e.clone()], &e.0),
        // Convert anything else to $v->unsupportedSyntax().
        // Type checking should prevent us hitting these cases.
        _ => v_meth_call(
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

    let pos = exprpos(&s.0);

    match &s.1 {
        Expr(e) => Some(rewrite_expr(&e)),
        Return(e) => match &**e {
            // Convert `return ...;` to `$v->returnStatement(new ExprPos(...), $v->...)`.
            Some(e) => Some(v_meth_call(
                "returnStatement",
                vec![pos, rewrite_expr(&e)],
                &s.0,
            )),
            // Convert `return;` to `$v->returnStatement(new ExprPos(...), null)`.
            None => Some(v_meth_call(
                "returnStatement",
                vec![pos, null_literal()],
                &s.0,
            )),
        },
        // Convert `if (...) {...} else {...}` to
        // `$v->ifStatement(new ExprPos(...), $v->..., vec[...], vec[...])`.
        If(if_stmt) => {
            let (e, then_block, else_block) = &**if_stmt;
            let then_stmts = rewrite_stmts(then_block);
            let else_stmts = rewrite_stmts(else_block);

            Some(v_meth_call(
                "ifStatement",
                vec![
                    pos,
                    rewrite_expr(&e),
                    vec_literal(then_stmts),
                    vec_literal(else_stmts),
                ],
                &s.0,
            ))
        }
        // Convert `while (...) {...}` to
        // `$v->whileStatement(new ExprPos(...), $v->..., vec[...])`.
        While(w) => {
            let (e, body) = &**w;
            let body_stmts = rewrite_stmts(body);

            Some(v_meth_call(
                "whileStatement",
                vec![pos, rewrite_expr(&e), vec_literal(body_stmts)],
                &s.0,
            ))
        }
        // Convert `for (...; ...; ...) {...}` to
        // `$v->forStatement(new ExprPos(...), vec[...], ..., vec[...], vec[...])`.
        For(w) => {
            let (init, cond, incr, body) = &**w;
            let init_exprs = init.iter().map(rewrite_expr).collect();
            let cond_expr = match cond {
                Some(cond) => rewrite_expr(cond),
                None => null_literal(),
            };
            let incr_exprs = incr.iter().map(rewrite_expr).collect();

            let body_stmts = rewrite_stmts(body);

            Some(v_meth_call(
                "forStatement",
                vec![
                    pos,
                    vec_literal(init_exprs),
                    cond_expr,
                    vec_literal(incr_exprs),
                    vec_literal(body_stmts),
                ],
                &s.0,
            ))
        }
        // Convert `break;` to `$v->breakStatement(new ExprPos(...))`
        Break => Some(v_meth_call("breakStatement", vec![pos], &s.0)),
        // Convert `continue;` to `$v->continueStatement(new ExprPos(...))`
        Continue => Some(v_meth_call("continueStatement", vec![pos], &s.0)),
        Noop => None,
        // Convert anything else to $v->unsupportedSyntax().
        // Type checking should prevent us hitting these cases.
        _ => Some(v_meth_call(
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

fn int_literal(i: usize) -> Expr {
    Expr::new(Pos::make_none(), Expr_::Int(i.to_string()))
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

/// Build `new classname(args)`
fn new_obj(pos: &Pos, classname: &str, args: Vec<Expr>) -> Expr {
    Expr::new(
        pos.clone(),
        Expr_::New(Box::new((
            ClassId(
                pos.clone(),
                ClassId_::CIexpr(Expr::new(
                    pos.clone(),
                    Expr_::Id(Box::new(Id(pos.clone(), classname.to_string()))),
                )),
            ),
            vec![],
            args,
            None,
            pos.clone(),
        ))),
    )
}

/// Build `$v->meth_name(args)`.
fn v_meth_call(meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
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

fn meth_call(receiver: Expr, meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
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

fn static_meth_call(classname: &str, meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    let callee = Expr::new(
        pos.clone(),
        Expr_::ClassConst(Box::new((
            // TODO: Refactor ClassId creation with new_obj
            ClassId(
                pos.clone(),
                ClassId_::CIexpr(Expr::new(
                    pos.clone(),
                    Expr_::Id(Box::new(Id(pos.clone(), classname.to_string()))),
                )),
            ),
            (pos.clone(), meth_name.to_string()),
        ))),
    );
    Expr::new(
        pos.clone(),
        Expr_::Call(Box::new((callee, vec![], args, None))),
    )
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

/// Given a Pos, returns `new ExprPos(...)`
/// In case of Pos.none or invalid position, all elements set to 0
fn exprpos(pos: &Pos) -> Expr {
    if pos.is_none() || !pos.is_valid() {
        null_literal()
    } else {
        let ((start_lnum, start_bol, start_cnum), (end_lnum, end_bol, end_cnum)) =
            pos.to_start_and_end_lnum_bol_cnum();
        new_obj(
            &pos,
            "\\ExprPos",
            vec![
                int_literal(start_lnum),
                int_literal(start_cnum - start_bol),
                int_literal(end_lnum),
                int_literal(end_cnum - end_bol),
            ],
        )
    }
}
