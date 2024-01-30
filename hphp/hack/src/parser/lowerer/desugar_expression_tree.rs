use bstr::BString;
use naming_special_names_rust::classes;
use naming_special_names_rust::expression_trees as et;
use naming_special_names_rust::pseudo_functions;
use naming_special_names_rust::special_idents;
use oxidized::aast;
use oxidized::aast_visitor::visit;
use oxidized::aast_visitor::visit_mut;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::Node;
use oxidized::aast_visitor::NodeMut;
use oxidized::aast_visitor::Visitor;
use oxidized::aast_visitor::VisitorMut;
use oxidized::ast;
use oxidized::ast::ClassId;
use oxidized::ast::ClassId_;
use oxidized::ast::Expr;
use oxidized::ast::Expr_;
use oxidized::ast::Hint_;
use oxidized::ast::Sid;
use oxidized::ast::Stmt;
use oxidized::ast::Stmt_;
use oxidized::ast_defs;
use oxidized::ast_defs::*;
use oxidized::local_id;
use oxidized::pos::Pos;

use crate::lowerer::Env;

pub struct DesugarResult {
    pub expr: Expr,
    pub errors: Vec<(Pos, String)>,
}

struct RewriteResult {
    virtual_expr: Expr,
    desugar_expr: Expr,
}

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
pub fn desugar(hint: &aast::Hint, e: Expr, env: &Env<'_>) -> DesugarResult {
    let mut errors = vec![];

    let visitor_name = match hint_name(hint) {
        Ok(name) => name,
        Err((pos, msg)) => {
            errors.push((pos, msg));
            "unknown".into()
        }
    };
    let et_literal_pos = e.1.clone();
    let et_hint_pos = hint.0.clone();

    let mut temps = Temporaries {
        splices: vec![],
        global_function_pointers: vec![],
        static_method_pointers: vec![],
    };
    let rewritten_expr = rewrite_expr(
        &mut temps,
        e,
        &visitor_name,
        &mut errors,
        env.parser_options.tco_expression_tree_virtualize_functions,
    );

    let dollardollar_pos = rewrite_dollardollars(&mut temps.splices);

    let splice_count = temps.splices.len();
    let function_count = temps.global_function_pointers.len();
    let static_method_count = temps.static_method_pointers.len();

    let metadata = maketree_metadata(
        &et_hint_pos,
        &temps.splices,
        &temps.global_function_pointers,
        &temps.static_method_pointers,
    );

    // Make anonymous function of smart constructor calls
    let visitor_expr = wrap_return(rewritten_expr.desugar_expr, &et_literal_pos);
    let visitor_body = ast::FuncBody {
        fb_ast: ast::Block(vec![visitor_expr]),
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
        user_attributes: Default::default(),
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
            fb_ast: ast::Block(vec![wrap_return(
                rewritten_expr.virtual_expr,
                &et_literal_pos,
            )]),
        };
        let mut typing_fun_ = wrap_fun_(typing_fun_body, vec![], et_literal_pos.clone());
        typing_fun_.ctxs = Some(aast::Contexts(
            et_literal_pos.clone(),
            vec![ast::Hint(
                et_literal_pos.clone(),
                Box::new(Hint_::Happly(
                    Id(
                        et_literal_pos.clone(),
                        naming_special_names_rust::coeffects::DEFAULTS.to_string(),
                    ),
                    vec![],
                )),
            )],
        ));
        let mut spliced_vars: Vec<_> = (0..splice_count)
            .map(|i| {
                ast::CaptureLid(
                    (),
                    ast::Lid(et_hint_pos.clone(), (0, temp_splice_lvar_string(i))),
                )
            })
            .collect();
        let function_pointer_vars: Vec<_> = (0..function_count)
            .map(|i| {
                ast::CaptureLid(
                    (),
                    ast::Lid(
                        et_hint_pos.clone(),
                        (0, temp_function_pointer_lvar_string(i)),
                    ),
                )
            })
            .collect();
        let static_method_vars: Vec<_> = (0..static_method_count)
            .map(|i| {
                ast::CaptureLid(
                    (),
                    ast::Lid(et_hint_pos.clone(), (0, temp_static_method_lvar_string(i))),
                )
            })
            .collect();
        spliced_vars.extend(function_pointer_vars);
        spliced_vars.extend(static_method_vars);
        Expr::new(
            (),
            et_literal_pos.clone(),
            Expr_::mk_efun(aast::Efun {
                fun: typing_fun_,
                use_: spliced_vars,
                closure_class_name: None,
            }),
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
        &et_hint_pos,
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

    let expr = Expr::new(
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
    );
    DesugarResult { expr, errors }
}

/// Convert `foo` to `return foo;`.
fn wrap_return(e: Expr, pos: &Pos) -> Stmt {
    Stmt::new(pos.clone(), Stmt_::Return(Box::new(Some(e))))
}

/// Wrap a FuncBody into an anonymous Fun_
fn wrap_fun_(body: ast::FuncBody, params: Vec<ast::FunParam>, span: Pos) -> ast::Fun_ {
    ast::Fun_ {
        span,
        readonly_this: None,
        annotation: (),
        readonly_ret: None,
        ret: ast::TypeHint((), None),
        params,
        body,
        fun_kind: ast::FunKind::FSync,
        ctxs: None,        // TODO(T70095684)
        unsafe_ctxs: None, // TODO(T70095684)
        user_attributes: Default::default(),
        external: false,
        doc_comment: None,
    }
}

struct DollarDollarRewriter {
    pos: Option<Pos>,
}

impl<'ast> VisitorMut<'ast> for DollarDollarRewriter {
    type Params = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, Params = Self::Params> {
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

fn rewrite_dollardollars(el: &mut [ast::Expr]) -> Option<Pos> {
    let mut rewriter = DollarDollarRewriter { pos: None };
    for e in el.iter_mut() {
        visit_mut(&mut rewriter, &mut (), e).expect("DollarDollarRewriter never errors");
    }
    rewriter.pos
}

struct VoidReturnCheck {
    only_void_return: bool,
}

impl<'ast> Visitor<'ast> for VoidReturnCheck {
    type Params = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
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

fn only_void_return(lfun_body: &[ast::Stmt]) -> bool {
    let mut checker = VoidReturnCheck {
        only_void_return: true,
    };
    visit(&mut checker, &mut (), &lfun_body).unwrap();
    checker.only_void_return
}

struct NestedSpliceCheck {
    has_nested_splice: Option<Pos>,
    has_nested_expression_tree: Option<Pos>,
}

impl<'ast> Visitor<'ast> for NestedSpliceCheck {
    type Params = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
        self
    }

    fn visit_expr(&mut self, env: &mut (), e: &aast::Expr<(), ()>) -> Result<(), ()> {
        use aast::Expr_::*;

        match &e.2 {
            ETSplice(_) => {
                self.has_nested_splice = Some(e.1.clone());
            }
            ExpressionTree(_) => {
                self.has_nested_expression_tree = Some(e.1.clone());
            }
            _ if self.has_nested_splice.is_none() && self.has_nested_expression_tree.is_none() => {
                e.recurse(env, self)?
            }
            _ => {}
        }
        Ok(())
    }
}

/// Assumes that the Expr is the expression within a splice.
/// If the expression has an Expression Tree contained within or a splice, then
/// we have nested expression trees or splices and this should raise an error.
fn check_nested_splice(e: &ast::Expr) -> Result<(), (Pos, String)> {
    let mut checker = NestedSpliceCheck {
        has_nested_splice: None,
        has_nested_expression_tree: None,
    };
    visit(&mut checker, &mut (), e).unwrap();
    if let Some(p) = checker.has_nested_splice {
        return Err((p, "Splice syntax `${...}` cannot be nested.".into()));
    }
    if let Some(p) = checker.has_nested_expression_tree {
        return Err((p, "Expression trees may not be nested. Consider assigning to a local variable and splicing the local variable in.".into()));
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
    Expr::new(
        (),
        pos.clone(),
        Expr_::ValCollection(Box::new(((pos.clone(), aast::VcKind::Vec), None, items))),
    )
}

fn dict_literal(pos: &Pos, key_value_pairs: Vec<(Expr, Expr)>) -> Expr {
    let fields = key_value_pairs
        .into_iter()
        .map(|(k, v)| aast::Field(k, v))
        .collect();
    Expr::new(
        (),
        pos.clone(),
        Expr_::KeyValCollection(Box::new(((pos.clone(), aast::KvcKind::Dict), None, fields))),
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

    let c = Expr_::Call(Box::new(ast::CallExpr {
        func: Expr::new(
            (),
            pos.clone(),
            Expr_::ObjGet(Box::new((
                receiver,
                meth,
                OgNullFlavor::OGNullthrows,
                ast::PropOrMethod::IsMethod,
            ))),
        ),
        targs: vec![],
        args: build_args(args),
        unpacked_arg: None,
    }));
    Expr::new((), pos.clone(), c)
}

fn meth_call(receiver: Expr, meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    let meth = Expr::new(
        (),
        pos.clone(),
        Expr_::Id(Box::new(ast::Id(pos.clone(), meth_name.into()))),
    );

    let c = Expr_::Call(Box::new(ast::CallExpr {
        func: Expr::new(
            (),
            pos.clone(),
            Expr_::ObjGet(Box::new((
                receiver,
                meth,
                OgNullFlavor::OGNullthrows,
                ast::PropOrMethod::IsMethod,
            ))),
        ),
        targs: vec![],
        args: build_args(args),
        unpacked_arg: None,
    }));
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
        Expr_::Call(Box::new(ast::CallExpr {
            func: callee,
            targs: vec![],
            args: build_args(args),
            unpacked_arg: None,
        })),
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
        .unwrap_or(Pos::NONE)
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
                    Expr_::Binop(Box::new(aast::Binop {
                        bop: Bop::Eq(None),
                        lhs: mk_lvar(&expr.1, i),
                        rhs: expr,
                    })),
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
        let ((start_lnum, start_bol, start_offset), (end_lnum, end_bol, end_offset)) =
            pos.to_start_and_end_lnum_bol_offset();

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
                int_literal(pos.clone(), start_offset - start_bol),
            ),
            ("end_column", int_literal(pos.clone(), end_offset - end_bol)),
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
    errors: &mut Vec<(Pos, String)>,
    should_virtualize_functions: bool,
) -> RewriteResult {
    use aast::Expr_::*;

    // If we can't rewrite the expression (e.g. due to unsupported syntax), return the
    // original syntax unmodified. This is particularly useful during code completion,
    // where an unfinished code fragment might accidentally use unsupported syntax.
    let unchanged_result = RewriteResult {
        virtual_expr: e.clone(),
        desugar_expr: e.clone(),
    };

    let Expr(_, pos, expr_) = e;
    let pos_expr = exprpos(&pos);
    match expr_ {
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
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
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
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
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
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
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
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
        }
        // Source: MyDsl`null`
        // Virtualized: MyDsl::nullType()
        // Desugared: $0v->visitNull(new ExprPos(...))
        Null => {
            let virtual_expr = static_meth_call(visitor_name, et::NULL_TYPE, vec![], &pos);
            let desugar_expr = v_meth_call(et::VISIT_NULL, vec![pos_expr], &pos);
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
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
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
        }
        Binop(binop) => {
            let aast::Binop { bop, lhs, rhs } = *binop;
            let rewritten_lhs = rewrite_expr(
                temps,
                lhs,
                visitor_name,
                errors,
                should_virtualize_functions,
            );
            let rewritten_rhs = rewrite_expr(
                temps,
                rhs,
                visitor_name,
                errors,
                should_virtualize_functions,
            );

            if bop == Bop::Eq(None) {
                // Source: MyDsl`$x = ...`
                // Virtualized: $x = ...
                // Desugared: $0v->visitAssign(new ExprPos(...), $0v->visitLocal(...), ...)
                let desugar_expr = v_meth_call(
                    et::VISIT_ASSIGN,
                    vec![
                        pos_expr,
                        rewritten_lhs.desugar_expr,
                        rewritten_rhs.desugar_expr,
                    ],
                    &pos,
                );
                let virtual_expr = Expr(
                    (),
                    pos.clone(),
                    Binop(Box::new(aast::Binop {
                        bop,
                        lhs: rewritten_lhs.virtual_expr,
                        rhs: rewritten_rhs.virtual_expr,
                    })),
                );
                RewriteResult {
                    virtual_expr,
                    desugar_expr,
                }
            } else {
                // Source: MyDsl`... + ...`
                // Virtualized: ...->__plus(...)
                // Desugared: $0v->visitBinop(new ExprPos(...), ..., '__plus', ...)
                let binop_str = match bop {
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
                        errors.push((
                            pos.clone(),
                            "Expression trees do not support the exponent operator `**`.".into(),
                        ));
                        "__unsupported"
                    }
                    Bop::Eqeq | Bop::Diff => {
                        errors.push((
                            pos.clone(),
                            "Expression trees only support strict equality operators `===` and `!==`".into(),
                        ));
                        "__unsupported"
                    }
                    Bop::Cmp => {
                        errors.push((
                            pos.clone(),
                            "Expression trees do not support the spaceship operator `<=>`. Try comparison operators like `<` and `>=`".into(),
                        ));
                        "__unsupported"
                    }
                    Bop::QuestionQuestion => {
                        errors.push((
                            pos.clone(),
                            "Expression trees do not support the null coalesce operator `??`."
                                .into(),
                        ));
                        "__unsupported"
                    }
                    Bop::Eq(_) => {
                        errors.push((
                            pos.clone(),
                            "Expression trees do not support compound assignments. Try the long form style `$foo = $foo + $bar` instead.".into(),
                        ));
                        "__unsupported"
                    }
                };
                let virtual_expr = meth_call(
                    rewritten_lhs.virtual_expr,
                    binop_str,
                    vec![rewritten_rhs.virtual_expr],
                    &pos,
                );
                let desugar_expr = v_meth_call(
                    et::VISIT_BINOP,
                    vec![
                        pos_expr,
                        rewritten_lhs.desugar_expr,
                        string_literal(pos.clone(), binop_str),
                        rewritten_rhs.desugar_expr,
                    ],
                    &pos,
                );
                RewriteResult {
                    virtual_expr,
                    desugar_expr,
                }
            }
        }
        // Source: MyDsl`!...`
        // Virtualized: ...->__exclamationMark(...)
        // Desugared: $0v->visitUnop(new ExprPos(...), ..., '__exclamationMark')
        Unop(unop) => {
            let (op, operand) = *unop;
            let rewritten_operand = rewrite_expr(
                temps,
                operand,
                visitor_name,
                errors,
                should_virtualize_functions,
            );

            let op_str = match op {
                // Allow boolean not operator !$x
                Uop::Unot => "__exclamationMark",
                // Allow negation -$x (required for supporting negative literals -123)
                Uop::Uminus => "__negate",
                // Allow bitwise complement
                Uop::Utild => "__tilde",
                // Currently not allowed operators
                Uop::Uplus => {
                    errors.push((
                        pos.clone(),
                        "Expression trees do not support the unary plus operator.".into(),
                    ));
                    "__unsupported"
                }
                // Postfix ++
                Uop::Upincr => "__postfixPlusPlus",
                // Prefix ++
                Uop::Uincr => {
                    errors.push((
                        pos.clone(),
                        "Expression trees only support postfix increment operator `$x++`.".into(),
                    ));
                    "__unsupported"
                }
                // Postfix --
                Uop::Updecr => "__postfixMinusMinus",
                // Prefix --
                Uop::Udecr => {
                    errors.push((
                        pos.clone(),
                        "Expression trees only support postfix decrement operator `$x--`.".into(),
                    ));
                    "__unsupported"
                }
                Uop::Usilence => {
                    errors.push((
                        pos.clone(),
                        "Expression trees do not support the error suppression operator `@`."
                            .into(),
                    ));
                    "__unsupported"
                }
            };
            let virtual_expr = meth_call(rewritten_operand.virtual_expr, op_str, vec![], &pos);
            let desugar_expr = v_meth_call(
                et::VISIT_UNOP,
                vec![
                    pos_expr,
                    rewritten_operand.desugar_expr,
                    string_literal(pos.clone(), op_str),
                ],
                &pos,
            );
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
        }
        // Source: MyDsl`... ? ... : ...`
        // Virtualized: ...->__bool() ? ... : ...
        // Desugared: $0v->visitTernary(new ExprPos(...), ..., ..., ...)
        Eif(eif) => {
            let (e1, e2o, e3) = *eif;

            let rewritten_e1 =
                rewrite_expr(temps, e1, visitor_name, errors, should_virtualize_functions);
            let rewritten_e2 = if let Some(e2) = e2o {
                rewrite_expr(temps, e2, visitor_name, errors, should_virtualize_functions)
            } else {
                errors.push((
                    pos.clone(),
                    "Unsupported expression tree syntax: Elvis operator".into(),
                ));
                unchanged_result
            };
            let rewritten_e3 =
                rewrite_expr(temps, e3, visitor_name, errors, should_virtualize_functions);

            let desugar_expr = v_meth_call(
                et::VISIT_TERNARY,
                vec![
                    pos_expr,
                    rewritten_e1.desugar_expr,
                    rewritten_e2.desugar_expr,
                    rewritten_e3.desugar_expr,
                ],
                &pos,
            );
            let virtual_expr = Expr(
                (),
                pos,
                Eif(Box::new((
                    boolify(rewritten_e1.virtual_expr),
                    Some(rewritten_e2.virtual_expr),
                    rewritten_e3.virtual_expr,
                ))),
            );
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
        }
        // Source: MyDsl`...()`
        // Virtualized: (...->__unwrap())()
        // Desugared: $0v->visitCall(new ExprPos(...), ..., vec[])
        Call(call) => {
            let ast::CallExpr {
                func: recv,
                targs,
                args,
                unpacked_arg: variadic,
            } = *call;

            if variadic.is_some() {
                errors.push((
                    pos.clone(),
                    "Expression trees do not support variadic calls.".into(),
                ));
            }
            if !targs.is_empty() {
                errors.push((
                    pos.clone(),
                    "Expression trees do not support function calls with generics.".into(),
                ));
            }
            match &recv.2 {
                // Don't transform calls to `hh_show`.
                Id(sid) if is_typechecker_fun_name(&sid.1) => {
                    let call_e = Expr::new(
                        (),
                        pos,
                        Call(Box::new(ast::CallExpr {
                            func: recv,
                            targs,
                            args,
                            unpacked_arg: variadic,
                        })),
                    );
                    return RewriteResult {
                        desugar_expr: call_e.clone(),
                        virtual_expr: call_e,
                    };
                }
                _ => {}
            }

            let mut args_without_inout = vec![];
            for arg in args {
                match arg {
                    (ParamKind::Pnormal, e) => args_without_inout.push(e),
                    (ParamKind::Pinout(_), Expr(_, p, _)) => errors.push((
                        p,
                        "Expression trees do not support `inout` function calls.".into(),
                    )),
                }
            }

            let (virtual_args, desugar_args) = rewrite_exprs(
                temps,
                args_without_inout,
                visitor_name,
                errors,
                should_virtualize_functions,
            );

            match recv.2 {
                // Source: MyDsl`foo()`
                // Virtualized: (MyDsl::symbolType($0fpXX)->__unwrap())()
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
                        Call(Box::new(ast::CallExpr {
                            func: _virtualize_call(
                                static_meth_call(
                                    visitor_name,
                                    et::SYMBOL_TYPE,
                                    vec![temp_variable],
                                    &pos,
                                ),
                                &pos,
                                should_virtualize_functions,
                            ),
                            targs: vec![],
                            args: build_args(virtual_args),
                            unpacked_arg: None,
                        })),
                    );
                    RewriteResult {
                        virtual_expr,
                        desugar_expr,
                    }
                }
                // Source: MyDsl`Foo::bar()`
                // Virtualized: (MyDsl::symbolType($0smXX)->__unwrap())()
                // Desugared: $0v->visitCall(new ExprPos(...), $0v->visitStaticMethod(new ExprPos(...), $0smXX, vec[])
                ClassConst(cc) => {
                    let (cid, s) = *cc;
                    if let ClassId_::CIexpr(Expr(_, _, Id(sid))) = &cid.2 {
                        if sid.1 == classes::PARENT
                            || sid.1 == classes::SELF
                            || sid.1 == classes::STATIC
                        {
                            errors.push((
                                pos,
                                "Static method calls in expression trees require explicit class names.".into(),
                            ));
                            return unchanged_result;
                        }
                    } else {
                        errors.push((
                            pos,
                            "Expression trees only support function calls and static method calls on named classes.".into(),
                        ));
                        return unchanged_result;
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
                        Call(Box::new(ast::CallExpr {
                            func: _virtualize_call(
                                static_meth_call(
                                    visitor_name,
                                    et::SYMBOL_TYPE,
                                    vec![temp_variable],
                                    &pos,
                                ),
                                &pos,
                                should_virtualize_functions,
                            ),
                            targs: vec![],
                            args: build_args(virtual_args),
                            unpacked_arg: None,
                        })),
                    );
                    RewriteResult {
                        virtual_expr,
                        desugar_expr,
                    }
                }
                // Source: MyDsl`$x->bar()`
                // Virtualized: $x->bar()
                // Desugared: $0v->visitCall($0v->visitMethodCall(new ExprPos(...), $0v->visitLocal(new ExprPos(...), '$x'), 'bar'), vec[])
                ObjGet(og) if og.3 == ast::PropOrMethod::IsMethod => {
                    errors.push((
                        pos,
                        "Expression trees do not support calling instance methods".into(),
                    ));
                    unchanged_result
                }
                _ => {
                    let rewritten_recv = rewrite_expr(
                        temps,
                        Expr((), recv.1, recv.2),
                        visitor_name,
                        errors,
                        should_virtualize_functions,
                    );

                    let desugar_expr = v_meth_call(
                        et::VISIT_CALL,
                        vec![
                            pos_expr,
                            rewritten_recv.desugar_expr,
                            vec_literal(desugar_args),
                        ],
                        &pos,
                    );
                    let virtual_expr = Expr(
                        (),
                        pos.clone(),
                        Call(Box::new(ast::CallExpr {
                            func: _virtualize_call(
                                rewritten_recv.virtual_expr,
                                &pos,
                                should_virtualize_functions,
                            ),
                            targs: vec![],
                            args: build_args(virtual_args),
                            unpacked_arg: None,
                        })),
                    );
                    RewriteResult {
                        virtual_expr,
                        desugar_expr,
                    }
                }
            }
        }
        // Source: MyDsl`($x) ==> { ... }`
        // Virtualized: ($x) ==> { ...; return MyDsl::voidType(); }
        //   if no `return expr;` statements.
        // Desugared: $0v->visitLambda(new ExprPos(...), vec['$x'], vec[...]).
        Lfun(lf) => {
            let mut fun_ = lf.0;

            match &fun_ {
                aast::Fun_ {
                    // Allow a plain function that isn't async.
                    fun_kind: ast::FunKind::FSync,
                    body: _,
                    span: _,
                    doc_comment: _,
                    ret: _,
                    annotation: (),
                    params: _,
                    user_attributes: _,
                    // The function should not use any of these newer features.
                    readonly_this: None,
                    readonly_ret: None,
                    ctxs: None,
                    unsafe_ctxs: None,
                    external: false,
                } => {}
                _ => {
                    errors.push((
                            pos.clone(),
                            "Expression trees only support simple lambdas, without features like `async`, generators or capabilities."
                                .into(),
                        ));
                }
            }

            let mut param_names = Vec::with_capacity(fun_.params.len());
            for param in &fun_.params {
                if param.expr.is_some() {
                    errors.push((
                        param.pos.clone(),
                        "Expression trees do not support parameters with default values.".into(),
                    ));
                }
                param_names.push(string_literal(param.pos.clone(), &param.name));
            }

            let body = std::mem::take(&mut fun_.body.fb_ast.0);

            let should_append_return = only_void_return(&body);

            let (mut virtual_body_stmts, desugar_body) = rewrite_stmts(
                temps,
                body,
                visitor_name,
                errors,
                should_virtualize_functions,
            );
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
            fun_.body.fb_ast = ast::Block(virtual_body_stmts);

            let virtual_expr = _virtualize_lambda(
                visitor_name,
                Expr((), pos.clone(), Lfun(Box::new((fun_, vec![])))),
                &pos,
                should_virtualize_functions,
            );

            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
        }
        // Source: MyDsl`${ ... }`
        // Virtualized to `${ ... }`
        // Desugared to `$0v->splice(new ExprPos(...), '$var_name', ...)`
        ETSplice(e) => {
            if let Err(err) = check_nested_splice(&e) {
                errors.push(err);
            };

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
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
        }
        // Source: MyDsl`(...)->foo`
        // Virtualized to: `(...)->foo`
        // Desugared to `$0v->visitPropertyAccess(new ExprPos(...), ...), 'foo')`
        ObjGet(og) => {
            let (e1, e2, null_flavor, is_prop_call) = *og;

            if null_flavor == OgNullFlavor::OGNullsafe {
                errors.push((
                    pos.clone(),
                    "Expression Trees do not support nullsafe property access".into(),
                ));
            }
            let rewritten_e1 =
                rewrite_expr(temps, e1, visitor_name, errors, should_virtualize_functions);

            let id = if let Id(id) = &e2.2 {
                string_literal(id.0.clone(), &id.1)
            } else {
                errors.push((
                    pos.clone(),
                    "Expression trees only support named property access.".into(),
                ));
                e2.clone()
            };
            let desugar_expr = v_meth_call(
                et::VISIT_PROPERTY_ACCESS,
                vec![pos_expr, rewritten_e1.desugar_expr, id],
                &pos,
            );

            let virtual_expr = Expr(
                (),
                pos,
                ObjGet(Box::new((
                    rewritten_e1.virtual_expr,
                    e2,
                    null_flavor,
                    is_prop_call,
                ))),
            );
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
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
            for attr in attrs {
                match attr {
                    aast::XhpAttribute::XhpSimple(xs) => {
                        let (attr_name_pos, attr_name) = xs.name.clone();
                        let dict_key =
                            Expr::new((), attr_name_pos, Expr_::String(BString::from(attr_name)));

                        let rewritten_attr_expr = rewrite_expr(
                            temps,
                            xs.expr,
                            visitor_name,
                            errors,
                            should_virtualize_functions,
                        );
                        desugar_attrs.push((dict_key, rewritten_attr_expr.desugar_expr));
                        virtual_attrs.push(aast::XhpAttribute::XhpSimple(aast::XhpSimple {
                            expr: rewritten_attr_expr.virtual_expr,
                            ..xs
                        }))
                    }
                    aast::XhpAttribute::XhpSpread(e) => {
                        errors.push((
                            e.1,
                            "Expression trees do not support attribute spread syntax.".into(),
                        ));
                    }
                }
            }

            let (virtual_children, desugar_children) = rewrite_exprs(
                temps,
                children,
                visitor_name,
                errors,
                should_virtualize_functions,
            );

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
                (hint_pos, "class".to_string()),
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
            RewriteResult {
                virtual_expr,
                desugar_expr,
            }
        }
        ClassConst(_) => {
            errors.push((
                pos,
                "Expression trees do not support directly referencing class consts. Consider splicing values defined outside the scope of an Expression Tree using ${...}.".into(),
            ));
            unchanged_result
        }
        Efun(_) => {
            errors.push((
                pos,
                "Expression trees do not support PHP lambdas. Consider using Hack lambdas `() ==> {}` instead.".into(),
            ));
            unchanged_result
        }
        ExpressionTree(_) => {
            errors.push((
                pos,
                "Expression trees may not be nested. Consider splicing Expression trees together using `${}`.".into()
            ));
            unchanged_result
        }
        _ => {
            errors.push((pos, "Unsupported expression tree syntax.".into()));
            unchanged_result
        }
    }
}

fn rewrite_exprs(
    temps: &mut Temporaries,
    exprs: Vec<Expr>,
    visitor_name: &str,
    errors: &mut Vec<(Pos, String)>,
    should_virtualize_functions: bool,
) -> (Vec<Expr>, Vec<Expr>) {
    let mut virtual_results = Vec::with_capacity(exprs.len());
    let mut desugar_results = Vec::with_capacity(exprs.len());
    for expr in exprs {
        let rewritten_expr = rewrite_expr(
            temps,
            expr,
            visitor_name,
            errors,
            should_virtualize_functions,
        );
        virtual_results.push(rewritten_expr.virtual_expr);
        desugar_results.push(rewritten_expr.desugar_expr);
    }
    (virtual_results, desugar_results)
}

fn rewrite_stmts(
    temps: &mut Temporaries,
    stmts: Vec<Stmt>,
    visitor_name: &str,
    errors: &mut Vec<(Pos, String)>,
    should_virtualize_functions: bool,
) -> (Vec<Stmt>, Vec<Expr>) {
    let mut virtual_results = Vec::with_capacity(stmts.len());
    let mut desugar_results = Vec::with_capacity(stmts.len());
    for stmt in stmts {
        let (virtual_stmt, desugared_expr) = rewrite_stmt(
            temps,
            stmt,
            visitor_name,
            errors,
            should_virtualize_functions,
        );
        virtual_results.push(virtual_stmt);
        if let Some(desugared_expr) = desugared_expr {
            desugar_results.push(desugared_expr);
        }
    }
    (virtual_results, desugar_results)
}

fn rewrite_stmt(
    temps: &mut Temporaries,
    s: Stmt,
    visitor_name: &str,
    errors: &mut Vec<(Pos, String)>,
    should_virtualize_functions: bool,
) -> (Stmt, Option<Expr>) {
    use aast::Stmt_::*;

    let unchanged_result = (s.clone(), None);

    let Stmt(pos, stmt_) = s;
    let pos_expr = exprpos(&pos);

    match stmt_ {
        Expr(e) => {
            let result = rewrite_expr(temps, *e, visitor_name, errors, should_virtualize_functions);
            (
                Stmt(pos, Expr(Box::new(result.virtual_expr))),
                Some(result.desugar_expr),
            )
        }
        Return(e) => match *e {
            // Source: MyDsl`return ...;`
            // Virtualized: return ...;
            // Desugared: $0v->visitReturn(new ExprPos(...), $0v->...)
            Some(e) => {
                let result =
                    rewrite_expr(temps, e, visitor_name, errors, should_virtualize_functions);
                let desugar_expr =
                    v_meth_call(et::VISIT_RETURN, vec![pos_expr, result.desugar_expr], &pos);
                let virtual_stmt = Stmt(pos, Return(Box::new(Some(result.virtual_expr))));
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

            let rewritten_cond = rewrite_expr(
                temps,
                cond_expr,
                visitor_name,
                errors,
                should_virtualize_functions,
            );
            let (virtual_then_stmts, desugar_then) = rewrite_stmts(
                temps,
                then_block.0,
                visitor_name,
                errors,
                should_virtualize_functions,
            );
            let (virtual_else_stmts, desugar_else) = rewrite_stmts(
                temps,
                else_block.0,
                visitor_name,
                errors,
                should_virtualize_functions,
            );

            let desugar_expr = v_meth_call(
                et::VISIT_IF,
                vec![
                    pos_expr,
                    rewritten_cond.desugar_expr,
                    vec_literal(desugar_then),
                    vec_literal(desugar_else),
                ],
                &pos,
            );
            let virtual_stmt = Stmt(
                pos,
                If(Box::new((
                    boolify(rewritten_cond.virtual_expr),
                    ast::Block(virtual_then_stmts),
                    ast::Block(virtual_else_stmts),
                ))),
            );
            (virtual_stmt, Some(desugar_expr))
        }
        // Source: MyDsl`while (...) {...}`
        // Virtualized: while (...->__bool()) {...}
        // Desugared: $0v->visitWhile(new ExprPos(...), $0v->..., vec[...])
        While(w) => {
            let (cond, body) = *w;

            let rewritten_cond = rewrite_expr(
                temps,
                cond,
                visitor_name,
                errors,
                should_virtualize_functions,
            );
            let (virtual_body_stmts, desugar_body) = rewrite_stmts(
                temps,
                body.0,
                visitor_name,
                errors,
                should_virtualize_functions,
            );

            let desugar_expr = v_meth_call(
                et::VISIT_WHILE,
                vec![
                    pos_expr,
                    rewritten_cond.desugar_expr,
                    vec_literal(desugar_body),
                ],
                &pos,
            );
            let virtual_stmt = Stmt(
                pos,
                While(Box::new((
                    boolify(rewritten_cond.virtual_expr),
                    ast::Block(virtual_body_stmts),
                ))),
            );
            (virtual_stmt, Some(desugar_expr))
        }
        // Source: MyDsl`for (...; ...; ...) {...}`
        // Virtualized: for (...; ...->__bool(); ...) {...}
        // Desugared: $0v->visitFor(new ExprPos(...), vec[...], ..., vec[...], vec[...])
        For(w) => {
            let (init, cond, incr, body) = *w;

            let (virtual_init_exprs, desugar_init_exprs) = rewrite_exprs(
                temps,
                init,
                visitor_name,
                errors,
                should_virtualize_functions,
            );

            let (virtual_cond_option, desugar_cond_expr) = match cond {
                Some(cond) => {
                    let rewritten_cond = rewrite_expr(
                        temps,
                        cond,
                        visitor_name,
                        errors,
                        should_virtualize_functions,
                    );
                    (
                        Some(boolify(rewritten_cond.virtual_expr)),
                        rewritten_cond.desugar_expr,
                    )
                }
                None => (None, null_literal(pos.clone())),
            };

            let (virtual_incr_exprs, desugar_incr_exprs) = rewrite_exprs(
                temps,
                incr,
                visitor_name,
                errors,
                should_virtualize_functions,
            );

            let (virtual_body_stmts, desugar_body) = rewrite_stmts(
                temps,
                body.0,
                visitor_name,
                errors,
                should_virtualize_functions,
            );

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
                    ast::Block(virtual_body_stmts),
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
            errors.push((
                pos,
                "Expression trees do not support `do while` loops. Consider using a `while` loop instead.".into(),
            ));
            unchanged_result
        }
        Switch(_) => {
            errors.push((
                pos,
                "Expression trees do not support `switch` statements. Consider using `if`/`else if`/`else` instead.".into(),
            ));
            unchanged_result
        }
        Foreach(_) => {
            errors.push((
                pos,
                "Expression trees do not support `foreach` loops. Consider using a `for` loop or a `while` loop instead.".into(),
            ));
            unchanged_result
        }
        _ => {
            errors.push((
                pos,
                "Expression trees do not support this statement syntax.".into(),
            ));
            unchanged_result
        }
    }
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
                user_attributes: Default::default(),
                visibility: None,
            }
        })
        .collect();

    let call_args = call_args
        .into_iter()
        .map(|e: Expr| -> (ParamKind, Expr) { (ParamKind::Pnormal, e) })
        .collect();

    let func_body = ast::FuncBody {
        fb_ast: ast::Block(stmts),
    };
    let fun_ = wrap_fun_(func_body, fun_params, pos.clone());
    let lambda_expr = Expr::new((), pos.clone(), Expr_::mk_lfun(fun_, vec![]));

    Expr::new(
        (),
        pos.clone(),
        Expr_::Call(Box::new(ast::CallExpr {
            func: lambda_expr,
            targs: vec![],
            args: call_args,
            unpacked_arg: None,
        })),
    )
}

/// Is this is a typechecker pseudo function like `hh_show` that
/// shouldn't be desugared?
fn is_typechecker_fun_name(name: &str) -> bool {
    strip_ns(name) == strip_ns(pseudo_functions::HH_SHOW)
        || strip_ns(name) == strip_ns(pseudo_functions::HH_EXPECT)
        || strip_ns(name) == strip_ns(pseudo_functions::HH_EXPECT_EQUIVALENT)
        || strip_ns(name) == strip_ns(pseudo_functions::HH_SHOW_ENV)
}

fn strip_ns(name: &str) -> &str {
    match name.chars().next() {
        Some('\\') => &name[1..],
        _ => name,
    }
}

fn _virtualize_call(e: Expr, pos: &Pos, should_virtualize_functions: bool) -> Expr {
    if should_virtualize_functions {
        meth_call(e, "__unwrap", vec![], pos)
    } else {
        e
    }
}

fn _virtualize_lambda(
    visitor_name: &str,
    e: Expr,
    pos: &Pos,
    should_virtualize_functions: bool,
) -> Expr {
    if should_virtualize_functions {
        static_meth_call(visitor_name, et::LAMBDA_TYPE, vec![e], pos)
    } else {
        e
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
    let functions_vec = vec_literal_with_pos(pos, function_vars);

    let static_method_vars = static_methods
        .iter()
        .enumerate()
        .map(|(i, expr)| temp_static_method_lvar(&expr.1, i))
        .collect();
    let static_method_vec = vec_literal_with_pos(pos, static_method_vars);

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
        pos,
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
