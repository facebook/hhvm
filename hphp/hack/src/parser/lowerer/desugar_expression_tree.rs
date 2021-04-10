use crate::lowerer::Env;
use bstr::BString;
use naming_special_names_rust::classes;
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

/// Rewrite the contents of an expression tree literal into an
/// expression on a visitor class.
///
/// For example, given the following expression tree literal:
///
/// ```
/// $et = MyDsl`foo() + 1`;
/// ```
///
/// Transform the `foo() + 1` to:
///
/// ```
/// (() ==> {
///   // Splices are evaluated immediately.
///   $0splice0 = MyDsl::liftSymbol(foo<>);
///   $0splice1 = MyDsl::liftInt(1);
///
///   return MyDsl::makeTree(
///     // At runtime, expression tree visitors know the position of the literal.
///     new ExprPos("whatever.php", ...),
///
///     // Pass the splices outside of the visitor, so visitors can access the
///     // spliced values without having to re-run the visit function.
///     dict['$0splice0' => $0splice0, '$0splice1' => $0splice1],
///
///     // The visit function itself. Visitors define what they want to do when
///     // they see each piece of syntax. They might build an AST, or construct a
///     // SQL query.
///     function (MyDsl $v) {
///       // (ignoring ExprPos arguments for brevity)
///       return $v->visitMethCall(
///         $v->visitCall(
///           $v->splice('$0splice0', $0splice0),
///           vec[]),
///         "__plus",
///         vec[$v->splice('$0splice1, $0splice1)]);
///     },
///
///     // An additional function used for type checking.
///     function (MyDsl $v) {
///       return ${ $0splice0 }()->__plus(${ $0splice1 })
///     },
///   );
/// )()
/// ```
pub fn desugar<TF>(hint: &aast::Hint, mut e: Expr, env: &Env<TF>) -> Result<Expr, (Pos, String)> {
    let visitor_name = hint_name(hint);

    virtualize_expr_types(&visitor_name, &mut e)?;
    virtualize_void_returns(&visitor_name, &mut e);
    virtualize_expr_calls(&visitor_name, &mut e)?;

    let extracted_splices = extract_and_replace_splices(&mut e)?;
    let splice_count = extracted_splices.len();
    let et_literal_pos = e.0.clone();

    // Create dict of spliced values
    let key_value_pairs = extracted_splices
        .iter()
        .enumerate()
        .map(|(i, expr)| {
            let key = Expr::new(
                expr.0.clone(),
                Expr_::String(BString::from(temp_lvar_string(i))),
            );
            let value = temp_lvar(&expr.0, i);
            (key, value)
        })
        .collect();
    let spliced_dict = dict_literal(et_literal_pos.clone(), key_value_pairs);

    // Create assignments of extracted splices
    // `$0splice0 = spliced_expr0;`
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
    let visitor_expr = wrap_return(rewrite_expr(env, &e)?, &et_literal_pos);
    let visitor_body = ast::FuncBody {
        ast: vec![visitor_expr],
        annotation: (),
    };
    let param = ast::FunParam {
        annotation: hint.0.clone(),
        type_hint: ast::TypeHint((), Some(hint.clone())),
        is_variadic: false,
        pos: hint.0.clone(),
        name: visitor_variable(),
        expr: None,
        callconv: None,
        readonly: None,
        user_attributes: vec![],
        visibility: None,
    };
    let visitor_fun_ = wrap_fun_(visitor_body, vec![param], et_literal_pos.clone(), env);
    let visitor_lambda = Expr::new(et_literal_pos.clone(), Expr_::mk_lfun(visitor_fun_, vec![]));

    let inferred_type = if env.codegen {
        null_literal(et_literal_pos.clone())
    } else {
        // Make anonymous function for typing purposes
        let typing_fun_body = ast::FuncBody {
            ast: vec![wrap_return(e, &et_literal_pos)],
            annotation: (),
        };
        let typing_fun_ = wrap_fun_(typing_fun_body, vec![], et_literal_pos.clone(), env);
        let spliced_vars = (0..splice_count)
            .into_iter()
            .map(|i| ast::Lid(et_literal_pos.clone(), (0, temp_lvar_string(i))))
            .collect();
        Expr::new(
            et_literal_pos.clone(),
            Expr_::mk_efun(typing_fun_, spliced_vars),
        )
    };

    // Make `return Visitor::makeTree(...)`
    let return_stmt = wrap_return(
        static_meth_call(
            &visitor_name,
            "makeTree",
            vec![
                exprpos(&et_literal_pos),
                spliced_dict,
                visitor_lambda,
                inferred_type,
            ],
            &et_literal_pos.clone(),
        ),
        &et_literal_pos.clone(),
    );

    // Add to the body of the thunk after the splice assignments
    thunk_body.push(return_stmt);

    Ok(immediately_invoked_lambda(env, &et_literal_pos, thunk_body))
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
        span: pos.clone(),
        readonly_this: None,
        annotation: (),
        mode: file_info::Mode::Mstrict,
        readonly_ret: None,
        ret: ast::TypeHint((), None),
        name: make_id(pos, ";anonymous"),
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
    }
}

/// Virtualize syntax in `e`, so that it can be evaluated without
/// caring about operator semantics.
///
/// This handles literals (which become splices), operators (which
/// become method calls), and implicit boolean checks (also method
/// calls). This allows users to choose which types support which
/// operators inside their expression tree literals.
///
/// If we encounter an unsupported operator, return its position and
/// an error message.
fn virtualize_expr_types(visitor_name: &str, mut e: &mut Expr) -> Result<(), (Pos, String)> {
    let mut visitor = TypeVirtualizer { visitor_name };
    visitor.visit_expr(&mut (), &mut e)
}

struct TypeVirtualizer<'a> {
    visitor_name: &'a str,
}

// Only used as temporary replacements when modifying the AST
// This node should never show up in the final AST
fn dummy_expr() -> Expr {
    Expr::new(Pos::make_none(), aast::Expr_::Null)
}

// Converts `expr` to `expr->__bool()`
fn coerce_to_bool(receiver: &mut ast::Expr) -> ast::Expr {
    let pos = receiver.0.clone();
    let receiver = std::mem::replace(receiver, dummy_expr());
    meth_call(receiver, "__bool", vec![], &pos)
}

impl<'ast> VisitorMut<'ast> for TypeVirtualizer<'_> {
    type P = AstParams<(), (Pos, String)>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &mut Expr) -> Result<(), (Pos, String)> {
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
            // Always allow local variables $foo.
            Lvar(_) => {}
            // Allow function calls as long as the subexpressions are allowed.
            Call(ref mut call) => {
                let (ref mut recv, _targs, ref mut args, _variadic) = &mut **call;
                match recv.1 {
                    Id(_) => {
                        // Allow Id in a call, where it represents a
                        // call to a global function. We don't allow
                        // Id in other positions, where it represents
                        // a global constant.
                    }
                    ClassConst(_) => {
                        // Allow Foo::bar(), but don't allow Foo::bar
                        // in other positions (e.g. static property
                        // access).
                    }
                    _ => {
                        recv.accept(env, self.object())?;
                    }
                }
                args.accept(env, self.object())?;
            }
            ExpressionTree(_) => {
                // Ban: Foo` Foo`1` `;
                return Err((e.0.clone(), "Expression trees may not be nested.".into()));
            }
            // Convert `1` to `${ Visitor::liftInt(1) }`.
            Int(_) => {
                *e = mk_splice(static_meth_call(
                    self.visitor_name,
                    "liftInt",
                    vec![e.clone()],
                    &pos,
                ))
            }
            // Convert `1.0` to `${ Visitor::liftFloat(1.0) }`.
            Float(_) => {
                *e = mk_splice(static_meth_call(
                    self.visitor_name,
                    "liftFloat",
                    vec![e.clone()],
                    &pos,
                ))
            }
            // Convert `"foo"` to `${ Visitor::liftString("foo") }`
            String(_) => {
                *e = mk_splice(static_meth_call(
                    self.visitor_name,
                    "liftString",
                    vec![e.clone()],
                    &pos,
                ))
            }
            // Convert `true` to `${ Visitor::liftBool(true) }`
            True | False => {
                *e = mk_splice(static_meth_call(
                    self.visitor_name,
                    "liftBool",
                    vec![e.clone()],
                    &pos,
                ))
            }
            // Convert `null` to `${ Visitor::liftNull() }`
            Null => {
                *e = mk_splice(static_meth_call(
                    self.visitor_name,
                    "liftNull",
                    vec![],
                    &pos,
                ))
            }
            // Do not want to recurse into splices.
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
                    Bop::Percent => *e = virtualize_binop(lhs, "__percent", rhs, &e.0),
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
                    // Convert string concatenation
                    Bop::Dot => *e = virtualize_binop(lhs, "__dot", rhs, &e.0),
                    // Convert bitwise operators, &, |, ^, <<, >>
                    Bop::Amp => *e = virtualize_binop(lhs, "__amp", rhs, &e.0),
                    Bop::Bar => *e = virtualize_binop(lhs, "__bar", rhs, &e.0),
                    Bop::Xor => *e = virtualize_binop(lhs, "__caret", rhs, &e.0),
                    Bop::Ltlt => *e = virtualize_binop(lhs, "__lessThanLessThan", rhs, &e.0),
                    Bop::Gtgt => *e = virtualize_binop(lhs, "__greaterThanGreaterThan", rhs, &e.0),
                    // Assignment is special and not virtualized
                    Bop::Eq(None) => {}
                    _ => {
                        return Err((pos, "Expression trees only support comparison (`<`, `===` etc) and basic arithmetic operators (`+` etc).".into()));
                    }
                }
            }
            Unop(ref mut unop) => {
                let (ref op, ref mut operand) = **unop;
                // Recurse into the operand
                operand.accept(env, self.object())?;

                match op {
                    // Allow boolean not operator !$x
                    Uop::Unot => *e = virtualize_unop(operand, "__exclamationMark", &e.0),
                    // Allow negation -$x (required for supporting negative literals -123)
                    Uop::Uminus => *e = virtualize_unop(operand, "__negate", &e.0),
                    // Allow bitwise complement
                    Uop::Utild => *e = virtualize_unop(operand, "__tilde", &e.0),
                    _ => {
                        return Err((
                            pos,
                            "Expression trees do not support this unary operator.".into(),
                        ));
                    }
                }
            }
            // Convert `condition ? e1 : e2` to
            //   `condition->__bool() ? e1 : e2`
            Eif(ref mut eif) => {
                // Allow ternary _ ? _ : _, but not Elvis operator _ ?: _
                if eif.1.is_none() {
                    return Err((pos, "Expression trees do not support `_ ?: _` with two operands. Use `_ ? _ : _` instead.".into()));
                }


                let (ref mut e1, ref mut e2, ref mut e3) = **eif;
                e1.accept(env, self.object())?;
                e2.accept(env, self.object())?;
                e3.accept(env, self.object())?;

                let e2 = e2.take();
                let e3 = std::mem::replace(e3, dummy_expr());
                *e = Expr::new(pos, Eif(Box::new((coerce_to_bool(e1), e2, e3))))
            }
            // Allow lambdas () ==> { ... } but not PHP-style function() { ... }
            Lfun(ref mut lf) => {
                for param in &lf.0.params {
                    if param.expr.is_some() {
                        return Err((
                            param.pos.clone(),
                            "Expression trees do not support parameters with default values."
                                .into(),
                        ));
                    }
                }
                e.recurse(env, self.object())?;
            }
            _ => {
                return Err((pos, "Unsupported syntax for expression trees.".into()));
            }
        }
        Ok(())
    }

    fn visit_stmt_(&mut self, env: &mut (), s: &mut Stmt_) -> Result<(), (Pos, String)> {
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
fn virtualize_expr_calls(visitor_name: &str, mut e: &mut Expr) -> Result<(), (Pos, String)> {
    let mut visitor = CallVirtualizer { visitor_name };
    visitor.visit_expr(&mut (), &mut e)
}

struct CallVirtualizer<'a> {
    visitor_name: &'a str,
}

impl<'ast> VisitorMut<'ast> for CallVirtualizer<'_> {
    type P = AstParams<(), (Pos, String)>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &mut Expr) -> Result<(), (Pos, String)> {
        use aast::Expr_::*;

        let mk_splice = |e: Expr| -> Expr { Expr::new(e.0.clone(), Expr_::ETSplice(Box::new(e))) };

        match &mut e.1 {
            // Convert `foo(...)` to `${ Visitor::liftSymbol(foo<>) }(...)`
            Call(ref mut call) => {
                let (ref recv, ref mut targs, ref mut args, ref variadic) = **call;

                if let Some(variadic) = variadic {
                    return Err((
                        variadic.0.clone(),
                        "Expression trees do not support variadic calls.".into(),
                    ));
                }

                if let Some(targ) = targs.pop() {
                    let pos = (targ.1).0;
                    return Err((
                        pos,
                        "Expression trees do not support function calls with generics.".into(),
                    ));
                }

                match &recv.1 {
                    Id(sid) => {
                        let pos = sid.0.clone();
                        let fp = Expr::new(
                            pos.clone(),
                            Expr_::FunctionPointer(Box::new((
                                ast::FunctionPtrId::FPId((**sid).clone()),
                                vec![],
                            ))),
                        );
                        let callee = mk_splice(static_meth_call(
                            self.visitor_name,
                            "liftSymbol",
                            vec![fp],
                            &pos,
                        ));

                        args.accept(env, self.object())?;
                        let args = std::mem::replace(args, vec![]);
                        e.1 = Call(Box::new((callee, vec![], args, None)))
                    }
                    // Convert `Foo::bar(...)` to `${ Visitor::liftSymbol(Foo::bar<>) }(...)`
                    ClassConst(cc) => {
                        let pos = recv.0.clone();
                        let (ref cid, ref s) = **cc;
                        if let ClassId_::CIexpr(Expr(_, Id(sid))) = &cid.1 {
                            if sid.1 == classes::PARENT
                                || sid.1 == classes::SELF
                                || sid.1 == classes::STATIC
                            {
                                return Err((
                                    pos,
                                    "Static method calls in expression trees require explicit class names.".into(),
                                ));
                            }
                        } else {
                            return Err((
                                pos,
                                "Expression trees only support function calls and static method calls on named classes.".into()));
                        };

                        let fp = Expr::new(
                            pos.clone(),
                            Expr_::FunctionPointer(Box::new((
                                aast::FunctionPtrId::FPClassConst(cid.clone(), s.clone()),
                                vec![],
                            ))),
                        );

                        let callee = mk_splice(static_meth_call(
                            self.visitor_name,
                            "liftSymbol",
                            vec![fp],
                            &pos,
                        ));

                        args.accept(env, self.object())?;

                        let args = std::mem::replace(args, vec![]);
                        e.1 = Call(Box::new((callee, vec![], args, None)))
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

fn virtualize_void_returns(visitor_name: &str, mut e: &mut Expr) {
    let mut visitor = ReturnVirtualizer { visitor_name };
    visitor.visit_expr(&mut (), &mut e).unwrap();
}

struct ReturnVirtualizer<'a> {
    visitor_name: &'a str,
}

impl<'ast> VisitorMut<'ast> for ReturnVirtualizer<'_> {
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
                            self.visitor_name,
                            "liftVoid",
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
fn rewrite_expr<TF>(env: &Env<TF>, e: &Expr) -> Result<Expr, (Pos, String)> {
    use aast::Expr_::*;

    let pos = exprpos(&e.0);
    let e = match &e.1 {
        // Convert `$x` to `$v->visitLocal(new ExprPos(...), "$x")` (note the quoting).
        Lvar(lid) => v_meth_call(
            "visitLocal",
            vec![pos, string_literal(e.0.clone(), &((lid.1).1))],
            &e.0,
        ),
        // Convert `... = ...` to `$v->visitAssign(new ExprPos(...), $v->..., $v->...)`.
        Binop(bop) => match &**bop {
            (Bop::Eq(None), lhs, rhs) => v_meth_call(
                "visitAssign",
                vec![pos, rewrite_expr(env, &lhs)?, rewrite_expr(env, &rhs)?],
                &e.0,
            ),
            _ => {
                return Err((
                    e.0.clone(),
                    "Unsupported expression tree syntax: binary operator after virtualization. Please file a bug.".into(),
                ));
            }
        },
        // Convert ... ? ... : ... to `$v->visitTernary(new ExprPos(...), $v->..., $v->..., $v->...)`
        Eif(eif) => {
            let (e1, e2o, e3) = &**eif;
            let e2 = if let Some(e2) = e2o {
                rewrite_expr(env, &e2)?
            } else {
                null_literal(e.0.clone())
            };
            v_meth_call(
                "visitTernary",
                vec![pos, rewrite_expr(env, &e1)?, e2, rewrite_expr(env, &e3)?],
                &e.0,
            )
        }
        Call(call) => {
            let (recv, _, args, _) = &**call;
            match &recv.1 {
                // Convert `$foo->bar(args)` to
                // `$v->visitMethCall(new ExprPos(...), $foo, 'bar', vec[args])`
                // Parenthesized expressions e.g. `($foo->bar)(args)` unsupported.
                ObjGet(objget) if !objget.as_ref().3 => {
                    let (receiver, meth, _, _) = &**objget;
                    match &meth.1 {
                        Id(sid) => {
                            let fn_name = string_literal(sid.0.clone(), &*sid.1);
                            let desugared_args = vec![
                                pos,
                                rewrite_expr(env, &receiver)?,
                                fn_name,
                                vec_literal(rewrite_exprs(env, args)?),
                            ];
                            v_meth_call("visitMethCall", desugared_args, &e.0)
                        }
                        _ => {
                            return Err((
                                e.0.clone(),
                                "Unsupported expression tree syntax: call after virtualization. Please file a bug.".into(),
                            ));
                        }
                    }
                }
                // Convert expr( ... )(args) to `$v->visitCall(new ExprPos(..), rewrite_expr(expr), vec[args])`
                _ => {
                    let args = vec![
                        pos,
                        rewrite_expr(env, recv)?,
                        vec_literal(rewrite_exprs(env, args)?),
                    ];
                    v_meth_call("visitCall", args, &e.0)
                }
            }
        }
        // Convert `($x) ==> { ... }` to `$v->visitLambda(new ExprPos(...), vec["$x"], vec[...])`.
        Lfun(lf) => {
            let fun_ = &lf.0;

            let mut param_names = Vec::with_capacity(fun_.params.len());
            for param in &fun_.params {
                if param.expr.is_some() {
                    return Err((
                        param.pos.clone(),
                        "Expression trees do not support parameters with default values.".into(),
                    ));
                }
                param_names.push(string_literal(param.pos.clone(), &param.name));
            }

            let body_stmts = rewrite_stmts(env, &fun_.body.ast)?;
            v_meth_call(
                "visitLambda",
                vec![pos, vec_literal(param_names), vec_literal(body_stmts)],
                &e.0,
            )
        }
        // Convert `${ expr }` to `$v->splice(new ExprPos(...), "\$var_name", expr )`
        ETSplice(e) => {
            // Assumes extract and replace has already occurred
            let s = if let Lvar(lid) = &e.1 {
                let aast::Lid(_, (_, lid)) = &**lid;
                string_literal(e.0.clone(), lid)
            } else {
                null_literal(e.0.clone())
            };
            v_meth_call("splice", vec![pos, s, *e.clone()], &e.0)
        }
        _ => {
            return Err((
                e.0.clone(),
                "Unsupported expression tree syntax: expression after virtualization. Please file a bug.".into(),
            ));
        }
    };
    Ok(e)
}

fn rewrite_exprs<TF>(env: &Env<TF>, exprs: &[Expr]) -> Result<Vec<Expr>, (Pos, String)> {
    let mut result = Vec::with_capacity(exprs.len());
    for expr in exprs {
        result.push(rewrite_expr(env, expr)?);
    }
    Ok(result)
}

/// Convert expression tree statements to method calls.
fn rewrite_stmts<TF>(env: &Env<TF>, stmts: &[Stmt]) -> Result<Vec<Expr>, (Pos, String)> {
    let mut result = Vec::with_capacity(stmts.len());
    for stmt in stmts {
        match rewrite_stmt(env, stmt)? {
            Some(e) => {
                result.push(e);
            }
            None => {
                // Discard empty statements.
            }
        }
    }
    Ok(result)
}

fn rewrite_stmt<TF>(env: &Env<TF>, s: &Stmt) -> Result<Option<Expr>, (Pos, String)> {
    use aast::Stmt_::*;

    let pos = exprpos(&s.0);

    let e = match &s.1 {
        Expr(e) => Some(rewrite_expr(env, &e)?),
        Return(e) => match &**e {
            // Convert `return ...;` to `$v->visitReturn(new ExprPos(...), $v->...)`.
            Some(e) => Some(v_meth_call(
                "visitReturn",
                vec![pos, rewrite_expr(env, &e)?],
                &s.0,
            )),
            None => {
                return Err((
                    s.0.clone(),
                    "Unsupported expression tree syntax: empty return after void virtualization. Please file a bug.".into(),
                ));
            }
        },
        // Convert `if (...) {...} else {...}` to
        // `$v->visitIf(new ExprPos(...), $v->..., vec[...], vec[...])`.
        If(if_stmt) => {
            let (e, then_block, else_block) = &**if_stmt;
            let then_stmts = rewrite_stmts(env, then_block)?;
            let else_stmts = rewrite_stmts(env, else_block)?;

            Some(v_meth_call(
                "visitIf",
                vec![
                    pos,
                    rewrite_expr(env, &e)?,
                    vec_literal(then_stmts),
                    vec_literal(else_stmts),
                ],
                &s.0,
            ))
        }
        // Convert `while (...) {...}` to
        // `$v->visitWhile(new ExprPos(...), $v->..., vec[...])`.
        While(w) => {
            let (e, body) = &**w;
            let body_stmts = rewrite_stmts(env, body)?;

            Some(v_meth_call(
                "visitWhile",
                vec![pos, rewrite_expr(env, &e)?, vec_literal(body_stmts)],
                &s.0,
            ))
        }
        // Convert `for (...; ...; ...) {...}` to
        // `$v->visitFor(new ExprPos(...), vec[...], ..., vec[...], vec[...])`.
        For(w) => {
            let (init, cond, incr, body) = &**w;
            let init_exprs = rewrite_exprs(env, init)?;
            let cond_expr = match cond {
                Some(cond) => rewrite_expr(env, cond)?,
                None => null_literal(s.0.clone()),
            };
            let incr_exprs = rewrite_exprs(env, incr)?;

            let body_stmts = rewrite_stmts(env, body)?;

            Some(v_meth_call(
                "visitFor",
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
        // Convert `break;` to `$v->visitBreak(new ExprPos(...))`
        Break => Some(v_meth_call("visitBreak", vec![pos], &s.0)),
        // Convert `continue;` to `$v->visitContinue(new ExprPos(...))`
        Continue => Some(v_meth_call("visitContinue", vec![pos], &s.0)),
        Noop => None,
        _ => {
            return Err((
                s.0.clone(),
                "Expression trees do not support this statement syntax.".into(),
            ));
        }
    };
    Ok(e)
}

fn null_literal(pos: Pos) -> Expr {
    Expr::new(pos, Expr_::Null)
}

fn string_literal(pos: Pos, s: &str) -> Expr {
    Expr::new(pos, Expr_::String(BString::from(s)))
}

fn int_literal(pos: Pos, i: usize) -> Expr {
    Expr::new(pos, Expr_::Int(i.to_string()))
}

fn vec_literal(items: Vec<Expr>) -> Expr {
    let positions: Vec<_> = items.iter().map(|x| &x.0).collect();
    let position = merge_positions(&positions);
    let fields: Vec<_> = items.into_iter().map(|e| ast::Afield::AFvalue(e)).collect();
    Expr::new(
        position.clone(),
        Expr_::Collection(Box::new((make_id(position, "vec"), None, fields))),
    )
}

fn dict_literal(pos: Pos, key_value_pairs: Vec<(Expr, Expr)>) -> Expr {
    let fields = key_value_pairs
        .into_iter()
        .map(|(k, v)| ast::Afield::AFkvalue(k, v))
        .collect();
    Expr::new(
        pos.clone(),
        Expr_::Collection(Box::new((make_id(pos, "dict"), None, fields))),
    )
}

fn make_id(pos: Pos, name: &str) -> ast::Id {
    ast::Id(pos, name.into())
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

fn visitor_variable() -> String {
    "$0v".to_string()
}

/// Build `$v->meth_name(args)`.
fn v_meth_call(meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    let receiver = Expr::mk_lvar(pos, &visitor_variable());
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

/// Replace all the splices in the expression tree with variables, and
/// return a vec of the splice expressions found.
///
/// ```
/// $c = Code`${$x->foo()} + ${$y};
/// $c_after = Code`$0splice0 + $0splice1`;
/// ```
///
/// Returns an error if users have nested splices.
fn extract_and_replace_splices(e: &mut Expr) -> Result<Vec<Expr>, (Pos, String)> {
    let mut visitor = SpliceExtractor {
        extracted_splices: vec![],
    };
    let mut in_splice = false;
    visitor.visit_expr(&mut in_splice, e)?;
    Ok(visitor.extracted_splices)
}

struct SpliceExtractor {
    extracted_splices: Vec<Expr>,
}

impl<'ast> VisitorMut<'ast> for SpliceExtractor {
    type P = AstParams<bool, (Pos, String)>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, in_splice: &mut bool, e: &mut Expr) -> Result<(), (Pos, String)> {
        use aast::Expr_::*;
        match &mut e.1 {
            ETSplice(ref mut ex) => {
                if *in_splice {
                    return Err((
                        e.0.clone(),
                        "Splice syntax `${...}` cannot be nested.".into(),
                    ));
                } else {
                    // Check for nested splices.
                    *in_splice = true;
                    ex.recurse(in_splice, self.object())?;
                    *in_splice = false;

                    // Extract this expression.
                    let len = self.extracted_splices.len();
                    self.extracted_splices.push((**ex).clone());
                    (*e).1 = ETSplice(Box::new(temp_lvar(&ex.0, len)));
                }
            }
            _ => e.recurse(in_splice, self.object())?,
        }
        Ok(())
    }
}

fn temp_lvar_string(num: usize) -> String {
    format!("$0splice{}", num.to_string())
}

fn temp_lvar(pos: &Pos, num: usize) -> Expr {
    Expr::mk_lvar(pos, &temp_lvar_string(num))
}

/// Given a Pos, returns `new ExprPos(...)`
/// In case of Pos.none or invalid position, all elements set to 0
fn exprpos(pos: &Pos) -> Expr {
    if pos.is_none() || !pos.is_valid() {
        null_literal(pos.clone())
    } else {
        let ((start_lnum, start_bol, start_cnum), (end_lnum, end_bol, end_cnum)) =
            pos.to_start_and_end_lnum_bol_cnum();
        new_obj(
            &pos,
            "\\ExprPos",
            vec![
                Expr::new(
                    pos.clone(),
                    Expr_::Id(Box::new(make_id(pos.clone(), "__FILE__"))),
                ),
                int_literal(pos.clone(), start_lnum),
                int_literal(pos.clone(), start_cnum - start_bol),
                int_literal(pos.clone(), end_lnum),
                int_literal(pos.clone(), end_cnum - end_bol),
            ],
        )
    }
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

fn hint_name(hint: &aast::Hint) -> String {
    let name = {
        if let Hint_::Happly(id, _) = &*hint.1 {
            &id.1
        } else {
            ""
        }
    };
    name.into()
}
