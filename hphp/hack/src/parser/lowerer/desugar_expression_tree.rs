use crate::lowerer::Env;
use bstr::BString;
use naming_special_names_rust::{classes, pseudo_functions};
use oxidized::{
    aast,
    aast_visitor::{visit, AstParams, Node, NodeMut, Visitor, VisitorMut},
    ast,
    ast::{ClassId, ClassId_, Expr, Expr_, Hint_, Sid, Stmt, Stmt_},
    ast_defs::*,
    pos::Pos,
};

/// Rewrite the contents of an expression tree literal into an
/// expression on a visitor class.
///
/// Given the following expression tree:
/// ```
/// MyDsl`foo(1) + ${ $x }`;
/// ```
///
/// First, the splices are extracted and assigned to temporary variables:
/// ```
/// { $0splice0 = $x; }
/// ```
///
/// Then the expression is virtualized as virtualized_expr
/// ```
/// MyDsl::symbolType(foo<>)(MyDsl::intType())->__plus( ${ $0splice0 } )
/// ```
/// Where virtualized_expr is used in helping type Expression Trees
///
/// Finally, the expression is desugared as runtime_expr
/// ```
/// MyDsl::makeTree(
///   // At runtime, expression tree visitors know the position of the literal.
///   shape('path' => 'whatever.php', 'start_line' => 123, ...),
///
///   // We provide metadata of values used inside the visitor, so users can access
///   // spliced or called values without having to re-run the visit method.
///   shape(
///     'splices' => dict['$0splice0' => $0splice0],
///     'functions' => vec[foo<>],
///     'static_methods' => vec[],
///   )
///
///   (MyDsl $0v) ==> {
///     $0v->visitBinop(
///       // (ignoring ExprPos arguments for brevity)
///       $0v->visitCall(
///         $0v->visitGlobalFunction(foo<>),
///         vec[$0v->visitInt(1)],
///       ),
///       '__plus',
///       $0v->splice('$0splice0', $0splice0),
///     )
///   },
/// )
/// ```
/// Which is the runtime representation of the Expression Tree
pub fn desugar<TF>(hint: &aast::Hint, mut e: Expr, env: &Env<TF>) -> Result<Expr, (Pos, String)> {
    let visitor_name = hint_name(hint)?;

    // Extract calls to functions and static methods before any
    // desugaring occurs.
    let (functions, static_methods) = extract_calls_as_fun_ptrs(&e);

    let extracted_splices = extract_and_replace_splices(&mut e)?;
    let splice_count = extracted_splices.len();
    let et_literal_pos = e.0.clone();

    let metadata = maketree_metadata(
        &et_literal_pos,
        &extracted_splices,
        functions,
        static_methods,
    );

    // Create assignments of extracted splices
    // `$0splice0 = spliced_expr0;`
    let splice_assignments: Vec<Stmt> = extracted_splices
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

    let (virtual_expr, desugar_expr) = rewrite_expr(env, e, &visitor_name)?;

    // Make anonymous function of smart constructor calls
    let visitor_expr = wrap_return(desugar_expr, &et_literal_pos);
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
    let visitor_fun_ = wrap_fun_(visitor_body, vec![param], et_literal_pos.clone());
    let visitor_lambda = Expr::new(et_literal_pos.clone(), Expr_::mk_lfun(visitor_fun_, vec![]));

    // Wrap this in an Efun with appropriate variables for typing.
    // This enables us to report unbound variables correctly.
    let virtualized_expr = {
        let typing_fun_body = ast::FuncBody {
            ast: vec![wrap_return(virtual_expr, &et_literal_pos)],
            annotation: (),
        };
        let typing_fun_ = wrap_fun_(typing_fun_body, vec![], et_literal_pos.clone());
        let spliced_vars = (0..splice_count)
            .into_iter()
            .map(|i| ast::Lid(et_literal_pos.clone(), (0, temp_lvar_string(i))))
            .collect();
        Expr::new(
            et_literal_pos.clone(),
            Expr_::Call(Box::new((
                Expr::new(
                    et_literal_pos.clone(),
                    Expr_::mk_efun(typing_fun_, spliced_vars),
                ),
                vec![],
                vec![],
                None,
            ))),
        )
    };

    let make_tree = static_meth_call(
        &visitor_name,
        "makeTree",
        vec![exprpos(&et_literal_pos), metadata, visitor_lambda],
        &et_literal_pos.clone(),
    );

    let runtime_expr = if splice_assignments.is_empty() {
        make_tree
    } else {
        let mut body = splice_assignments.clone();
        body.push(wrap_return(make_tree, &et_literal_pos));
        immediately_invoked_lambda(&et_literal_pos, body)
    };

    Ok(Expr::new(
        et_literal_pos,
        Expr_::mk_expression_tree(ast::ExpressionTree {
            hint: hint.clone(),
            splices: splice_assignments,
            virtualized_expr,
            runtime_expr,
        }),
    ))
}

/// Convert `foo` to `return foo;`.
fn wrap_return(e: Expr, pos: &Pos) -> Stmt {
    Stmt::new(pos.clone(), Stmt_::Return(Box::new(Some(e))))
}

/// Wrap a FuncBody into an anonymous Fun_
fn wrap_fun_(body: ast::FuncBody, params: Vec<ast::FunParam>, pos: Pos) -> ast::Fun_ {
    ast::Fun_ {
        span: pos.clone(),
        readonly_this: None,
        annotation: (),
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
        external: false,
        doc_comment: None,
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
    vec_literal_with_pos(&position, items)
}

fn vec_literal_with_pos(pos: &Pos, items: Vec<Expr>) -> Expr {
    let fields: Vec<_> = items.into_iter().map(|e| ast::Afield::AFvalue(e)).collect();
    Expr::new(
        pos.clone(),
        Expr_::Collection(Box::new((make_id(pos.clone(), "vec"), None, fields))),
    )
}

fn dict_literal(pos: &Pos, key_value_pairs: Vec<(Expr, Expr)>) -> Expr {
    let fields = key_value_pairs
        .into_iter()
        .map(|(k, v)| ast::Afield::AFkvalue(k, v))
        .collect();
    Expr::new(
        pos.clone(),
        Expr_::Collection(Box::new((make_id(pos.clone(), "dict"), None, fields))),
    )
}

fn make_id(pos: Pos, name: &str) -> ast::Id {
    ast::Id(pos, name.into())
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

/// Find all the calls in expression `e` and return a vec of the
/// global function pointers and a vec of static method pointers.
///
/// For example, given the expression tree literal:
///
///     $et = Code`foo() + Bar::baz()`;
///
/// Our first vec contains foo<> and our second contains Bar::baz<>.
fn extract_calls_as_fun_ptrs(e: &Expr) -> (Vec<Expr>, Vec<Expr>) {
    let mut visitor = CallExtractor {
        functions: vec![],
        static_methods: vec![],
    };
    visitor
        .visit_expr(&mut (), e)
        .expect("CallExtractor should not error");

    (visitor.functions, visitor.static_methods)
}

struct CallExtractor {
    functions: Vec<Expr>,
    static_methods: Vec<Expr>,
}

impl<'ast> Visitor<'ast> for CallExtractor {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &Expr) -> Result<(), ()> {
        use aast::Expr_::*;
        match &e.1 {
            // Don't recurse into splices.
            ETSplice(_) => {
                return Ok(());
            }

            Call(call) => {
                let (recv, _, _, _) = &**call;
                match &recv.1 {
                    Id(sid) => {
                        self.functions.push(global_func_ptr(&sid));
                    }
                    ClassConst(cc) => {
                        let (ref cid, ref s) = **cc;
                        self.static_methods.push(static_meth_ptr(&recv.0, cid, s));
                    }
                    _ => {}
                }
            }
            _ => {}
        };
        e.recurse(env, self)
    }
}

fn temp_lvar_string(num: usize) -> String {
    format!("$0splice{}", num.to_string())
}

fn temp_lvar(pos: &Pos, num: usize) -> Expr {
    Expr::mk_lvar(pos, &temp_lvar_string(num))
}

/// Given a Pos, returns a shape literal expression representing it.
///
/// ```
/// shape(
///   'path' => __FILE__,
///   'start_line' => 1,
///   'end_line' => 10,
///   'start_column' => 0,
///   'end_column' => 80,
/// )
/// ```
///
/// If this Pos is Pos.none or invalid, return a literal null instead.
fn exprpos(pos: &Pos) -> Expr {
    if pos.is_none() || !pos.is_valid() {
        null_literal(pos.clone())
    } else {
        let ((start_lnum, start_bol, start_cnum), (end_lnum, end_bol, end_cnum)) =
            pos.to_start_and_end_lnum_bol_cnum();

        let fields = vec![
            (
                "path",
                Expr::new(
                    pos.clone(),
                    Expr_::Id(Box::new(make_id(pos.clone(), "__FILE__"))),
                ),
            ),
            ("start_line", int_literal(pos.clone(), start_lnum)),
            ("end_line", int_literal(pos.clone(), end_lnum)),
            (
                "start_column",
                int_literal(pos.clone(), start_cnum - start_bol),
            ),
            ("end_column", int_literal(pos.clone(), end_cnum - end_bol)),
        ];

        shape_literal(pos, fields)
    }
}

fn shape_literal(pos: &Pos, fields: Vec<(&str, Expr)>) -> Expr {
    let shape_fields: Vec<_> = fields
        .into_iter()
        .map(|(name, value)| {
            let bs = BString::from(name);
            let field_name = ShapeFieldName::SFlitStr((pos.clone(), bs));
            (field_name, value)
        })
        .collect();
    Expr::new(pos.clone(), Expr_::Shape(shape_fields))
}

fn boolify(receiver: Expr) -> Expr {
    let pos = receiver.0.clone();
    meth_call(receiver, "__bool", vec![], &pos)
}

/// Performs both the virtualization and the desugaring in tandem
/// Assumes that splices have been extracted and replaced by temporaries
fn rewrite_expr<TF>(
    env: &Env<TF>,
    e: Expr,
    visitor_name: &str,
) -> Result<(Expr, Expr), (Pos, String)> {
    use aast::Expr_::*;

    let Expr(pos, expr_) = e;
    let pos_expr = exprpos(&pos);
    let (virtual_expr, desugar_expr) = match expr_ {
        // Source: MyDsl`1`
        // Virtualized: MyDsl::intType()
        // Desugared: $0v->visitInt(new ExprPos(...), 1)
        Int(_) => {
            let virtual_expr = static_meth_call(visitor_name, "intType", vec![], &pos);
            let desugar_expr =
                v_meth_call("visitInt", vec![pos_expr, Expr(pos.clone(), expr_)], &pos);
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`1.0`
        // Virtualized: MyDsl::floatType()
        // Desugared: $0v->visitFloat(new ExprPos(...), 1.0)
        Float(_) => {
            let virtual_expr = static_meth_call(visitor_name, "floatType", vec![], &pos);
            let desugar_expr =
                v_meth_call("visitFloat", vec![pos_expr, Expr(pos.clone(), expr_)], &pos);
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`'foo'`
        // Virtualized: MyDsl::stringType()
        // Desugared: $0v->visitString(new ExprPos(...), 'foo')
        String(_) => {
            let virtual_expr = static_meth_call(visitor_name, "stringType", vec![], &pos);
            let desugar_expr = v_meth_call(
                "visitString",
                vec![pos_expr, Expr(pos.clone(), expr_)],
                &pos,
            );
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`true`
        // Virtualized: MyDsl::boolType()
        // Desugared: $0v->visitBool(new ExprPos(...), true)
        True | False => {
            let virtual_expr = static_meth_call(visitor_name, "boolType", vec![], &pos);
            let desugar_expr =
                v_meth_call("visitBool", vec![pos_expr, Expr(pos.clone(), expr_)], &pos);
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`null`
        // Virtualized: MyDsl::nullType()
        // Desugared: $0v->visitNull(new ExprPos(...))
        Null => {
            let virtual_expr = static_meth_call(visitor_name, "nullType", vec![], &pos);
            let desugar_expr = v_meth_call("visitNull", vec![pos_expr], &pos);
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`$x`
        // Virtualized: $x
        // Desugared: $0v->visitLocal(new ExprPos(...), '$x')
        Lvar(lid) => {
            let desugar_expr = v_meth_call(
                "visitLocal",
                vec![pos_expr, string_literal(lid.0.clone(), &((lid.1).1))],
                &pos,
            );
            let virtual_expr = Expr(pos, Lvar(lid));
            (virtual_expr, desugar_expr)
        }
        Binop(bop) => {
            let (op, lhs, rhs) = *bop;
            let (virtual_lhs, desugar_lhs) = rewrite_expr(env, lhs, visitor_name)?;
            let (virtual_rhs, desugar_rhs) = rewrite_expr(env, rhs, visitor_name)?;
            if op == Bop::Eq(None) {
                // Source: MyDsl`$x = ...`
                // Virtualized: $x = ...
                // Desugared: $0v->visitAssign(new ExprPos(...), $0v->visitLocal(...), ...)
                let desugar_expr = v_meth_call(
                    "visitAssign",
                    vec![pos_expr, desugar_lhs, desugar_rhs],
                    &pos,
                );
                let virtual_expr = Expr(pos, Binop(Box::new((op, virtual_lhs, virtual_rhs))));
                (virtual_expr, desugar_expr)
            } else {
                // Source: MyDsl`... + ...`
                // Virtualized: ...->__plus(...)
                // Desugared: $0v->visitBinop(new ExprPos(...), ..., '__plus', ...)
                let binop_str = match op {
                    Bop::Plus => "__plus",
                    Bop::Minus => "__minus",
                    Bop::Star => "__star",
                    Bop::Slash => "__slash",
                    Bop::Percent => "__percent",
                    // Convert boolean &&, ||
                    Bop::Ampamp => "__ampamp",
                    Bop::Barbar => "__barbar",
                    // Convert comparison operators, <, <=, >, >=, ===, !==
                    Bop::Lt => "__lessThan",
                    Bop::Lte => "__lessThanEqual",
                    Bop::Gt => "__greaterThan",
                    Bop::Gte => "__greaterThanEqual",
                    Bop::Eqeqeq => "__tripleEquals",
                    Bop::Diff2 => "__notTripleEquals",
                    // Convert string concatenation
                    Bop::Dot => "__dot",
                    // Convert bitwise operators, &, |, ^, <<, >>
                    Bop::Amp => "__amp",
                    Bop::Bar => "__bar",
                    Bop::Xor => "__caret",
                    Bop::Ltlt => "__lessThanLessThan",
                    Bop::Gtgt => "__greaterThanGreaterThan",
                    // Explicit list of unsupported operators and error messages
                    Bop::Starstar => {
                        return Err((
                            pos,
                            "Expression trees do not support the exponent operator `**`.".into(),
                        ));
                    }
                    Bop::Eqeq | Bop::Diff => {
                        return Err((
                            pos,
                            "Expression trees only support strict equality operators `===` and `!==`".into(),
                        ));
                    }
                    Bop::Cmp => {
                        return Err((
                            pos,
                            "Expression trees do not support the spaceship operator `<=>`. Try comparison operators like `<` and `>=`".into(),
                        ));
                    }
                    Bop::QuestionQuestion => {
                        return Err((
                            pos,
                            "Expression trees do not support the null coalesce operator `??`."
                                .into(),
                        ));
                    }
                    Bop::Eq(_) => {
                        return Err((
                            pos,
                            "Expression trees do not support compound assignments. Try the long form style `$foo = $foo + $bar` instead.".into(),
                        ));
                    }
                };
                let virtual_expr = meth_call(virtual_lhs, &binop_str, vec![virtual_rhs], &pos);
                let desugar_expr = v_meth_call(
                    "visitBinop",
                    vec![
                        pos_expr,
                        desugar_lhs,
                        string_literal(pos.clone(), &binop_str),
                        desugar_rhs,
                    ],
                    &pos,
                );
                (virtual_expr, desugar_expr)
            }
        }
        // Source: MyDsl`!...`
        // Virtualized: ...->__exclamationMark(...)
        // Desugared: $0v->visitUnop(new ExprPos(...), ..., '__exclamationMark')
        Unop(unop) => {
            let (op, operand) = *unop;
            let (virtual_operand, desugar_operand) = rewrite_expr(env, operand, visitor_name)?;
            let op_str = match op {
                // Allow boolean not operator !$x
                Uop::Unot => "__exclamationMark",
                // Allow negation -$x (required for supporting negative literals -123)
                Uop::Uminus => "__negate",
                // Allow bitwise complement
                Uop::Utild => "__tilde",
                // Currently not allowed operators
                Uop::Uplus => {
                    return Err((
                        pos,
                        "Expression trees do not support the unary plus operator.".into(),
                    ));
                }
                Uop::Uincr | Uop::Upincr => {
                    return Err((
                        pos,
                        "Expression trees do not support the increment operator `++`.".into(),
                    ));
                }
                Uop::Udecr | Uop::Updecr => {
                    return Err((
                        pos,
                        "Expression trees do not support the decrement operator `--`.".into(),
                    ));
                }
                Uop::Usilence => {
                    return Err((
                        pos,
                        "Expression trees do not support the error suppression operator `@`."
                            .into(),
                    ));
                }
            };
            let virtual_expr = meth_call(virtual_operand, &op_str, vec![], &pos);
            let desugar_expr = v_meth_call(
                "visitUnop",
                vec![
                    pos_expr,
                    desugar_operand,
                    string_literal(pos.clone(), &op_str),
                ],
                &pos,
            );
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`... ? ... : ...`
        // Virtualized: ...->__bool() ? ... : ...
        // Desugared: $0v->visitTernary(new ExprPos(...), ..., ..., ...)
        Eif(eif) => {
            let (e1, e2o, e3) = *eif;
            let (virtual_e1, desugar_e1) = rewrite_expr(env, e1, visitor_name)?;
            let (virtual_e2, desugar_e2) = if let Some(e2) = e2o {
                rewrite_expr(env, e2, visitor_name)?
            } else {
                return Err((
                    pos,
                    "Unsupport expression tree syntax: Elvis operator".into(),
                ));
            };
            let (virtual_e3, desugar_e3) = rewrite_expr(env, e3, visitor_name)?;

            let desugar_expr = v_meth_call(
                "visitTernary",
                vec![pos_expr, desugar_e1, desugar_e2, desugar_e3],
                &pos,
            );
            let virtual_expr = Expr(
                pos,
                Eif(Box::new((
                    boolify(virtual_e1),
                    Some(virtual_e2),
                    virtual_e3,
                ))),
            );
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`...()`
        // Virtualized: ...()
        // Desugared: $0v->visitCall(new ExprPos(...), ..., vec[])
        Call(call) => {
            let (recv, targs, args, variadic) = *call;
            if variadic.is_some() {
                return Err((
                    pos,
                    "Expression trees do not support variadic calls.".into(),
                ));
            }
            if !targs.is_empty() {
                return Err((
                    pos,
                    "Expression trees do not support function calls with generics.".into(),
                ));
            }
            match &recv.1 {
                // Don't transform calls to `hh_show`.
                Id(sid) if is_typechecker_fun_name(&sid.1) => {
                    let call_e =
                        Expr::new(pos.clone(), Call(Box::new((recv, targs, args, variadic))));
                    return Ok((call_e.clone(), call_e));
                }
                _ => {}
            }

            let (virtual_args, desugar_args) = rewrite_exprs(env, args, visitor_name)?;

            match recv.1 {
                // Source: MyDsl`foo()`
                // Virtualized: MyDsl::symbolType(foo<>)()
                // Desugared: $0v->visitCall(new ExprPos(...), $0v->visitGlobalFunction(new ExprPos(...), foo<>), vec[])
                Id(sid) => {
                    let fp = global_func_ptr(&sid);
                    let callee =
                        static_meth_call(visitor_name, "symbolType", vec![fp.clone()], &pos);

                    let desugar_recv =
                        v_meth_call("visitGlobalFunction", vec![pos_expr.clone(), fp], &pos);
                    let desugar_expr = v_meth_call(
                        "visitCall",
                        vec![pos_expr, desugar_recv, vec_literal(desugar_args)],
                        &pos,
                    );
                    let virtual_expr =
                        Expr(pos, Call(Box::new((callee, vec![], virtual_args, None))));
                    (virtual_expr, desugar_expr)
                }
                // Source: MyDsl`Foo::bar()`
                // Virtualized: MyDsl::symbolType(Foo::bar<>)()
                // Desugared: $0v->visitCall(new ExprPos(...), $0v->visitStaticMethod(new ExprPos(...), Foo::bar<>), vec[])
                ClassConst(cc) => {
                    let (cid, s) = *cc;
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

                    let fp = static_meth_ptr(&pos, &cid, &s);

                    let callee =
                        static_meth_call(visitor_name, "symbolType", vec![fp.clone()], &pos);

                    let desugar_recv =
                        v_meth_call("visitStaticMethod", vec![pos_expr.clone(), fp], &pos);
                    let desugar_expr = v_meth_call(
                        "visitCall",
                        vec![pos_expr, desugar_recv, vec_literal(desugar_args)],
                        &pos,
                    );
                    let virtual_expr =
                        Expr(pos, Call(Box::new((callee, vec![], virtual_args, None))));
                    (virtual_expr, desugar_expr)
                }
                _ => {
                    let (virtual_recv, desugar_recv) =
                        rewrite_expr(env, Expr(recv.0, recv.1), visitor_name)?;
                    let desugar_expr = v_meth_call(
                        "visitCall",
                        vec![pos_expr, desugar_recv, vec_literal(desugar_args)],
                        &pos,
                    );
                    let virtual_expr = Expr(
                        pos,
                        Call(Box::new((virtual_recv, vec![], virtual_args, None))),
                    );
                    (virtual_expr, desugar_expr)
                }
            }
        }
        // Source: MyDsl`($x) ==> { ... }`
        // Virtualized: ($x) ==> { ...; return MyDsl::voidType(); }
        //   if no `return expr;` statements.
        // Desugared: $0v->visitLambda(new ExprPos(...), vec['$x'], vec[...]).
        Lfun(lf) => {
            let mut fun_ = lf.0;

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

            let body = std::mem::take(&mut fun_.body.ast);

            let should_append_return = only_void_return(&body);

            let (mut virtual_body_stmts, desugar_body) = rewrite_stmts(env, body, visitor_name)?;

            if should_append_return {
                virtual_body_stmts.push(Stmt(
                    pos.clone(),
                    aast::Stmt_::Return(Box::new(Some(static_meth_call(
                        visitor_name,
                        "voidType",
                        vec![],
                        &pos,
                    )))),
                ));
            }

            let desugar_expr = v_meth_call(
                "visitLambda",
                vec![
                    pos_expr,
                    vec_literal(param_names),
                    vec_literal(desugar_body),
                ],
                &pos,
            );
            fun_.body.ast = virtual_body_stmts;
            let virtual_expr = Expr(pos, Lfun(Box::new((fun_, vec![]))));
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`${ ... }`
        // Virtualized to `${ ... }`
        // Desugared to `$0v->splice(new ExprPos(...), '$var_name', ...)`
        ETSplice(e) => {
            // Assumes extract and replace has already occurred
            let s = if let Lvar(lid) = &e.1 {
                let aast::Lid(p, (_, lid)) = &**lid;
                string_literal(p.clone(), lid)
            } else {
                return Err((pos, "Please report a bug".into()));
            };
            let desugar_expr = v_meth_call("splice", vec![pos_expr, s, *e.clone()], &pos);
            let virtual_expr = Expr(pos, ETSplice(e));
            (virtual_expr, desugar_expr)
        }
        ExpressionTree(_) => {
            return Err((pos, "Expression trees may not be nested".into()));
        }
        _ => {
            return Err((pos, "Unsupported expression tree syntax.".into()));
        }
    };
    Ok((virtual_expr, desugar_expr))
}

fn rewrite_exprs<TF>(
    env: &Env<TF>,
    exprs: Vec<Expr>,
    visitor_name: &str,
) -> Result<(Vec<Expr>, Vec<Expr>), (Pos, String)> {
    let mut virtual_results = Vec::with_capacity(exprs.len());
    let mut desugar_results = Vec::with_capacity(exprs.len());
    for expr in exprs {
        let (virtual_expr, desugar_expr) = rewrite_expr(env, expr, visitor_name)?;
        virtual_results.push(virtual_expr);
        desugar_results.push(desugar_expr);
    }
    Ok((virtual_results, desugar_results))
}

fn rewrite_stmts<TF>(
    env: &Env<TF>,
    stmts: Vec<Stmt>,
    visitor_name: &str,
) -> Result<(Vec<Stmt>, Vec<Expr>), (Pos, String)> {
    let mut virtual_results = Vec::with_capacity(stmts.len());
    let mut desugar_results = Vec::with_capacity(stmts.len());
    for stmt in stmts {
        let (virtual_stmt, desugared_expr) = rewrite_stmt(env, stmt, visitor_name)?;
        virtual_results.push(virtual_stmt);
        if let Some(desugared_expr) = desugared_expr {
            desugar_results.push(desugared_expr);
        }
    }
    Ok((virtual_results, desugar_results))
}

fn rewrite_stmt<TF>(
    env: &Env<TF>,
    s: Stmt,
    visitor_name: &str,
) -> Result<(Stmt, Option<Expr>), (Pos, String)> {
    use aast::Stmt_::*;

    let Stmt(pos, stmt_) = s;
    let pos_expr = exprpos(&pos);

    let virtual_desugar = match stmt_ {
        Expr(e) => {
            let (virtual_expr, desugar_expr) = rewrite_expr(env, *e, visitor_name)?;
            (Stmt(pos, Expr(Box::new(virtual_expr))), Some(desugar_expr))
        }
        Return(e) => match *e {
            // Source: MyDsl`return ...;`
            // Virtualized: return ...;
            // Desugared: $0v->visitReturn(new ExprPos(...), $0v->...)
            Some(e) => {
                let (virtual_expr, desugar_expr) = rewrite_expr(env, e, visitor_name)?;
                let desugar_expr = v_meth_call("visitReturn", vec![pos_expr, desugar_expr], &pos);
                let virtual_stmt = Stmt(pos, Return(Box::new(Some(virtual_expr))));
                (virtual_stmt, Some(desugar_expr))
            }
            // Source: MyDsl`return;`
            // Virtualized: return MyDsl::voidType();
            // Desugared: $0v->visitReturn(new ExprPos(...), null)
            None => {
                let desugar_expr = v_meth_call(
                    "visitReturn",
                    vec![pos_expr, null_literal(pos.clone())],
                    &pos,
                );

                let virtual_void_expr = static_meth_call(visitor_name, "voidType", vec![], &pos);
                let virtual_stmt = Stmt(pos, Return(Box::new(Some(virtual_void_expr))));
                (virtual_stmt, Some(desugar_expr))
            }
        },
        // Source: MyDsl`if (...) {...} else {...}`
        // Virtualized: if (...->__bool())) {...} else {...}
        // Desugared: $0v->visitIf(new ExprPos(...), $0v->..., vec[...], vec[...])
        If(if_stmt) => {
            let (cond_expr, then_block, else_block) = *if_stmt;
            let (virtual_cond, desugar_cond) = rewrite_expr(env, cond_expr, visitor_name)?;
            let (virtual_then_stmts, desugar_then) = rewrite_stmts(env, then_block, visitor_name)?;
            let (virtual_else_stmts, desugar_else) = rewrite_stmts(env, else_block, visitor_name)?;

            let desugar_expr = v_meth_call(
                "visitIf",
                vec![
                    pos_expr,
                    desugar_cond,
                    vec_literal(desugar_then),
                    vec_literal(desugar_else),
                ],
                &pos,
            );
            let virtual_stmt = Stmt(
                pos,
                If(Box::new((
                    boolify(virtual_cond),
                    virtual_then_stmts,
                    virtual_else_stmts,
                ))),
            );
            (virtual_stmt, Some(desugar_expr))
        }
        // Source: MyDsl`while (...) {...}`
        // Virtualized: while (...->__bool()) {...}
        // Desugared: $0v->visitWhile(new ExprPos(...), $0v->..., vec[...])
        While(w) => {
            let (cond, body) = *w;
            let (virtual_cond, desugar_cond) = rewrite_expr(env, cond, visitor_name)?;
            let (virtual_body_stmts, desugar_body) = rewrite_stmts(env, body, visitor_name)?;

            let desugar_expr = v_meth_call(
                "visitWhile",
                vec![pos_expr, desugar_cond, vec_literal(desugar_body)],
                &pos,
            );
            let virtual_stmt = Stmt(
                pos,
                While(Box::new((boolify(virtual_cond), virtual_body_stmts))),
            );
            (virtual_stmt, Some(desugar_expr))
        }
        // Source: MyDsl`for (...; ...; ...) {...}`
        // Virtualized: for (...; ...->__bool(); ...) {...}
        // Desugared: $0v->visitFor(new ExprPos(...), vec[...], ..., vec[...], vec[...])
        For(w) => {
            let (init, cond, incr, body) = *w;
            let (virtual_init_exprs, desugar_init_exprs) = rewrite_exprs(env, init, visitor_name)?;
            let (virtual_cond_option, desugar_cond_expr) = match cond {
                Some(cond) => {
                    let (virtual_cond, desugar_cond) = rewrite_expr(env, cond, visitor_name)?;
                    (Some(boolify(virtual_cond)), desugar_cond)
                }
                None => (None, null_literal(pos.clone())),
            };
            let (virtual_incr_exprs, desugar_incr_exprs) = rewrite_exprs(env, incr, visitor_name)?;
            let (virtual_body_stmts, desugar_body) = rewrite_stmts(env, body, visitor_name)?;

            let desugar_expr = v_meth_call(
                "visitFor",
                vec![
                    pos_expr,
                    vec_literal(desugar_init_exprs),
                    desugar_cond_expr,
                    vec_literal(desugar_incr_exprs),
                    vec_literal(desugar_body),
                ],
                &pos,
            );
            let virtual_stmt = Stmt(
                pos,
                For(Box::new((
                    virtual_init_exprs,
                    virtual_cond_option,
                    virtual_incr_exprs,
                    virtual_body_stmts,
                ))),
            );
            (virtual_stmt, Some(desugar_expr))
        }
        // Source: MyDsl`break;`
        // Virtualized: break;
        // Desugared: $0v->visitBreak(new ExprPos(...))
        Break => {
            let desugar_expr = v_meth_call("visitBreak", vec![pos_expr], &pos);
            let virtual_stmt = Stmt(pos, Break);
            (virtual_stmt, Some(desugar_expr))
        }
        // Source: MyDsl`continue;`
        // Virtualized: continue;
        // Desugared: $0v->visitContinue(new ExprPos(...))
        Continue => {
            let desugar_expr = v_meth_call("visitContinue", vec![pos_expr], &pos);
            let virtual_stmt = Stmt(pos, Continue);
            (virtual_stmt, Some(desugar_expr))
        }
        Noop => (Stmt(pos, Noop), None),
        _ => {
            return Err((
                pos,
                "Expression trees do not support this statement syntax.".into(),
            ));
        }
    };
    Ok(virtual_desugar)
}

fn hint_name(hint: &aast::Hint) -> Result<String, (Pos, String)> {
    if let Hint_::Happly(id, _) = &*hint.1 {
        Ok(id.1.clone())
    } else {
        Err((
            hint.0.clone(),
            "Could not determine the visitor type for this Expression Tree".into(),
        ))
    }
}

fn immediately_invoked_lambda(pos: &Pos, stmts: Vec<Stmt>) -> Expr {
    let func_body = ast::FuncBody {
        ast: stmts,
        annotation: (),
    };
    let fun_ = wrap_fun_(func_body, vec![], pos.clone());
    let lambda_expr = Expr::new(pos.clone(), Expr_::mk_lfun(fun_, vec![]));

    Expr::new(
        pos.clone(),
        Expr_::Call(Box::new((lambda_expr, vec![], vec![], None))),
    )
}

/// Is this is a typechecker pseudo function like `hh_show` that
/// shouldn't be desugared?
fn is_typechecker_fun_name(name: &str) -> bool {
    strip_ns(name) == strip_ns(pseudo_functions::HH_SHOW)
        || strip_ns(name) == strip_ns(pseudo_functions::HH_SHOW_ENV)
}

fn strip_ns(name: &str) -> &str {
    match name.chars().next() {
        Some('\\') => &name[1..],
        _ => name,
    }
}

/// Return a shape literal that describes the values inside this
/// expression tree literal. For example, given the expression tree:
///
///     $et = Code`${ $x } + foo() + Bar::baz()`;
///
/// The metadata is:
///
///     shape(
///       // Simplified: We actually use a temporary variable whose value is $x.
///       'splices' => dict['$0splice0' => $x],
///
///       'functions' => vec[foo<>],
///       'static_methods' => vec[Bar::baz<>],
///     )
fn maketree_metadata(
    pos: &Pos,
    splices: &[Expr],
    functions: Vec<Expr>,
    static_methods: Vec<Expr>,
) -> Expr {
    let key_value_pairs = splices
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
    let splices_dict = dict_literal(pos, key_value_pairs);

    shape_literal(
        pos,
        vec![
            ("splices", splices_dict),
            ("functions", vec_literal_with_pos(&pos, functions)),
            ("static_methods", vec_literal_with_pos(&pos, static_methods)),
        ],
    )
}

fn global_func_ptr(sid: &Sid) -> Expr {
    let pos = sid.0.clone();
    Expr::new(
        pos.clone(),
        Expr_::FunctionPointer(Box::new((ast::FunctionPtrId::FPId(sid.clone()), vec![]))),
    )
}

fn static_meth_ptr(pos: &Pos, cid: &ClassId, meth: &Pstring) -> Expr {
    Expr::new(
        pos.clone(),
        Expr_::FunctionPointer(Box::new((
            aast::FunctionPtrId::FPClassConst(cid.clone(), meth.clone()),
            vec![],
        ))),
    )
}
