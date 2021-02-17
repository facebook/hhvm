use crate::lowerer::Env;
use bstr::BString;
use ocamlrep::rc::RcOc;
use oxidized::{
    aast,
    aast_visitor::{visit, AstParams, Node, NodeMut, Visitor, VisitorMut},
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

    let mut e = e.clone();
    let mut e = virtualize_expr_types(visitor_name.to_string(), &mut e);
    let mut e = virtualize_void_returns(visitor_name.to_string(), &mut e);
    let e = virtualize_expr_calls(visitor_name.to_string(), &mut e);
    let (e, extracted_splices) = extract_and_replace_splices(&e);
    let splice_count = extracted_splices.len();
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

    // Create dict of spliced values
    let key_value_pairs = (0..splice_count)
        .into_iter()
        .map(|i| {
            let key = Expr::new(
                Pos::make_none(),
                Expr_::String(BString::from(temp_lvar_string(i))),
            );
            let value = temp_lvar(&Pos::make_none(), i);
            (key, value)
        })
        .collect();
    let spliced_dict = dict_literal(key_value_pairs);

    // Make anonymous function of smart constructor calls
    let visitor_expr = wrap_return(rewrite_expr(env, &e), &temp_pos);
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
        readonly: None,
        user_attributes: vec![],
        visibility: None,
    };
    let visitor_fun_ = wrap_fun_(visitor_body, vec![param], temp_pos.clone(), env);
    let visitor_lambda = Expr::new(temp_pos.clone(), Expr_::mk_lfun(visitor_fun_, vec![]));

    let inferred_type = if env.codegen {
        // null
        dummy_expr()
    } else {
        // Make anonymous function for typing purposes
        let typing_fun_body = ast::FuncBody {
            ast: vec![wrap_return(e, &temp_pos)],
            annotation: (),
        };
        let typing_fun_ = wrap_fun_(typing_fun_body, vec![], temp_pos.clone(), env);
        let spliced_vars = (0..splice_count)
            .into_iter()
            .map(|i| ast::Lid(Pos::make_none(), (0, temp_lvar_string(i))))
            .collect();
        Expr::new(temp_pos.clone(), Expr_::mk_efun(typing_fun_, spliced_vars))
    };

    // Make `return Visitor::makeTree(...)`
    let return_stmt = wrap_return(
        static_meth_call(
            &visitor_name,
            "makeTree",
            vec![
                exprpos(&temp_pos),
                spliced_dict,
                visitor_lambda,
                inferred_type,
            ],
            &temp_pos.clone(),
        ),
        &temp_pos.clone(),
    );

    // Add to the body of the thunk after the splice assignments
    thunk_body.push(return_stmt);

    immediately_invoked_lambda(env, &temp_pos, thunk_body)
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
        readonly_ret: None,
        ret: ast::TypeHint((), None),
        name: make_id(";anonymous"),
        tparams: vec![],
        where_constraints: vec![],
        variadic: aast::FunVariadicity::FVnonVariadic,
        params,
        body,
        fun_kind: ast::FunKind::FSync,
        ctxs: None,        // TODO(T70095684)
        unsafe_ctxs: None, // TODO(T70095684)
        user_attributes: vec![],
        file_attributes: vec![],
        external: false,
        doc_comment: None,
        namespace: RcOc::clone(&env.empty_ns_env),
        static_: false,
    }
}

/// Virtualizes expressions that could leak Hack type semantics
///   Converts literals, operators, and implicit boolean checks
fn virtualize_expr_types(visitor_name: String, mut e: &mut Expr) -> &mut Expr {
    let mut visitor = TypeVirtualizer { visitor_name };
    visitor.visit_expr(&mut (), &mut e).unwrap();
    e
}

struct TypeVirtualizer {
    visitor_name: String,
}

fn dummy_expr() -> Expr {
    Expr::new(Pos::make_none(), aast::Expr_::Null)
}

// Converts `expr` to `expr->__bool()`
fn coerce_to_bool(receiver: &mut ast::Expr) -> ast::Expr {
    let pos = receiver.0.clone();
    let receiver = std::mem::replace(receiver, dummy_expr());
    meth_call(receiver, "__bool", vec![], &pos)
}

impl<'ast> VisitorMut<'ast> for TypeVirtualizer {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &mut Expr) -> Result<(), ()> {
        fn virtualize_binop(lhs: &mut Expr, meth_name: &str, rhs: &mut Expr, pos: &Pos) -> Expr {
            let lhs = std::mem::replace(lhs, dummy_expr());
            let rhs = std::mem::replace(rhs, dummy_expr());
            meth_call(lhs, meth_name, vec![rhs], pos)
        }

        fn virtualize_unop(operand: &mut Expr, meth_name: &str, pos: &Pos) -> Expr {
            let operand = std::mem::replace(operand, dummy_expr());
            meth_call(operand, meth_name, vec![], pos)
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
            // Convert `1.0` to `__splice__(Visitor::floatLiteral(1.0))`.
            Float(_) => {
                *e = mk_splice(static_meth_call(
                    &self.visitor_name,
                    "floatLiteral",
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
                    // Convert arithmetic operators `... + ...` to `$lhs->__plus(vec[$rhs])`
                    Bop::Plus => *e = virtualize_binop(lhs, "__plus", rhs, &e.0),
                    Bop::Minus => *e = virtualize_binop(lhs, "__minus", rhs, &e.0),
                    Bop::Star => *e = virtualize_binop(lhs, "__star", rhs, &e.0),
                    Bop::Slash => *e = virtualize_binop(lhs, "__slash", rhs, &e.0),
                    // Convert boolean &&, ||
                    Bop::Ampamp => *e = virtualize_binop(lhs, "__ampamp", rhs, &e.0),
                    Bop::Barbar => *e = virtualize_binop(lhs, "__barbar", rhs, &e.0),
                    // Convert comparison operators, <, <=, >, >=, ===, !==
                    Bop::Lt => *e = virtualize_binop(lhs, "__lessThan", rhs, &e.0),
                    Bop::Lte => *e = virtualize_binop(lhs, "__lessThanEqual", rhs, &e.0),
                    Bop::Gt => *e = virtualize_binop(lhs, "__greaterThan", rhs, &e.0),
                    Bop::Gte => *e = virtualize_binop(lhs, "__greaterThanEqual", rhs, &e.0),
                    Bop::Eqeqeq => *e = virtualize_binop(lhs, "__tripleEquals", rhs, &e.0),
                    Bop::Diff2 => *e = virtualize_binop(lhs, "__notTripleEquals", rhs, &e.0),
                    // Assignment is special and not virtualized
                    Bop::Eq(None) => {}
                    // The rest should be parser errors from expression_tree_check
                    _ => {}
                }
            }
            Unop(ref mut unop) => {
                let (ref op, ref mut operand) = **unop;
                // Recurse into the operand
                operand.accept(env, self.object())?;

                match op {
                    Uop::Unot => *e = virtualize_unop(operand, "__exclamationMark", &e.0),
                    // The rest should be parser errors from expression_tree_check
                    _ => {}
                }
            }
            // Convert `condition ? e1 : e2` to
            //   `condition->__bool() ? e1 : e2`
            Eif(ref mut eif) => {
                let (ref mut e1, ref mut e2, ref mut e3) = **eif;
                e1.accept(env, self.object())?;
                e2.accept(env, self.object())?;
                e3.accept(env, self.object())?;

                let e2 = e2.take();
                let e3 = std::mem::replace(e3, dummy_expr());
                *e = Expr::new(pos, Eif(Box::new((coerce_to_bool(e1), e2, e3))))
            }
            _ => e.recurse(env, self.object())?,
        }
        Ok(())
    }

    fn visit_stmt_(&mut self, env: &mut (), s: &mut Stmt_) -> Result<(), ()> {
        use aast::Stmt_::*;

        match s {
            // Convert `while(condition) { block }` to
            //   `while(condition->coerceToBool()) { block }`
            While(ref mut w) => {
                let (ref mut condition, ref mut block) = **w;
                condition.accept(env, self.object())?;
                block.accept(env, self.object())?;

                let block = std::mem::replace(block, vec![]);
                *s = While(Box::new((coerce_to_bool(condition), block)))
            }
            // Convert `if(condition) { block }` to
            //   `if(condition->coerceToBool()) { block }`
            If(i) => {
                let (ref mut condition, ref mut b1, ref mut b2) = **i;
                condition.accept(env, self.object())?;
                b1.accept(env, self.object())?;
                b2.accept(env, self.object())?;

                let b1 = std::mem::replace(b1, vec![]);
                let b2 = std::mem::replace(b2, vec![]);
                *s = If(Box::new((coerce_to_bool(condition), b1, b2)))
            }
            // Convert `for(i; condition; j) { block }` to
            //   `for(i; condition->coerceToBool(); j) { block }`
            For(f) => {
                let (ref mut inits, ref mut condition, ref mut increments, ref mut block) = **f;
                inits.accept(env, self.object())?;
                increments.accept(env, self.object())?;
                block.accept(env, self.object())?;

                let inits = std::mem::replace(inits, vec![]);
                let increments = std::mem::replace(increments, vec![]);
                let block = std::mem::replace(block, vec![]);
                let condition = if let Some(c) = condition {
                    c.accept(env, self.object())?;
                    Some(coerce_to_bool(c))
                } else {
                    None
                };

                *s = For(Box::new((inits, condition, increments, block)))
            }
            _ => s.recurse(env, self.object())?,
        }
        Ok(())
    }
}

/// Virtualizes function calls
fn virtualize_expr_calls(visitor_name: String, mut e: &mut Expr) -> &mut Expr {
    let mut visitor = CallVirtualizer { visitor_name };
    visitor.visit_expr(&mut (), &mut e).unwrap();
    e
}

struct CallVirtualizer {
    visitor_name: String,
}

impl<'ast> VisitorMut<'ast> for CallVirtualizer {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &mut Expr) -> Result<(), ()> {
        use aast::Expr_::*;

        let pos = e.0.clone();

        let mk_splice = |e: Expr| -> Expr { Expr::new(pos.clone(), Expr_::ETSplice(Box::new(e))) };

        match &mut e.1 {
            // Convert `foo(...)` to `__splice__(Visitor::symbol('foo', foo<>))(...)`
            Call(ref mut call) => {
                let (ref recv, ref mut targs, ref mut args, ref mut variadic) = **call;
                match &recv.1 {
                    Id(sid) => {
                        let fn_name = string_literal(&*sid.1);
                        targs.accept(env, self.object())?;
                        let targs = std::mem::replace(targs, vec![]);

                        let fp = Expr::new(
                            pos.clone(),
                            Expr_::FunctionPointer(Box::new((
                                ast::FunctionPtrId::FPId((**sid).clone()),
                                targs,
                            ))),
                        );
                        let callee = mk_splice(static_meth_call(
                            &self.visitor_name,
                            "symbol",
                            vec![fn_name, fp],
                            &pos,
                        ));

                        args.accept(env, self.object())?;
                        variadic.accept(env, self.object())?;

                        let args = std::mem::replace(args, vec![]);
                        let variadic = variadic.take();
                        e.1 = Call(Box::new((callee, vec![], args, variadic)))
                    }
                    // Convert `Foo::bar(...)` to `${ Visitor::symbol('Foo::bar', Foo::bar<>) }(...)`
                    ClassConst(cc) => {
                        let (ref cid, ref s) = **cc;
                        let fn_name = if let ClassId_::CIexpr(Expr(_, Id(sid))) = &cid.1 {
                            let name = format!("{}::{}", &*sid.1, &s.1);
                            string_literal(&name)
                        } else {
                            // Should be unreachable
                            string_literal("__ILLEGAL_STATIC_CALL_IN_EXPRESSION_TREE")
                        };

                        targs.accept(env, self.object())?;
                        let targs = std::mem::replace(targs, vec![]);

                        let fp = Expr::new(
                            pos.clone(),
                            Expr_::FunctionPointer(Box::new((
                                aast::FunctionPtrId::FPClassConst(cid.clone(), s.clone()),
                                targs,
                            ))),
                        );

                        let callee = mk_splice(static_meth_call(
                            &self.visitor_name,
                            "symbol",
                            vec![fn_name, fp],
                            &pos,
                        ));

                        args.accept(env, self.object())?;
                        variadic.accept(env, self.object())?;

                        let args = std::mem::replace(args, vec![]);
                        let variadic = variadic.take();
                        e.1 = Call(Box::new((callee, vec![], args, variadic)))
                    }
                    _ => e.recurse(env, self.object())?,
                }
            }
            // Do not want to recurse into splices
            ETSplice(_) => {}
            _ => e.recurse(env, self.object())?,
        }
        Ok(())
    }
}

struct VoidReturnCheck {
    only_void_return: bool,
}

impl<'ast> Visitor<'ast> for VoidReturnCheck {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &aast::Expr<Pos, (), (), ()>) -> Result<(), ()> {
        use aast::Expr_::*;

        match &e.1 {
            // Don't recurse into splices or LFuns
            ETSplice(_) | Lfun(_) => Ok(()),
            // TODO: Do we even recurse on expressions?
            _ => e.recurse(env, self),
        }
    }

    fn visit_stmt(&mut self, env: &mut (), s: &'ast aast::Stmt<Pos, (), (), ()>) -> Result<(), ()> {
        use aast::Stmt_::*;

        match &s.1 {
            Return(e) => {
                if (*e).is_some() {
                    self.only_void_return = false;
                }
                Ok(())
            }
            _ => s.recurse(env, self),
        }
    }
}

fn only_void_return(lfun_body: &ast::Block) -> bool {
    let mut checker = VoidReturnCheck {
        only_void_return: true,
    };
    visit(&mut checker, &mut (), lfun_body).unwrap();
    checker.only_void_return
}

fn virtualize_void_returns(visitor_name: String, mut e: &mut Expr) -> &mut Expr {
    let mut visitor = ReturnVirtualizer { visitor_name };
    visitor.visit_expr(&mut (), &mut e).unwrap();
    e
}

struct ReturnVirtualizer {
    visitor_name: String,
}

impl<'ast> VisitorMut<'ast> for ReturnVirtualizer {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &mut Expr) -> Result<(), ()> {
        use aast::Expr_::*;

        let should_append_return = match &e.1 {
            Lfun(lf) => only_void_return(&lf.0.body.ast),
            _ => false,
        };

        match &mut e.1 {
            Lfun(ref mut lf) if should_append_return => {
                // If the body of a lambda doesn't have a return as the last statement
                if lf.0.body.ast.is_empty() {
                    lf.0.body.ast =
                        vec![Stmt::new(e.0.clone(), ast::Stmt_::Return(Box::new(None)))];
                } else {
                    // If the body of a lambda doesn't have a return as the last statement
                    match lf.0.body.ast[lf.0.body.ast.len() - 1].1 {
                        ast::Stmt_::Return(_) => {}
                        _ => {
                            lf.0.body
                                .ast
                                .push(Stmt::new(e.0.clone(), ast::Stmt_::Return(Box::new(None))))
                        }
                    }
                }
                e.recurse(env, self.object())?
            }
            // Do not recurse into splices
            ETSplice(_) => {}
            _ => e.recurse(env, self.object())?,
        }
        Ok(())
    }

    fn visit_stmt(&mut self, env: &mut (), s: &mut Stmt) -> Result<(), ()> {
        let pos = s.0.clone();

        let mk_splice = |e: Expr| -> Expr { Expr::new(pos.clone(), Expr_::ETSplice(Box::new(e))) };

        match s.1 {
            Stmt_::Return(ref mut e) => {
                if None == **e {
                    // Virtualize void returns to client void
                    *s = Stmt::new(
                        s.0.clone(),
                        Stmt_::Return(Box::new(Some(mk_splice(static_meth_call(
                            &self.visitor_name,
                            "voidLiteral",
                            vec![],
                            &pos,
                        ))))),
                    )
                } else {
                    s.recurse(env, self.object())?
                }
            }
            _ => s.recurse(env, self.object())?,
        }
        Ok(())
    }
}

/// Convert expression tree expressions to method calls.
fn rewrite_expr<TF>(env: &Env<TF>, e: &Expr) -> Expr {
    use aast::Expr_::*;

    let pos = exprpos(&e.0);
    match &e.1 {
        // Convert `$x` to `$v->localVar(new ExprPos(...), "$x")` (note the quoting).
        Lvar(lid) => v_meth_call("localVar", vec![pos, string_literal(&((lid.1).1))], &e.0),
        // Convert `... = ...` to `$v->assign(new ExprPos(...), $v->..., $v->...)`.
        Binop(bop) => match &**bop {
            (Bop::Eq(None), lhs, rhs) => v_meth_call(
                "assign",
                vec![pos, rewrite_expr(env, &lhs), rewrite_expr(env, &rhs)],
                &e.0,
            ),
            _ => throw_exception_expr(
                env,
                &e.0,
                &format!("Unsupported syntax: binary operator: {:#?}", &bop.0),
            ),
        },
        // Convert ... ? ... : ... to `$v->ternary(new ExprPos(...), $v->..., $v->..., $v->...)`
        Eif(eif) => {
            let (e1, e2o, e3) = &**eif;
            let e2 = if let Some(e2) = e2o {
                rewrite_expr(env, &e2)
            } else {
                null_literal()
            };
            v_meth_call(
                "ternary",
                vec![pos, rewrite_expr(env, &e1), e2, rewrite_expr(env, &e3)],
                &e.0,
            )
        }
        Call(call) => {
            let (recv, _, args, _) = &**call;
            match &recv.1 {
                // Convert `$foo->bar(args)` to
                // `$v->methCall(new ExprPos(...), $foo, 'bar', vec[args])`
                // Parenthesized expressions e.g. `($foo->bar)(args)` unsupported.
                ObjGet(objget) if !objget.as_ref().3 => {
                    let (receiver, meth, _, _) = &**objget;
                    match &meth.1 {
                        Id(sid) => {
                            let fn_name = string_literal(&*sid.1);
                            let desugared_args = vec![
                                pos,
                                rewrite_expr(env, &receiver),
                                fn_name,
                                vec_literal(args.iter().map(|e| rewrite_expr(env, e)).collect()),
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
                // Convert expr( ... )(args) to `$v->call(new ExprPos(..), rewrite_expr(expr), vec[args])`
                _ => {
                    let args = vec![
                        pos,
                        rewrite_expr(env, recv),
                        vec_literal(args.iter().map(|e| rewrite_expr(env, e)).collect()),
                    ];
                    v_meth_call("call", args, &e.0)
                }
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
            let body_stmts = rewrite_stmts(env, &fun_.body.ast);

            v_meth_call(
                "lambdaLiteral",
                vec![pos, vec_literal(param_names), vec_literal(body_stmts)],
                &e.0,
            )
        }
        // Convert `{ expr }` to `$v->splice(new ExprPos(...), "\$var_name", expr )`
        ETSplice(e) => {
            // Assumes extract and replace has already occurred
            let s = if let Lvar(lid) = &e.1 {
                let aast::Lid(_, (_, lid)) = &**lid;
                Expr::new(Pos::make_none(), Expr_::String(BString::from(lid.clone())))
            } else {
                null_literal()
            };
            v_meth_call("splice", vec![pos, s, *e.clone()], &e.0)
        }
        // Convert anything else to `throw new Exception()`.
        // We should have already produced a parse error, and an exception prevents
        // us seeing type errors too.
        _ => throw_exception_expr(env, &e.0, &format!("Unsupported syntax: {:#?}", &e.1)),
    }
}

/// Convert expression tree statements to method calls.
fn rewrite_stmts<TF>(env: &Env<TF>, stmts: &[Stmt]) -> Vec<Expr> {
    stmts.iter().filter_map(|s| rewrite_stmt(env, s)).collect()
}

fn rewrite_stmt<TF>(env: &Env<TF>, s: &Stmt) -> Option<Expr> {
    use aast::Stmt_::*;

    let pos = exprpos(&s.0);

    match &s.1 {
        Expr(e) => Some(rewrite_expr(env, &e)),
        Return(e) => match &**e {
            // Convert `return ...;` to `$v->returnStatement(new ExprPos(...), $v->...)`.
            Some(e) => Some(v_meth_call(
                "returnStatement",
                vec![pos, rewrite_expr(env, &e)],
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
            let then_stmts = rewrite_stmts(env, then_block);
            let else_stmts = rewrite_stmts(env, else_block);

            Some(v_meth_call(
                "ifStatement",
                vec![
                    pos,
                    rewrite_expr(env, &e),
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
            let body_stmts = rewrite_stmts(env, body);

            Some(v_meth_call(
                "whileStatement",
                vec![pos, rewrite_expr(env, &e), vec_literal(body_stmts)],
                &s.0,
            ))
        }
        // Convert `for (...; ...; ...) {...}` to
        // `$v->forStatement(new ExprPos(...), vec[...], ..., vec[...], vec[...])`.
        For(w) => {
            let (init, cond, incr, body) = &**w;
            let init_exprs = init.iter().map(|e| rewrite_expr(env, e)).collect();
            let cond_expr = match cond {
                Some(cond) => rewrite_expr(env, cond),
                None => null_literal(),
            };
            let incr_exprs = incr.iter().map(|e| rewrite_expr(env, e)).collect();

            let body_stmts = rewrite_stmts(env, body);

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
        // Convert anything else to `throw new Exception()`.
        // We should have already produced a parse error, and an exception prevents
        // us seeing type errors too.
        _ => Some(throw_exception_expr(
            env,
            &s.0,
            &format!("Unsupported syntax: {:#?}", &s.1),
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

fn dict_literal(key_value_pairs: Vec<(Expr, Expr)>) -> Expr {
    let pos = Pos::make_none();
    let fields = key_value_pairs
        .into_iter()
        .map(|(k, v)| ast::Afield::AFkvalue(k, v))
        .collect();
    Expr::new(
        pos,
        Expr_::Collection(Box::new((make_id("dict"), None, fields))),
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
            Expr_::ObjGet(Box::new((
                receiver,
                meth,
                OgNullFlavor::OGNullthrows,
                false,
            ))),
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
            Expr_::ObjGet(Box::new((
                receiver,
                meth,
                OgNullFlavor::OGNullthrows,
                false,
            ))),
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
/// $c = Code`${$x->foo()} + ${$y};
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

fn temp_lvar_string(num: usize) -> String {
    format!("$__{}", num.to_string())
}

fn temp_lvar(pos: &Pos, num: usize) -> Expr {
    Expr::mk_lvar(pos, &temp_lvar_string(num))
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
                Expr::new(Pos::make_none(), Expr_::Id(Box::new(make_id("__FILE__")))),
                int_literal(start_lnum),
                int_literal(start_cnum - start_bol),
                int_literal(end_lnum),
                int_literal(end_cnum - end_bol),
            ],
        )
    }
}

fn new_exception(pos: &Pos, msg: &str) -> Expr {
    Expr::new(
        pos.clone(),
        Expr_::New(Box::new((
            ClassId(
                pos.clone(),
                ClassId_::CIexpr(Expr::new(
                    pos.clone(),
                    Expr_::Id(Box::new(Id(pos.clone(), "\\Exception".to_string()))),
                )),
            ),
            vec![],
            vec![string_literal(msg)],
            None,
            pos.clone(),
        ))),
    )
}

/// Build a statement `throw new Exception("msg here")`.
fn throw_exception(pos: &Pos, msg: &str) -> Stmt {
    Stmt::new(
        pos.clone(),
        Stmt_::Throw(Box::new(new_exception(&pos, msg))),
    )
}

/// Build an expression that throws an exception.
///
/// ```
/// (() ==> { throw new Exception("msg here"); })()
/// ```
fn throw_exception_expr<TF>(env: &Env<TF>, pos: &Pos, msg: &str) -> Expr {
    immediately_invoked_lambda(env, pos, vec![throw_exception(pos, msg)])
}

/// Wrap `stmts` in a lambda that's immediately called.
///
/// ```
/// (() ==> { foo; bar; })()
/// ```
fn immediately_invoked_lambda<TF>(env: &Env<TF>, pos: &Pos, stmts: Vec<Stmt>) -> Expr {
    let func_body = ast::FuncBody {
        ast: stmts,
        annotation: (),
    };
    let fun_ = wrap_fun_(func_body, vec![], pos.clone(), env);
    let lambda_expr = Expr::new(pos.clone(), Expr_::mk_lfun(fun_, vec![]));

    Expr::new(
        pos.clone(),
        Expr_::Call(Box::new((lambda_expr, vec![], vec![], None))),
    )
}
