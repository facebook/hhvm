use crate::lowerer::Env;
use bstr::BString;
use naming_special_names_rust::{
    classes, expression_trees as et, pseudo_functions, special_idents,
};
use oxidized::{
    aast,
    aast_visitor::{visit, visit_mut, AstParams, Node, NodeMut, Visitor, VisitorMut},
    ast,
    ast::{ClassId, ClassId_, Expr, Expr_, Hint_, Sid, Stmt, Stmt_},
    ast_defs,
    ast_defs::*,
    local_id,
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
pub fn desugar<TF>(hint: &aast::Hint, e: Expr, env: &Env<'_, TF>) -> Result<Expr, (Pos, String)> {
    let visitor_name = hint_name(hint)?;
    let et_literal_pos = e.1.clone();

    let mut temps = Temporaries {
        splices: vec![],
        global_function_pointers: vec![],
        static_method_pointers: vec![],
    };
    let (virtual_expr, desugar_expr) = rewrite_expr(&mut temps, e, &visitor_name)?;

    let dollardollar_pos = rewrite_dollardollars(&mut temps.splices);

    let splice_count = temps.splices.len();
    let function_count = temps.global_function_pointers.len();
    let static_method_count = temps.static_method_pointers.len();

    let metadata = maketree_metadata(
        &et_literal_pos,
        &temps.splices,
        &temps.global_function_pointers,
        &temps.static_method_pointers,
    );

    // Make anonymous function of smart constructor calls
    let visitor_expr = wrap_return(desugar_expr, &et_literal_pos);
    let visitor_body = ast::FuncBody {
        fb_ast: vec![visitor_expr],
    };
    let param = ast::FunParam {
        annotation: (),
        type_hint: ast::TypeHint((), Some(hint.clone())),
        is_variadic: false,
        pos: hint.0.clone(),
        name: visitor_variable(),
        expr: None,
        callconv: ParamKind::Pnormal,
        readonly: None,
        user_attributes: vec![],
        visibility: None,
    };
    let visitor_fun_ = wrap_fun_(visitor_body, vec![param], et_literal_pos.clone());
    let visitor_lambda = Expr::new(
        (),
        et_literal_pos.clone(),
        Expr_::mk_lfun(visitor_fun_, vec![]),
    );

    // Wrap this in an Efun with appropriate variables for typing.
    // This enables us to report unbound variables correctly.
    let virtualized_expr = {
        let typing_fun_body = ast::FuncBody {
            fb_ast: vec![wrap_return(virtual_expr, &et_literal_pos)],
        };
        let typing_fun_ = wrap_fun_(typing_fun_body, vec![], et_literal_pos.clone());
        let mut spliced_vars: Vec<ast::Lid> = (0..splice_count)
            .into_iter()
            .map(|i| ast::Lid(et_literal_pos.clone(), (0, temp_splice_lvar_string(i))))
            .collect();
        let function_pointer_vars: Vec<ast::Lid> = (0..function_count)
            .into_iter()
            .map(|i| {
                ast::Lid(
                    et_literal_pos.clone(),
                    (0, temp_function_pointer_lvar_string(i)),
                )
            })
            .collect();
        let static_method_vars: Vec<ast::Lid> = (0..static_method_count)
            .into_iter()
            .map(|i| {
                ast::Lid(
                    et_literal_pos.clone(),
                    (0, temp_static_method_lvar_string(i)),
                )
            })
            .collect();
        spliced_vars.extend(function_pointer_vars);
        spliced_vars.extend(static_method_vars);
        Expr::new(
            (),
            et_literal_pos.clone(),
            Expr_::Call(Box::new((
                Expr::new(
                    (),
                    et_literal_pos.clone(),
                    Expr_::mk_efun(typing_fun_, spliced_vars),
                ),
                vec![],
                vec![],
                None,
            ))),
        )
    };

    // Create assignment of the extracted expressions to temporary variables
    // `$0splice0 = spliced_expr0;`
    let splice_assignments: Vec<Stmt> = create_temp_statements(temps.splices, temp_splice_lvar);
    // `$0fp0 = foo<>;`
    let function_pointer_assignments: Vec<Stmt> =
        create_temp_statements(temps.global_function_pointers, temp_function_pointer_lvar);
    // `$0sm0 = Foo::bar<>;`
    let static_method_assignments: Vec<Stmt> =
        create_temp_statements(temps.static_method_pointers, temp_static_method_lvar);

    let mut function_pointers = vec![];
    function_pointers.extend(function_pointer_assignments);
    function_pointers.extend(static_method_assignments);

    let make_tree = static_meth_call(
        &visitor_name,
        et::MAKE_TREE,
        vec![exprpos(&et_literal_pos), metadata, visitor_lambda],
        &et_literal_pos.clone(),
    );

    let runtime_expr = if splice_assignments.is_empty() && function_pointers.is_empty() {
        make_tree
    } else {
        let body = if env.codegen {
            let mut b = splice_assignments.clone();
            b.extend(function_pointers.clone());
            b.push(wrap_return(make_tree, &et_literal_pos));
            b
        } else {
            vec![wrap_return(make_tree, &et_literal_pos)]
        };

        let lambda_args = match &dollardollar_pos {
            Some(pipe_pos) => vec![(
                (et::DOLLARDOLLAR_TMP_VAR.to_string(), pipe_pos.clone()),
                Expr::mk_lvar(pipe_pos, special_idents::DOLLAR_DOLLAR),
            )],
            _ => vec![],
        };

        immediately_invoked_lambda(&et_literal_pos, body, lambda_args)
    };

    Ok(Expr::new(
        (),
        et_literal_pos,
        Expr_::mk_expression_tree(ast::ExpressionTree {
            hint: hint.clone(),
            splices: splice_assignments,
            function_pointers,
            virtualized_expr,
            runtime_expr,
            dollardollar_pos,
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

struct DollarDollarRewriter {
    pos: Option<Pos>,
}

impl<'ast> VisitorMut<'ast> for DollarDollarRewriter {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &mut aast::Expr<(), ()>) -> Result<(), ()> {
        use aast::Expr_::*;

        match &mut e.2 {
            // Rewrite all occurrences to $0dollardollar
            Lvar(l) => {
                if local_id::get_name(&l.1) == special_idents::DOLLAR_DOLLAR {
                    // Replace and remember the position
                    e.2 = Lvar(Box::new(ast::Lid(
                        e.1.clone(),
                        local_id::make_unscoped(et::DOLLARDOLLAR_TMP_VAR),
                    )));
                    if self.pos.is_none() {
                        self.pos = Some(e.1.clone());
                    }
                }
                Ok(())
            }
            // Don't need to recurse into the new scopes of lambdas
            Lfun(_) | Efun(_) => Ok(()),
            // Don't recurse into Expression Trees
            ExpressionTree(_) | ETSplice(_) => Ok(()),
            // Only recurse into the left hand side of any pipe as the rhs has new $$
            Pipe(p) => (&mut p.1).accept(env, self.object()),
            // Otherwise, recurse completely on the other expressions
            _ => e.recurse(env, self.object()),
        }
    }
}

fn rewrite_dollardollars(el: &mut Vec<ast::Expr>) -> Option<Pos> {
    let mut rewriter = DollarDollarRewriter { pos: None };
    for e in el.into_iter() {
        visit_mut(&mut rewriter, &mut (), e).expect("DollarDollarRewriter never errors");
    }
    rewriter.pos
}

struct VoidReturnCheck {
    only_void_return: bool,
}

impl<'ast> Visitor<'ast> for VoidReturnCheck {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &aast::Expr<(), ()>) -> Result<(), ()> {
        use aast::Expr_::*;

        match &e.2 {
            // Don't recurse into splices or LFuns
            ETSplice(_) | Lfun(_) => Ok(()),
            // TODO: Do we even recurse on expressions?
            _ => e.recurse(env, self),
        }
    }

    fn visit_stmt(&mut self, env: &mut (), s: &'ast aast::Stmt<(), ()>) -> Result<(), ()> {
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

struct NestedSpliceCheck {
    has_nested_splice: Option<Pos>,
}

impl<'ast> Visitor<'ast> for NestedSpliceCheck {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &aast::Expr<(), ()>) -> Result<(), ()> {
        use aast::Expr_::*;

        match &e.2 {
            ETSplice(_) => {
                self.has_nested_splice = Some(e.1.clone());
            }
            _ if self.has_nested_splice.is_none() => e.recurse(env, self)?,
            _ => {}
        }
        Ok(())
    }
}

/// Assumes that the Expr is the expression within a splice.
/// If the expression has a splice contained within, then we have
/// nested splices and this will raise an error
fn check_nested_splice(e: &ast::Expr) -> Result<(), (Pos, String)> {
    let mut checker = NestedSpliceCheck {
        has_nested_splice: None,
    };
    visit(&mut checker, &mut (), e).unwrap();
    if let Some(p) = checker.has_nested_splice {
        return Err((p, "Splice syntax `${...}` cannot be nested.".into()));
    }
    Ok(())
}

fn null_literal(pos: Pos) -> Expr {
    Expr::new((), pos, Expr_::Null)
}

fn string_literal(pos: Pos, s: &str) -> Expr {
    Expr::new((), pos, Expr_::String(BString::from(s)))
}

fn int_literal(pos: Pos, i: usize) -> Expr {
    Expr::new((), pos, Expr_::Int(i.to_string()))
}

fn vec_literal(items: Vec<Expr>) -> Expr {
    let positions: Vec<_> = items.iter().map(|x| &x.1).collect();
    let position = merge_positions(&positions);
    vec_literal_with_pos(&position, items)
}

fn vec_literal_with_pos(pos: &Pos, items: Vec<Expr>) -> Expr {
    let fields: Vec<_> = items.into_iter().map(|e| ast::Afield::AFvalue(e)).collect();
    Expr::new(
        (),
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
        (),
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

/// Given a list of arguments, make each a "normal" argument by annotating it with
/// `ParamKind::Pnormal`
fn build_args(args: Vec<Expr>) -> Vec<(ParamKind, Expr)> {
    args.into_iter().map(|n| (ParamKind::Pnormal, n)).collect()
}

/// Build `$v->meth_name(args)`.
fn v_meth_call(meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    let receiver = Expr::mk_lvar(pos, &visitor_variable());
    let meth = Expr::new(
        (),
        pos.clone(),
        Expr_::Id(Box::new(ast::Id(pos.clone(), meth_name.into()))),
    );

    let c = Expr_::Call(Box::new((
        Expr::new(
            (),
            pos.clone(),
            Expr_::ObjGet(Box::new((
                receiver,
                meth,
                OgNullFlavor::OGNullthrows,
                ast::PropOrMethod::IsMethod,
            ))),
        ),
        vec![],
        build_args(args),
        None,
    )));
    Expr::new((), pos.clone(), c)
}

fn meth_call(receiver: Expr, meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    let meth = Expr::new(
        (),
        pos.clone(),
        Expr_::Id(Box::new(ast::Id(pos.clone(), meth_name.into()))),
    );

    let c = Expr_::Call(Box::new((
        Expr::new(
            (),
            pos.clone(),
            Expr_::ObjGet(Box::new((
                receiver,
                meth,
                OgNullFlavor::OGNullthrows,
                ast::PropOrMethod::IsMethod,
            ))),
        ),
        vec![],
        build_args(args),
        None,
    )));
    Expr::new((), pos.clone(), c)
}

fn static_meth_call(classname: &str, meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    let callee = Expr::new(
        (),
        pos.clone(),
        Expr_::ClassConst(Box::new((
            // TODO: Refactor ClassId creation with new_obj
            ClassId(
                (),
                pos.clone(),
                ClassId_::CIexpr(Expr::new(
                    (),
                    pos.clone(),
                    Expr_::Id(Box::new(Id(pos.clone(), classname.to_string()))),
                )),
            ),
            (pos.clone(), meth_name.to_string()),
        ))),
    );
    Expr::new(
        (),
        pos.clone(),
        Expr_::Call(Box::new((callee, vec![], build_args(args), None))),
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

fn create_temp_statements(exprs: Vec<Expr>, mk_lvar: fn(&Pos, usize) -> Expr) -> Vec<Stmt> {
    exprs
        .into_iter()
        .enumerate()
        .map(|(i, expr)| {
            Stmt::new(
                expr.1.clone(),
                Stmt_::Expr(Box::new(Expr::new(
                    (),
                    expr.1.clone(),
                    Expr_::Binop(Box::new((Bop::Eq(None), mk_lvar(&expr.1, i), expr))),
                ))),
            )
        })
        .collect()
}

fn temp_lvar_string(name: &str, num: usize) -> String {
    format!("$0{}{}", name, num)
}

fn temp_splice_lvar_string(num: usize) -> String {
    temp_lvar_string("splice", num)
}

fn temp_splice_lvar(pos: &Pos, num: usize) -> Expr {
    Expr::mk_lvar(pos, &temp_splice_lvar_string(num))
}

fn temp_function_pointer_lvar_string(num: usize) -> String {
    temp_lvar_string("fp", num)
}

fn temp_function_pointer_lvar(pos: &Pos, num: usize) -> Expr {
    Expr::mk_lvar(pos, &temp_function_pointer_lvar_string(num))
}

fn temp_static_method_lvar_string(num: usize) -> String {
    temp_lvar_string("sm", num)
}

fn temp_static_method_lvar(pos: &Pos, num: usize) -> Expr {
    Expr::mk_lvar(pos, &temp_static_method_lvar_string(num))
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
                    (),
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
    Expr::new((), pos.clone(), Expr_::Shape(shape_fields))
}

fn boolify(receiver: Expr) -> Expr {
    let pos = receiver.1.clone();
    meth_call(receiver, "__bool", vec![], &pos)
}

struct Temporaries {
    splices: Vec<Expr>,
    global_function_pointers: Vec<Expr>,
    static_method_pointers: Vec<Expr>,
}

/// Performs both the virtualization and the desugaring in tandem
/// Also extracts the expressions that need to be assigned to temporaries
/// Replaces the extracted splices, function pointers, and static method pointers
/// with temporary variables
fn rewrite_expr(
    temps: &mut Temporaries,
    e: Expr,
    visitor_name: &str,
) -> Result<(Expr, Expr), (Pos, String)> {
    use aast::Expr_::*;

    let Expr(_, pos, expr_) = e;
    let pos_expr = exprpos(&pos);
    let (virtual_expr, desugar_expr) = match expr_ {
        // Source: MyDsl`1`
        // Virtualized: MyDsl::intType()
        // Desugared: $0v->visitInt(new ExprPos(...), 1)
        Int(_) => {
            let virtual_expr = static_meth_call(visitor_name, et::INT_TYPE, vec![], &pos);
            let desugar_expr = v_meth_call(
                et::VISIT_INT,
                vec![pos_expr, Expr((), pos.clone(), expr_)],
                &pos,
            );
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`1.0`
        // Virtualized: MyDsl::floatType()
        // Desugared: $0v->visitFloat(new ExprPos(...), 1.0)
        Float(_) => {
            let virtual_expr = static_meth_call(visitor_name, et::FLOAT_TYPE, vec![], &pos);
            let desugar_expr = v_meth_call(
                et::VISIT_FLOAT,
                vec![pos_expr, Expr((), pos.clone(), expr_)],
                &pos,
            );
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`'foo'`
        // Virtualized: MyDsl::stringType()
        // Desugared: $0v->visitString(new ExprPos(...), 'foo')
        String(_) => {
            let virtual_expr = static_meth_call(visitor_name, et::STRING_TYPE, vec![], &pos);
            let desugar_expr = v_meth_call(
                et::VISIT_STRING,
                vec![pos_expr, Expr((), pos.clone(), expr_)],
                &pos,
            );
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`true`
        // Virtualized: MyDsl::boolType()
        // Desugared: $0v->visitBool(new ExprPos(...), true)
        True | False => {
            let virtual_expr = static_meth_call(visitor_name, et::BOOL_TYPE, vec![], &pos);
            let desugar_expr = v_meth_call(
                et::VISIT_BOOL,
                vec![pos_expr, Expr((), pos.clone(), expr_)],
                &pos,
            );
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`null`
        // Virtualized: MyDsl::nullType()
        // Desugared: $0v->visitNull(new ExprPos(...))
        Null => {
            let virtual_expr = static_meth_call(visitor_name, et::NULL_TYPE, vec![], &pos);
            let desugar_expr = v_meth_call(et::VISIT_NULL, vec![pos_expr], &pos);
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`$x`
        // Virtualized: $x
        // Desugared: $0v->visitLocal(new ExprPos(...), '$x')
        Lvar(lid) => {
            let desugar_expr = v_meth_call(
                et::VISIT_LOCAL,
                vec![pos_expr, string_literal(lid.0.clone(), &((lid.1).1))],
                &pos,
            );
            let virtual_expr = Expr((), pos, Lvar(lid));
            (virtual_expr, desugar_expr)
        }
        Binop(bop) => {
            let (op, lhs, rhs) = *bop;
            let (virtual_lhs, desugar_lhs) = rewrite_expr(temps, lhs, visitor_name)?;
            let (virtual_rhs, desugar_rhs) = rewrite_expr(temps, rhs, visitor_name)?;
            if op == Bop::Eq(None) {
                // Source: MyDsl`$x = ...`
                // Virtualized: $x = ...
                // Desugared: $0v->visitAssign(new ExprPos(...), $0v->visitLocal(...), ...)
                let desugar_expr = v_meth_call(
                    et::VISIT_ASSIGN,
                    vec![pos_expr, desugar_lhs, desugar_rhs],
                    &pos,
                );
                let virtual_expr = Expr((), pos, Binop(Box::new((op, virtual_lhs, virtual_rhs))));
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
                    et::VISIT_BINOP,
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
            let (virtual_operand, desugar_operand) = rewrite_expr(temps, operand, visitor_name)?;
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
                // Postfix ++
                Uop::Upincr => "__postfixPlusPlus",
                // Prefix ++
                Uop::Uincr => {
                    return Err((
                        pos,
                        "Expression trees only support postfix increment operator `$x++`.".into(),
                    ));
                }
                // Postfix --
                Uop::Updecr => "__postfixMinusMinus",
                // Prefix --
                Uop::Udecr => {
                    return Err((
                        pos,
                        "Expression trees only support postfix decrement operator `$x--`.".into(),
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
                et::VISIT_UNOP,
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
            let (virtual_e1, desugar_e1) = rewrite_expr(temps, e1, visitor_name)?;
            let (virtual_e2, desugar_e2) = if let Some(e2) = e2o {
                rewrite_expr(temps, e2, visitor_name)?
            } else {
                return Err((
                    pos,
                    "Unsupported expression tree syntax: Elvis operator".into(),
                ));
            };
            let (virtual_e3, desugar_e3) = rewrite_expr(temps, e3, visitor_name)?;

            let desugar_expr = v_meth_call(
                et::VISIT_TERNARY,
                vec![pos_expr, desugar_e1, desugar_e2, desugar_e3],
                &pos,
            );
            let virtual_expr = Expr(
                (),
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
            match &recv.2 {
                // Don't transform calls to `hh_show`.
                Id(sid) if is_typechecker_fun_name(&sid.1) => {
                    let call_e = Expr::new(
                        (),
                        pos.clone(),
                        Call(Box::new((recv, targs, args, variadic))),
                    );
                    return Ok((call_e.clone(), call_e));
                }
                _ => {}
            }

            let args = args
                .into_iter()
                .map(|e| match e {
                    (ParamKind::Pnormal, e) => Ok(e),
                    (ParamKind::Pinout(_), Expr(_, p, _)) => Err((
                        p,
                        "Expression trees do not support `inout` function calls.".into(),
                    )),
                })
                .collect::<Result<Vec<_>, _>>()?;

            let (virtual_args, desugar_args) = rewrite_exprs(temps, args, visitor_name)?;

            match recv.2 {
                // Source: MyDsl`foo()`
                // Virtualized: MyDsl::symbolType($0fpXX)()
                // Desugared: $0v->visitCall(new ExprPos(...), $0v->visitGlobalFunction(new ExprPos(...), $0fpXX), vec[])
                Id(sid) => {
                    let len = temps.global_function_pointers.len();
                    temps.global_function_pointers.push(global_func_ptr(&sid));
                    let temp_variable = temp_function_pointer_lvar(&recv.1, len);

                    let desugar_expr = v_meth_call(
                        et::VISIT_CALL,
                        vec![
                            pos_expr.clone(),
                            v_meth_call(
                                et::VISIT_GLOBAL_FUNCTION,
                                vec![pos_expr, temp_variable.clone()],
                                &pos,
                            ),
                            vec_literal(desugar_args),
                        ],
                        &pos,
                    );
                    let virtual_expr = Expr(
                        (),
                        pos.clone(),
                        Call(Box::new((
                            static_meth_call(
                                visitor_name,
                                et::SYMBOL_TYPE,
                                vec![temp_variable],
                                &pos,
                            ),
                            vec![],
                            build_args(virtual_args),
                            None,
                        ))),
                    );
                    (virtual_expr, desugar_expr)
                }
                // Source: MyDsl`Foo::bar()`
                // Virtualized: MyDsl::symbolType($0smXX)()
                // Desugared: $0v->visitCall(new ExprPos(...), $0v->visitStaticMethod(new ExprPos(...), $0smXX, vec[])
                ClassConst(cc) => {
                    let (cid, s) = *cc;
                    if let ClassId_::CIexpr(Expr(_, _, Id(sid))) = &cid.2 {
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

                    let len = temps.static_method_pointers.len();
                    temps
                        .static_method_pointers
                        .push(static_meth_ptr(&recv.1, &cid, &s));
                    let temp_variable = temp_static_method_lvar(&recv.1, len);

                    let desugar_expr = v_meth_call(
                        et::VISIT_CALL,
                        vec![
                            pos_expr.clone(),
                            v_meth_call(
                                et::VISIT_STATIC_METHOD,
                                vec![pos_expr, temp_variable.clone()],
                                &pos,
                            ),
                            vec_literal(desugar_args),
                        ],
                        &pos,
                    );
                    let virtual_expr = Expr(
                        (),
                        pos.clone(),
                        Call(Box::new((
                            static_meth_call(
                                visitor_name,
                                et::SYMBOL_TYPE,
                                vec![temp_variable],
                                &pos,
                            ),
                            vec![],
                            build_args(virtual_args),
                            None,
                        ))),
                    );
                    (virtual_expr, desugar_expr)
                }
                // Source: MyDsl`$x->bar()`
                // Virtualized: $x->bar()
                // Desugared: $0v->visitCall($0v->visitMethodCall(new ExprPos(...), $0v->visitLocal(new ExprPos(...), '$x'), 'bar'), vec[])
                ObjGet(og) if og.3 == ast::PropOrMethod::IsMethod => {
                    let (e1, e2, null_flavor, is_prop_call) = *og;
                    if null_flavor == OgNullFlavor::OGNullsafe {
                        return Err((
                            pos,
                            "Expression Trees do not support nullsafe method calls".into(),
                        ));
                    }
                    let (virtual_e1, desugar_e1) = rewrite_expr(temps, e1, visitor_name)?;
                    let id = if let Id(id) = &e2.2 {
                        string_literal(id.0.clone(), &id.1)
                    } else {
                        return Err((
                            pos,
                            "Expression trees only support named method calls.".into(),
                        ));
                    };
                    let desugar_expr = v_meth_call(
                        et::VISIT_CALL,
                        vec![
                            pos_expr.clone(),
                            v_meth_call(
                                et::VISIT_INSTANCE_METHOD,
                                vec![pos_expr, desugar_e1, id],
                                &pos,
                            ),
                            vec_literal(desugar_args),
                        ],
                        &pos,
                    );
                    let virtual_expr = Expr(
                        (),
                        pos.clone(),
                        Call(Box::new((
                            Expr(
                                (),
                                pos,
                                ObjGet(Box::new((virtual_e1, e2, null_flavor, is_prop_call))),
                            ),
                            vec![],
                            build_args(virtual_args),
                            None,
                        ))),
                    );
                    (virtual_expr, desugar_expr)
                }
                _ => {
                    let (virtual_recv, desugar_recv) =
                        rewrite_expr(temps, Expr((), recv.1, recv.2), visitor_name)?;
                    let desugar_expr = v_meth_call(
                        et::VISIT_CALL,
                        vec![pos_expr, desugar_recv, vec_literal(desugar_args)],
                        &pos,
                    );
                    let virtual_expr = Expr(
                        (),
                        pos,
                        Call(Box::new((
                            virtual_recv,
                            vec![],
                            build_args(virtual_args),
                            None,
                        ))),
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

            let body = std::mem::take(&mut fun_.body.fb_ast);

            let should_append_return = only_void_return(&body);

            let (mut virtual_body_stmts, desugar_body) = rewrite_stmts(temps, body, visitor_name)?;

            if should_append_return {
                virtual_body_stmts.push(Stmt(
                    pos.clone(),
                    aast::Stmt_::Return(Box::new(Some(static_meth_call(
                        visitor_name,
                        et::VOID_TYPE,
                        vec![],
                        &pos,
                    )))),
                ));
            }

            let desugar_expr = v_meth_call(
                et::VISIT_LAMBDA,
                vec![
                    pos_expr,
                    vec_literal(param_names),
                    vec_literal(desugar_body),
                ],
                &pos,
            );
            fun_.body.fb_ast = virtual_body_stmts;
            let virtual_expr = Expr((), pos, Lfun(Box::new((fun_, vec![]))));
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`${ ... }`
        // Virtualized to `${ ... }`
        // Desugared to `$0v->splice(new ExprPos(...), '$var_name', ...)`
        ETSplice(e) => {
            check_nested_splice(&e)?;
            let len = temps.splices.len();
            let expr_pos = e.1.clone();
            temps.splices.push(*e);
            let temp_variable = temp_splice_lvar(&expr_pos, len);
            let temp_variable_string = string_literal(expr_pos, &temp_splice_lvar_string(len));
            let desugar_expr = v_meth_call(
                et::SPLICE,
                vec![pos_expr, temp_variable_string, temp_variable.clone()],
                &pos,
            );
            let virtual_expr = Expr((), pos, ETSplice(Box::new(temp_variable)));
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`(...)->foo`
        // Virtualized to: `(...)->foo`
        // Desugared to `$0v->visitPropertyAccess(new ExprPos(...), ...), 'foo')`
        ObjGet(og) => {
            let (e1, e2, null_flavor, is_prop_call) = *og;
            if null_flavor == OgNullFlavor::OGNullsafe {
                return Err((
                    pos,
                    "Expression Trees do not support nullsafe property access".into(),
                ));
            }
            let (virtual_e1, desugar_e1) = rewrite_expr(temps, e1, visitor_name)?;
            let id = if let Id(id) = &e2.2 {
                string_literal(id.0.clone(), &id.1)
            } else {
                return Err((
                    pos,
                    "Expression trees only support named property access.".into(),
                ));
            };
            let desugar_expr = v_meth_call(
                et::VISIT_PROPERTY_ACCESS,
                vec![pos_expr, desugar_e1, id],
                &pos,
            );
            let virtual_expr = Expr(
                (),
                pos,
                ObjGet(Box::new((virtual_e1, e2, null_flavor, is_prop_call))),
            );
            (virtual_expr, desugar_expr)
        }
        // Source: MyDsl`<foo my-attr="stuff">text <foo-child/> </foo>`
        // Virtualized: <foo my-attr={MyDsl::stringType()}>{MyDsl::stringType()} <foo-child/> </foo>
        // Desugared:
        //   $0v->visitXhp(
        //     new ExprPos(...),
        //     :foo::class,
        //     dict["my-attr" => $0v->visitString(...)],
        //     vec[
        //       $0v->visitString(..., "text ")],
        //       $0v->visitXhp(..., :foo-child::class, ...),
        //     ],
        //   )
        Xml(xml) => {
            let (hint, attrs, children) = *xml;

            let mut virtual_attrs = vec![];
            let mut desugar_attrs = vec![];
            for attr in attrs.clone() {
                match attr {
                    aast::XhpAttribute::XhpSimple(xs) => {
                        let (attr_name_pos, attr_name) = xs.name.clone();
                        let dict_key =
                            Expr::new((), attr_name_pos, Expr_::String(BString::from(attr_name)));

                        let (virtual_attr_expr, desugar_attr_expr) =
                            rewrite_expr(temps, xs.expr, visitor_name)?;

                        desugar_attrs.push((dict_key, desugar_attr_expr));
                        virtual_attrs.push(aast::XhpAttribute::XhpSimple(aast::XhpSimple {
                            expr: virtual_attr_expr,
                            ..xs
                        }))
                    }
                    aast::XhpAttribute::XhpSpread(e) => {
                        return Err((
                            e.1,
                            "Expression trees do not support attribute spread syntax.".into(),
                        ));
                    }
                }
            }

            let (virtual_children, desugar_children) =
                rewrite_exprs(temps, children.clone(), visitor_name)?;

            // Construct :foo::class.
            let hint_pos = hint.0.clone();
            let hint_class = Expr_::ClassConst(Box::new((
                ClassId(
                    (),
                    hint_pos.clone(),
                    ClassId_::CIexpr(Expr::new(
                        (),
                        hint_pos.clone(),
                        Expr_::Id(Box::new(ast_defs::Id(hint_pos.clone(), hint.1.clone()))),
                    )),
                ),
                (hint_pos.clone(), "class".to_string()),
            )));

            let virtual_expr = Expr(
                (),
                pos.clone(),
                Xml(Box::new((hint, virtual_attrs, virtual_children))),
            );
            let desugar_expr = v_meth_call(
                et::VISIT_XHP,
                vec![
                    pos_expr,
                    Expr((), pos.clone(), hint_class),
                    dict_literal(&pos, desugar_attrs),
                    vec_literal(desugar_children),
                ],
                &pos,
            );
            (virtual_expr, desugar_expr)
        }
        ClassConst(_) => {
            return Err((
                pos,
                "Expression trees do not support directly referencing class consts. Consider splicing values defined outside the scope of an Expression Tree using ${...}.".into(),
            ));
        }
        Efun(_) => {
            return Err((
                pos,
                "Expression trees do not support PHP lambdas. Consider using Hack lambdas `() ==> {}` instead.".into(),
            ));
        }
        ExpressionTree(_) => {
            return Err((pos, "Expression trees may not be nested. Consider splicing Expression trees together using `${}`.".into()));
        }
        _ => {
            return Err((pos, "Unsupported expression tree syntax.".into()));
        }
    };
    Ok((virtual_expr, desugar_expr))
}

fn rewrite_exprs(
    temps: &mut Temporaries,
    exprs: Vec<Expr>,
    visitor_name: &str,
) -> Result<(Vec<Expr>, Vec<Expr>), (Pos, String)> {
    let mut virtual_results = Vec::with_capacity(exprs.len());
    let mut desugar_results = Vec::with_capacity(exprs.len());
    for expr in exprs {
        let (virtual_expr, desugar_expr) = rewrite_expr(temps, expr, visitor_name)?;
        virtual_results.push(virtual_expr);
        desugar_results.push(desugar_expr);
    }
    Ok((virtual_results, desugar_results))
}

fn rewrite_stmts(
    temps: &mut Temporaries,
    stmts: Vec<Stmt>,
    visitor_name: &str,
) -> Result<(Vec<Stmt>, Vec<Expr>), (Pos, String)> {
    let mut virtual_results = Vec::with_capacity(stmts.len());
    let mut desugar_results = Vec::with_capacity(stmts.len());
    for stmt in stmts {
        let (virtual_stmt, desugared_expr) = rewrite_stmt(temps, stmt, visitor_name)?;
        virtual_results.push(virtual_stmt);
        if let Some(desugared_expr) = desugared_expr {
            desugar_results.push(desugared_expr);
        }
    }
    Ok((virtual_results, desugar_results))
}

fn rewrite_stmt(
    temps: &mut Temporaries,
    s: Stmt,
    visitor_name: &str,
) -> Result<(Stmt, Option<Expr>), (Pos, String)> {
    use aast::Stmt_::*;

    let Stmt(pos, stmt_) = s;
    let pos_expr = exprpos(&pos);

    let virtual_desugar = match stmt_ {
        Expr(e) => {
            let (virtual_expr, desugar_expr) = rewrite_expr(temps, *e, visitor_name)?;
            (Stmt(pos, Expr(Box::new(virtual_expr))), Some(desugar_expr))
        }
        Return(e) => match *e {
            // Source: MyDsl`return ...;`
            // Virtualized: return ...;
            // Desugared: $0v->visitReturn(new ExprPos(...), $0v->...)
            Some(e) => {
                let (virtual_expr, desugar_expr) = rewrite_expr(temps, e, visitor_name)?;
                let desugar_expr =
                    v_meth_call(et::VISIT_RETURN, vec![pos_expr, desugar_expr], &pos);
                let virtual_stmt = Stmt(pos, Return(Box::new(Some(virtual_expr))));
                (virtual_stmt, Some(desugar_expr))
            }
            // Source: MyDsl`return;`
            // Virtualized: return MyDsl::voidType();
            // Desugared: $0v->visitReturn(new ExprPos(...), null)
            None => {
                let desugar_expr = v_meth_call(
                    et::VISIT_RETURN,
                    vec![pos_expr, null_literal(pos.clone())],
                    &pos,
                );

                let virtual_void_expr = static_meth_call(visitor_name, et::VOID_TYPE, vec![], &pos);
                let virtual_stmt = Stmt(pos, Return(Box::new(Some(virtual_void_expr))));
                (virtual_stmt, Some(desugar_expr))
            }
        },
        // Source: MyDsl`if (...) {...} else {...}`
        // Virtualized: if (...->__bool())) {...} else {...}
        // Desugared: $0v->visitIf(new ExprPos(...), $0v->..., vec[...], vec[...])
        If(if_stmt) => {
            let (cond_expr, then_block, else_block) = *if_stmt;
            let (virtual_cond, desugar_cond) = rewrite_expr(temps, cond_expr, visitor_name)?;
            let (virtual_then_stmts, desugar_then) =
                rewrite_stmts(temps, then_block, visitor_name)?;
            let (virtual_else_stmts, desugar_else) =
                rewrite_stmts(temps, else_block, visitor_name)?;

            let desugar_expr = v_meth_call(
                et::VISIT_IF,
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
            let (virtual_cond, desugar_cond) = rewrite_expr(temps, cond, visitor_name)?;
            let (virtual_body_stmts, desugar_body) = rewrite_stmts(temps, body, visitor_name)?;

            let desugar_expr = v_meth_call(
                et::VISIT_WHILE,
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
            let (virtual_init_exprs, desugar_init_exprs) =
                rewrite_exprs(temps, init, visitor_name)?;
            let (virtual_cond_option, desugar_cond_expr) = match cond {
                Some(cond) => {
                    let (virtual_cond, desugar_cond) = rewrite_expr(temps, cond, visitor_name)?;
                    (Some(boolify(virtual_cond)), desugar_cond)
                }
                None => (None, null_literal(pos.clone())),
            };
            let (virtual_incr_exprs, desugar_incr_exprs) =
                rewrite_exprs(temps, incr, visitor_name)?;
            let (virtual_body_stmts, desugar_body) = rewrite_stmts(temps, body, visitor_name)?;

            let desugar_expr = v_meth_call(
                et::VISIT_FOR,
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
            let desugar_expr = v_meth_call(et::VISIT_BREAK, vec![pos_expr], &pos);
            let virtual_stmt = Stmt(pos, Break);
            (virtual_stmt, Some(desugar_expr))
        }
        // Source: MyDsl`continue;`
        // Virtualized: continue;
        // Desugared: $0v->visitContinue(new ExprPos(...))
        Continue => {
            let desugar_expr = v_meth_call(et::VISIT_CONTINUE, vec![pos_expr], &pos);
            let virtual_stmt = Stmt(pos, Continue);
            (virtual_stmt, Some(desugar_expr))
        }
        Noop => (Stmt(pos, Noop), None),
        // Unsupported operators
        Do(_) => {
            return Err((
                pos,
                "Expression trees do not support `do while` loops. Consider using a `while` loop instead.".into(),
            ));
        }
        Switch(_) => {
            return Err((
                pos,
                "Expression trees do not support `switch` statements. Consider using `if`/`else if`/`else` instead.".into(),
            ));
        }
        Foreach(_) => {
            return Err((
                pos,
                "Expression trees do not support `foreach` loops. Consider using a `for` loop or a `while` loop instead.".into(),
            ));
        }
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

fn immediately_invoked_lambda(
    pos: &Pos,
    stmts: Vec<Stmt>,
    captured_arguments: Vec<((String, Pos), Expr)>,
) -> Expr {
    let (params_name_pos, call_args): (Vec<(String, Pos)>, Vec<Expr>) =
        captured_arguments.into_iter().unzip();

    let fun_params = params_name_pos
        .into_iter()
        .map(|(name, pos): (String, Pos)| -> ast::FunParam {
            ast::FunParam {
                annotation: (),
                type_hint: ast::TypeHint((), None),
                is_variadic: false,
                pos,
                name,
                expr: None,
                callconv: ParamKind::Pnormal,
                readonly: None,
                user_attributes: vec![],
                visibility: None,
            }
        })
        .collect();

    let call_args = call_args
        .into_iter()
        .map(|e: Expr| -> (ParamKind, Expr) { (ParamKind::Pnormal, e) })
        .collect();

    let func_body = ast::FuncBody { fb_ast: stmts };
    let fun_ = wrap_fun_(func_body, fun_params, pos.clone());
    let lambda_expr = Expr::new((), pos.clone(), Expr_::mk_lfun(fun_, vec![]));

    Expr::new(
        (),
        pos.clone(),
        Expr_::Call(Box::new((lambda_expr, vec![], call_args, None))),
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
    functions: &[Expr],
    static_methods: &[Expr],
) -> Expr {
    let key_value_pairs = splices
        .iter()
        .enumerate()
        .map(|(i, expr)| {
            let key = Expr::new(
                (),
                expr.1.clone(),
                Expr_::String(BString::from(temp_splice_lvar_string(i))),
            );
            let value = temp_splice_lvar(&expr.1, i);
            (key, value)
        })
        .collect();
    let splices_dict = dict_literal(pos, key_value_pairs);

    let function_vars = functions
        .iter()
        .enumerate()
        .map(|(i, expr)| temp_function_pointer_lvar(&expr.1, i))
        .collect();
    let functions_vec = vec_literal_with_pos(&pos, function_vars);

    let static_method_vars = static_methods
        .iter()
        .enumerate()
        .map(|(i, expr)| temp_static_method_lvar(&expr.1, i))
        .collect();
    let static_method_vec = vec_literal_with_pos(&pos, static_method_vars);

    shape_literal(
        pos,
        vec![
            ("splices", splices_dict),
            ("functions", functions_vec),
            ("static_methods", static_method_vec),
        ],
    )
}

fn global_func_ptr(sid: &Sid) -> Expr {
    let pos = sid.0.clone();
    Expr::new(
        (),
        pos.clone(),
        Expr_::FunctionPointer(Box::new((ast::FunctionPtrId::FPId(sid.clone()), vec![]))),
    )
}

fn static_meth_ptr(pos: &Pos, cid: &ClassId, meth: &Pstring) -> Expr {
    Expr::new(
        (),
        pos.clone(),
        Expr_::FunctionPointer(Box::new((
            aast::FunctionPtrId::FPClassConst(cid.clone(), meth.clone()),
            vec![],
        ))),
    )
}
