use std::collections::HashMap;

use bstr::BString;
use itertools::Itertools;
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
use oxidized::ast::Afield;
use oxidized::ast::ClassId;
use oxidized::ast::ClassId_;
use oxidized::ast::EtSplice;
use oxidized::ast::Expr;
use oxidized::ast::Expr_;
use oxidized::ast::Hint_;
use oxidized::ast::LocalId;
use oxidized::ast::Sid;
use oxidized::ast::Stmt;
use oxidized::ast::Stmt_;
use oxidized::ast_defs;
use oxidized::ast_defs::*;
use oxidized::experimental_features::FeatureName;
use oxidized::local_id;
use oxidized::pos::Pos;
use parser_core_types::syntax_error;

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
pub fn desugar(
    Id(visitor_pos, visitor_name): &aast::ClassName,
    e: Expr,
    env: &Env<'_>,
    is_nested: bool,
) -> DesugarResult {
    let et_literal_pos = e.1.clone();

    let free_vars = if is_nested {
        // Only compute the free variables for nested expression trees.
        // Top level ones will just desugar to a virtual expression with
        // free variables and get an error from that
        LiveVars::new_from_expression(&e).used
    } else {
        HashMap::default()
    };

    let mut state = RewriteState {
        splices: vec![],
        global_function_pointers: vec![],
        static_method_pointers: vec![],
        errors: vec![],
        contains_spliced_await: false,
    };
    let rewritten_expr = state.rewrite_expr(e, visitor_name);

    let dollardollar_pos = rewrite_dollardollars(&mut state.splices);

    let splice_count = state.splices.len();
    let function_count = state.global_function_pointers.len();
    let static_method_count = state.static_method_pointers.len();

    // Wrap this in an Efun with appropriate variables for typing.
    // This enables us to report unbound variables correctly.
    let virtualized_expr = {
        let typing_fun_body = ast::FuncBody {
            fb_ast: ast::Block(vec![wrap_return(
                rewritten_expr.virtual_expr,
                &et_literal_pos,
            )]),
        };
        let mut typing_fun_ = wrap_fun_(false, typing_fun_body, vec![], et_literal_pos.clone());
        typing_fun_.ctxs = Some(aast::Contexts(
            et_literal_pos.clone(),
            vec![ast::Hint::new(
                et_literal_pos.clone(),
                Hint_::Happly(
                    Id(
                        et_literal_pos.clone(),
                        naming_special_names_rust::coeffects::DEFAULTS.to_string(),
                    ),
                    vec![],
                ),
            )],
        ));
        let mut use_ = vec![];
        let spliced_vars = (0..splice_count).map(|i| {
            ast::CaptureLid(
                (),
                ast::Lid(visitor_pos.clone(), (0, temp_splice_lvar_string(i))),
            )
        });
        use_.extend(spliced_vars);
        let function_pointer_vars = (0..function_count).map(|i| {
            ast::CaptureLid(
                (),
                ast::Lid(
                    visitor_pos.clone(),
                    (0, temp_function_pointer_lvar_string(i)),
                ),
            )
        });
        use_.extend(function_pointer_vars);
        let static_method_vars = (0..static_method_count).map(|i| {
            ast::CaptureLid(
                (),
                ast::Lid(visitor_pos.clone(), (0, temp_static_method_lvar_string(i))),
            )
        });
        use_.extend(static_method_vars);
        if is_nested {
            let fvs = free_vars
                .iter()
                .map(|(v, pos)| ast::CaptureLid((), ast::Lid(pos.clone(), v.clone())));
            use_.extend(fvs);
        }
        Expr::new(
            (),
            et_literal_pos.clone(),
            Expr_::mk_efun(aast::Efun {
                fun: typing_fun_,
                use_,
                closure_class_name: None,
            }),
        )
    };

    let metadata = maketree_metadata(
        visitor_pos,
        &state.splices,
        &state.global_function_pointers,
        &state.static_method_pointers,
        env.is_typechecker(),
        virtualized_expr,
    );

    // Make anonymous function of smart constructor calls
    let visitor_expr = wrap_return(rewritten_expr.desugar_expr, &et_literal_pos);
    let visitor_body = ast::FuncBody {
        fb_ast: ast::Block(vec![visitor_expr]),
    };
    let param = ast::FunParam {
        annotation: (),
        type_hint: ast::TypeHint(
            (),
            Some(aast::Hint::new(
                visitor_pos.clone(),
                Hint_::Happly(Id(visitor_pos.clone(), visitor_name.clone()), vec![]),
            )),
        ),
        pos: visitor_pos.clone(),
        name: visitor_variable(),
        info: ast::FunParamInfo::ParamRequired,
        callconv: ParamKind::Pnormal,
        readonly: None,
        splat: None,
        user_attributes: Default::default(),
        visibility: None,
    };
    let visitor_fun_ = wrap_fun_(false, visitor_body, vec![param], et_literal_pos.clone());
    let visitor_lambda = Expr::new(
        (),
        et_literal_pos.clone(),
        Expr_::mk_lfun(visitor_fun_, vec![]),
    );

    let spliced_await = env.in_async && state.contains_spliced_await;

    // Create assignment of the extracted expressions to temporary variables
    // `$0splice0 = spliced_expr0;`
    let splice_assignments: Vec<Stmt> = if spliced_await {
        create_temp_statement_parallel(&et_literal_pos, state.splices, temp_splice_lvar)
    } else {
        create_temp_statements(state.splices, temp_splice_lvar)
    };
    // `$0fp0 = foo<>;`
    let function_pointer_assignments: Vec<Stmt> =
        create_temp_statements(state.global_function_pointers, temp_function_pointer_lvar);
    // `$0sm0 = Foo::bar<>;`
    let static_method_assignments: Vec<Stmt> =
        create_temp_statements(state.static_method_pointers, temp_static_method_lvar);

    let mut function_pointers = vec![];
    function_pointers.extend(function_pointer_assignments);
    function_pointers.extend(static_method_assignments);
    if is_nested
        && !free_vars.is_empty()
        && !FeatureName::ExpressionTreeNestedBindings
            .can_use(env.parser_options, &env.active_experimental_features)
    {
        state.errors.push((
            et_literal_pos.clone(),
            format!(
                "{} Free variables: {}",
                syntax_error::cannot_use_feature(FeatureName::ExpressionTreeNestedBindings.into()),
                free_vars.keys().map(|x| x.1.clone()).join(", ")
            ),
        ))
    }
    let make_tree = static_meth_call(
        visitor_name,
        et::MAKE_TREE,
        vec![exprpos(&et_literal_pos), metadata, visitor_lambda],
        visitor_pos,
    );

    let runtime_expr = if splice_assignments.is_empty() && function_pointers.is_empty() {
        make_tree
    } else {
        let body = {
            let mut b = splice_assignments.clone();
            b.extend(function_pointers.clone());
            b.push(wrap_return(make_tree, &et_literal_pos));
            b
        };
        let lambda_args = match &dollardollar_pos {
            Some(pipe_pos) => vec![(
                (et::DOLLARDOLLAR_TMP_VAR.to_string(), pipe_pos.clone()),
                Expr::mk_lvar(pipe_pos, special_idents::DOLLAR_DOLLAR),
            )],
            _ => vec![],
        };

        immediately_invoked_lambda(spliced_await, &et_literal_pos, body, lambda_args)
    };

    let expr = Expr::new(
        (),
        et_literal_pos,
        Expr_::mk_expression_tree(ast::ExpressionTree {
            class: Id(visitor_pos.clone(), visitor_name.clone()),
            runtime_expr,
            free_vars: if is_nested {
                Some(
                    free_vars
                        .into_iter()
                        .map(|(id, pos)| aast::Lid(pos, id))
                        .collect(),
                )
            } else {
                None
            },
        }),
    );
    DesugarResult {
        expr,
        errors: state.errors,
    }
}

/// Convert `foo` to `return foo;`.
fn wrap_return(e: Expr, pos: &Pos) -> Stmt {
    Stmt::new(pos.clone(), Stmt_::mk_return(Some(e)))
}

/// Wrap a FuncBody into an anonymous Fun_
fn wrap_fun_(
    async_: bool,
    body: ast::FuncBody,
    params: Vec<ast::FunParam>,
    span: Pos,
) -> ast::Fun_ {
    ast::Fun_ {
        span,
        readonly_this: None,
        annotation: (),
        readonly_ret: None,
        ret: ast::TypeHint((), None),
        params,
        body,
        fun_kind: if async_ {
            ast::FunKind::FAsync
        } else {
            ast::FunKind::FSync
        },
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
                    e.2 = Expr_::mk_lvar(ast::Lid(
                        e.1.clone(),
                        local_id::make_unscoped(et::DOLLARDOLLAR_TMP_VAR),
                    ));
                    if self.pos.is_none() {
                        self.pos = Some(e.1.clone());
                    }
                }
                Ok(())
            }
            // Don't need to recurse into the new scopes of lambdas or expression trees
            Lfun(_) | Efun(_) | ExpressionTree(_) => Ok(()),
            // Only recurse into the left hand side of any pipe as the rhs has new $$
            Pipe(p) => {
                let x = &mut p.1;
                x.accept(env, self.object())
            }
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
        Expr_::mk_val_collection((pos.clone(), aast::VcKind::Vec), None, items),
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
        Expr_::mk_key_val_collection((pos.clone(), aast::KvcKind::Dict), None, fields),
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
fn build_args(args: Vec<Expr>) -> Vec<ast::Argument> {
    args.into_iter().map(ast::Argument::Anormal).collect()
}

/// Build `$v->meth_name(args)`.
fn v_meth_call(meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    let receiver = Expr::mk_lvar(pos, &visitor_variable());
    let meth = Expr::new(
        (),
        pos.clone(),
        Expr_::mk_id(ast::Id(pos.clone(), meth_name.into())),
    );

    let c = Expr_::mk_call(ast::CallExpr {
        func: Expr::new(
            (),
            pos.clone(),
            Expr_::mk_obj_get(
                receiver,
                meth,
                OgNullFlavor::OGNullthrows,
                ast::PropOrMethod::IsMethod,
            ),
        ),
        targs: vec![],
        args: build_args(args),
        unpacked_arg: None,
    });
    Expr::new((), pos.clone(), c)
}

fn meth_call(receiver: Expr, meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    let meth = Expr::new(
        (),
        pos.clone(),
        Expr_::mk_id(ast::Id(pos.clone(), meth_name.into())),
    );

    let c = Expr_::mk_call(ast::CallExpr {
        func: Expr::new(
            (),
            pos.clone(),
            Expr_::mk_obj_get(
                receiver,
                meth,
                OgNullFlavor::OGNullthrows,
                ast::PropOrMethod::IsMethod,
            ),
        ),
        targs: vec![],
        args: build_args(args),
        unpacked_arg: None,
    });
    Expr::new((), pos.clone(), c)
}

fn static_meth_call_with_meth_pos(
    classname: &str,
    meth_name: &str,
    meth_pos: &Pos,
    args: Vec<Expr>,
    pos: &Pos,
) -> Expr {
    let callee = Expr::new(
        (),
        meth_pos.clone(),
        Expr_::mk_class_const(
            // TODO: Refactor ClassId creation with new_obj
            ClassId(
                (),
                meth_pos.clone(),
                ClassId_::CIexpr(Expr::new(
                    (),
                    meth_pos.clone(),
                    Expr_::mk_id(Id(meth_pos.clone(), classname.to_string())),
                )),
            ),
            (meth_pos.clone(), meth_name.to_string()),
        ),
    );
    Expr::new(
        (),
        pos.clone(),
        Expr_::mk_call(ast::CallExpr {
            func: callee,
            targs: vec![],
            args: build_args(args),
            unpacked_arg: None,
        }),
    )
}

fn static_meth_call(classname: &str, meth_name: &str, args: Vec<Expr>, pos: &Pos) -> Expr {
    static_meth_call_with_meth_pos(classname, meth_name, pos, args, pos)
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

fn create_temp_statement_parallel(
    pos: &Pos,
    exprs: Vec<Expr>,
    mk_lvar: fn(&Pos, usize) -> Expr,
) -> Vec<Stmt> {
    if exprs.len() < 2 {
        return create_temp_statements(exprs, mk_lvar);
    }
    let lhss = exprs
        .iter()
        .enumerate()
        .map(|(i, expr)| (mk_lvar(&expr.1, i)))
        .collect();
    // assign a tuple to a list to ensure any awaits can execute concurrently.
    // We don't use a concurrent statement because that requires each rhs to have
    // an await, and that might not be the case here.
    vec![Stmt::new(
        pos.clone(),
        Stmt_::mk_expr(Expr::new(
            (),
            pos.clone(),
            Expr_::mk_assign(
                Expr::new((), pos.clone(), Expr_::List(lhss)),
                None,
                Expr::new((), pos.clone(), Expr_::Tuple(exprs)),
            ),
        )),
    )]
}

fn create_temp_statements(exprs: Vec<Expr>, mk_lvar: fn(&Pos, usize) -> Expr) -> Vec<Stmt> {
    exprs
        .into_iter()
        .enumerate()
        .map(|(i, expr)| {
            Stmt::new(
                expr.1.clone(),
                Stmt_::mk_expr(Expr::new(
                    (),
                    expr.1.clone(),
                    Expr_::mk_assign(mk_lvar(&expr.1, i), None, expr),
                )),
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
                    Expr_::mk_id(make_id(pos.clone(), "__FILE__")),
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

struct RewriteState {
    splices: Vec<Expr>,
    global_function_pointers: Vec<Expr>,
    static_method_pointers: Vec<Expr>,
    errors: Vec<(Pos, String)>,
    contains_spliced_await: bool,
}

impl RewriteState {
    /// Performs both the virtualization and the desugaring in tandem
    /// Also extracts the expressions that need to be assigned to temporaries
    /// Replaces the extracted splices, function pointers, and static method pointers
    /// with temporary variables
    fn rewrite_expr(&mut self, e: Expr, visitor_name: &str) -> RewriteResult {
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
                let desugar_expr = mk_visit_string(&pos, expr_);
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
                let rewritten_lhs = self.rewrite_expr(lhs, visitor_name);
                let rewritten_rhs = self.rewrite_expr(rhs, visitor_name);

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
                        self.errors.push((
                            pos.clone(),
                            "Expression trees do not support the exponent operator `**`.".into(),
                        ));
                        "__unsupported"
                    }
                    Bop::Eqeq | Bop::Diff => {
                        self.errors.push((
                            pos.clone(),
                            "Expression trees only support strict equality operators `===` and `!==`".into(),
                        ));
                        "__unsupported"
                    }
                    Bop::Cmp => {
                        self.errors.push((
                            pos.clone(),
                            "Expression trees do not support the spaceship operator `<=>`. Try comparison operators like `<` and `>=`".into(),
                        ));
                        "__unsupported"
                    }
                    Bop::QuestionQuestion => {
                        self.errors.push((
                            pos.clone(),
                            "Expression trees do not support the null coalesce operator `??`."
                                .into(),
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
            Assign(assign) => {
                let (lhs, bop, rhs) = *assign;
                let rewritten_lhs = self.rewrite_expr(lhs, visitor_name);
                let rewritten_rhs = self.rewrite_expr(rhs, visitor_name);

                if bop.is_some() {
                    self.errors.push((pos.clone(), "Expression trees do not support compound assignments. Try the long form style `$foo = $foo + $bar` instead.".into(),));
                };
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
                    Expr_::mk_assign(rewritten_lhs.virtual_expr, bop, rewritten_rhs.virtual_expr),
                );
                RewriteResult {
                    virtual_expr,
                    desugar_expr,
                }
            }
            // Source: MyDsl`!...`
            // Virtualized: ...->__exclamationMark(...)
            // Desugared: $0v->visitUnop(new ExprPos(...), ..., '__exclamationMark')
            Unop(unop) => {
                let (op, operand) = *unop;
                let rewritten_operand = self.rewrite_expr(operand, visitor_name);

                let op_str = match op {
                    // Allow boolean not operator !$x
                    Uop::Unot => "__exclamationMark",
                    // Allow negation -$x (required for supporting negative literals -123)
                    Uop::Uminus => "__negate",
                    // Allow bitwise complement
                    Uop::Utild => "__tilde",
                    // Currently not allowed operators
                    Uop::Uplus => {
                        self.errors.push((
                            pos.clone(),
                            "Expression trees do not support the unary plus operator.".into(),
                        ));
                        "__unsupported"
                    }
                    // Postfix ++
                    Uop::Upincr => "__postfixPlusPlus",
                    // Prefix ++
                    Uop::Uincr => {
                        self.errors.push((
                            pos.clone(),
                            "Expression trees only support postfix increment operator `$x++`."
                                .into(),
                        ));
                        "__unsupported"
                    }
                    // Postfix --
                    Uop::Updecr => "__postfixMinusMinus",
                    // Prefix --
                    Uop::Udecr => {
                        self.errors.push((
                            pos.clone(),
                            "Expression trees only support postfix decrement operator `$x--`."
                                .into(),
                        ));
                        "__unsupported"
                    }
                    Uop::Usilence => {
                        self.errors.push((
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

                let rewritten_e1 = self.rewrite_expr(e1, visitor_name);
                let rewritten_e2 = if let Some(e2) = e2o {
                    self.rewrite_expr(e2, visitor_name)
                } else {
                    self.errors.push((
                        pos.clone(),
                        "Unsupported expression tree syntax: Elvis operator".into(),
                    ));
                    unchanged_result
                };
                let rewritten_e3 = self.rewrite_expr(e3, visitor_name);

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
                    Expr_::mk_eif(
                        boolify(rewritten_e1.virtual_expr),
                        Some(rewritten_e2.virtual_expr),
                        rewritten_e3.virtual_expr,
                    ),
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
                    self.errors.push((
                        pos.clone(),
                        "Expression trees do not support variadic calls.".into(),
                    ));
                }
                if !targs.is_empty() {
                    self.errors.push((
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
                            Expr_::mk_call(ast::CallExpr {
                                func: recv,
                                targs,
                                args,
                                unpacked_arg: variadic,
                            }),
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
                        ast::Argument::Anormal(e) => args_without_inout.push(e),
                        ast::Argument::Ainout(_, Expr(_, p, _)) => self.errors.push((
                            p,
                            "Expression trees do not support `inout` function calls.".into(),
                        )),
                    }
                }

                // type check any special function calls
                let builtin_virtual_expr =
                    handle_reserved_call(self, visitor_name, &pos, &recv.2, &args_without_inout);

                let (virtual_args, desugar_args) =
                    self.rewrite_exprs(args_without_inout, visitor_name);

                match recv.2 {
                    // Source: MyDsl`foo()`
                    // Virtualized: (MyDsl::symbolType($0fpXX)->__unwrap())()
                    // Desugared: $0v->visitCall(new ExprPos(...), $0v->visitGlobalFunction(new ExprPos(...), $0fpXX), vec[])
                    Id(sid) => {
                        let len = self.global_function_pointers.len();
                        self.global_function_pointers.push(global_func_ptr(&sid));
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
                            Expr_::mk_call(ast::CallExpr {
                                func: _virtualize_call(
                                    static_meth_call(
                                        visitor_name,
                                        et::SYMBOL_TYPE,
                                        vec![temp_variable],
                                        &pos,
                                    ),
                                    &pos,
                                ),
                                targs: vec![],
                                args: build_args(virtual_args),
                                unpacked_arg: None,
                            }),
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
                                self.errors.push((
                                pos,
                                "Static method calls in expression trees require explicit class names.".into(),
                            ));
                                return unchanged_result;
                            }
                        } else {
                            self.errors.push((
                            pos,
                            "Expression trees only support function calls and static method calls on named classes.".into(),
                        ));
                            return unchanged_result;
                        };

                        let len = self.static_method_pointers.len();
                        self.static_method_pointers
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

                        let virtual_expr = match builtin_virtual_expr {
                            Some(built_in_virtual_expr) => built_in_virtual_expr,
                            // no builtin, fallback to default behavior
                            None => Expr(
                                (),
                                pos.clone(),
                                Expr_::mk_call(ast::CallExpr {
                                    func: _virtualize_call(
                                        static_meth_call(
                                            visitor_name,
                                            et::SYMBOL_TYPE,
                                            vec![temp_variable],
                                            &pos,
                                        ),
                                        &pos,
                                    ),
                                    targs: vec![],
                                    args: build_args(virtual_args),
                                    unpacked_arg: None,
                                }),
                            ),
                        };

                        RewriteResult {
                            virtual_expr,
                            desugar_expr,
                        }
                    }
                    // Source: MyDsl`...()`
                    // Virtualized: ...()
                    // Desugared: $0v->visitCall(new ExprPos(...), ..., vec[])
                    _ => {
                        let should_virtualize_call = match recv.2 {
                            // We don't virtualize calls on instance methods
                            ObjGet(ref og) if og.3 == ast::PropOrMethod::IsMethod => false,
                            _ => true,
                        };
                        let rewritten_recv =
                            self.rewrite_expr(Expr((), recv.1, recv.2), visitor_name);

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
                            Expr_::mk_call(ast::CallExpr {
                                func: if should_virtualize_call {
                                    _virtualize_call(rewritten_recv.virtual_expr, &pos)
                                } else {
                                    rewritten_recv.virtual_expr
                                },
                                targs: vec![],
                                args: build_args(virtual_args),
                                unpacked_arg: None,
                            }),
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
                        self.errors.push((
                            pos.clone(),
                            "Expression trees only support simple lambdas, without features like `async`, generators or capabilities."
                                .into(),
                        ));
                    }
                }

                let mut desugar_optional_params = Vec::with_capacity(fun_.params.len());
                let mut param_names = Vec::with_capacity(fun_.params.len());
                for param in fun_.params.iter_mut() {
                    match &param.info {
                        ast::FunParamInfo::ParamOptional(Some(expr)) => {
                            let rewritten_lambda = self.rewrite_expr(expr.clone(), visitor_name);
                            let desugar_visit_optional_param_expr = v_meth_call(
                                et::VISIT_OPTIONAL_PARAMETER,
                                vec![
                                    pos_expr.clone(),
                                    string_literal(param.pos.clone(), &param.name),
                                    rewritten_lambda.desugar_expr,
                                ],
                                &pos,
                            );
                            desugar_optional_params.push(desugar_visit_optional_param_expr);
                            param.info = aast::FunParamInfo::ParamOptional(Some(
                                rewritten_lambda.virtual_expr,
                            ));
                        }
                        _ => {
                            param_names.push(string_literal(param.pos.clone(), &param.name));
                        }
                    }
                }

                let body = std::mem::take(&mut fun_.body.fb_ast.0);

                let should_append_return = only_void_return(&body);

                let (mut virtual_body_stmts, desugar_body) = self.rewrite_stmts(body, visitor_name);
                if should_append_return {
                    virtual_body_stmts.push(Stmt(
                        pos.clone(),
                        aast::Stmt_::mk_return(Some(static_meth_call(
                            visitor_name,
                            et::VOID_TYPE,
                            vec![],
                            &pos,
                        ))),
                    ));
                }
                let mut exprs = vec![
                    pos_expr,
                    vec_literal(param_names),
                    vec_literal(desugar_body),
                ];
                if !desugar_optional_params.is_empty() {
                    exprs.push(vec_literal(desugar_optional_params));
                }
                let desugar_expr = v_meth_call(et::VISIT_LAMBDA, exprs, &pos);

                fun_.body.fb_ast = ast::Block(virtual_body_stmts);

                let virtual_expr = _virtualize_lambda(
                    visitor_name,
                    Expr((), pos.clone(), Expr_::mk_lfun(fun_, vec![])),
                    &pos,
                );

                RewriteResult {
                    virtual_expr,
                    desugar_expr,
                }
            }

            // Splices are lifted out and assigned to temporary variables.
            // Splices can be either normal:
            // Source: MyDsl`${ ... }` where there are no nested free variables in ETs in the splice
            // Lifted to $0splice = ${ ... };
            // Virtualized to ${ $0splice }
            // Desugared to $0v->splice(new ExprPos(...), '$0splice', $0splice)
            //
            // Splices can also be macro splices. In that case, we wrap with a lambda to delay evaluation
            // Source: MyDsl`${ ... }` where there are nested free variables (say $x and $y) in ETs in the splice
            // Lifted to $0splice = ${ () ==> ... };
            // Virtualized to ${ $0splice() } where the splice records $x and $y
            // Desugared to $0v->macro_splice(new ExprPos(...), '$0splice', $0splice, vec!['$x', '$y'])
            // Where there is an await inside of the splice, the lambda created is async.
            // Further, the desugared expression calls async_macro_splice, rather than macro_splice
            ETSplice(box aast::EtSplice {
                spliced_expr,
                contains_await,
                extract_client_type: _,
                macro_variables,
                temp_lid: _,
            }) => {
                let len = self.splices.len();
                let expr_pos = spliced_expr.1.clone();
                let is_macro = macro_variables.is_some();
                let temp_variable = temp_splice_lvar(&expr_pos, len);
                let temp_variable_string =
                    string_literal(expr_pos.clone(), &temp_splice_lvar_string(len));
                let desugar_expr = if is_macro {
                    let macro_var_exprs = macro_variables
                        .as_ref()
                        .unwrap()
                        .iter()
                        .map(|ast::Lid(pos, (_, s))| string_literal(pos.clone(), s))
                        .collect();
                    v_meth_call(
                        if contains_await {
                            et::ASYNC_MACRO_SPLICE
                        } else {
                            et::MACRO_SPLICE
                        },
                        vec![
                            pos_expr,
                            temp_variable_string,
                            temp_variable.clone(),
                            vec_literal(macro_var_exprs),
                        ],
                        &pos,
                    )
                } else {
                    v_meth_call(
                        et::SPLICE,
                        vec![pos_expr, temp_variable_string, temp_variable.clone()],
                        &pos,
                    )
                };
                let virtual_spliced_expr = if is_macro {
                    let e = Expr::new(
                        (),
                        pos.clone(),
                        Expr_::mk_call(aast::CallExpr {
                            func: temp_variable.clone(),
                            targs: vec![],
                            args: vec![],
                            unpacked_arg: None,
                        }),
                    );
                    if contains_await {
                        Expr::new((), pos.clone(), Expr_::mk_await(e))
                    } else {
                        e
                    }
                } else {
                    temp_variable.clone()
                };
                let temp_lid = temp_variable.2.as_lvar_into().unwrap().1;
                let virtual_expr = Expr(
                    (),
                    pos,
                    Expr_::mk_etsplice(aast::EtSplice {
                        spliced_expr: virtual_spliced_expr,
                        extract_client_type: true,
                        contains_await,
                        macro_variables: None,
                        temp_lid: temp_lid.clone(),
                    }),
                );
                // Compute the rhs of the assignment to the temporary
                let wrapped_expr = if !is_macro {
                    spliced_expr
                } else {
                    let fun_ = wrap_fun_(
                        contains_await,
                        ast::FuncBody {
                            fb_ast: ast::Block(vec![Stmt::new(
                                expr_pos.clone(),
                                Stmt_::mk_return(Some(spliced_expr)),
                            )]),
                        },
                        vec![],
                        expr_pos.clone(),
                    );
                    Expr::new((), expr_pos.clone(), Expr_::mk_lfun(fun_, vec![]))
                };
                let expr_ = Expr_::mk_etsplice(aast::EtSplice {
                    spliced_expr: wrapped_expr,
                    contains_await,
                    extract_client_type: false,
                    macro_variables,
                    temp_lid,
                });
                let expr = Expr::new((), expr_pos, expr_);
                self.splices.push(expr);

                self.contains_spliced_await |= contains_await;
                RewriteResult {
                    virtual_expr,
                    desugar_expr,
                }
            }
            // Source:
            //  MyDsl`(...)->foo` or
            //  MyDsl`(...)->foo(...)`
            // Virtualized to:
            //   `(...)->foo` or
            //   `(...)->foo(...)`
            // Desugared to:
            //   `$0v->visitPropertyAccess(new ExprPos(...), ..., 'foo')` or
            //   `$0v->visitInstanceMethod(new ExprPos(...), ..., 'foo')`
            ObjGet(og) => {
                let (e1, e2, null_flavor, is_prop_call) = *og;

                if null_flavor == OgNullFlavor::OGNullsafe {
                    self.errors.push((
                        pos.clone(),
                        "Expression Trees do not support nullsafe property or instance method access".into(),
                    ));
                }
                let rewritten_e1 = self.rewrite_expr(e1, visitor_name);

                let id = if let Id(id) = &e2.2 {
                    string_literal(id.0.clone(), &id.1)
                } else {
                    self.errors.push((
                        pos.clone(),
                        "Expression trees only support named property or instance method access."
                            .into(),
                    ));
                    e2.clone()
                };
                let desugar_expr = v_meth_call(
                    match is_prop_call {
                        PropOrMethod::IsProp => et::VISIT_PROPERTY_ACCESS,
                        PropOrMethod::IsMethod => et::VISIT_INSTANCE_METHOD,
                    },
                    vec![pos_expr, rewritten_e1.desugar_expr, id],
                    &pos,
                );

                let virtual_expr = Expr(
                    (),
                    pos,
                    Expr_::mk_obj_get(rewritten_e1.virtual_expr, e2, null_flavor, is_prop_call),
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
            //     nameof :foo,
            //     dict["my-attr" => $0v->visitString(...)],
            //     vec[
            //       $0v->visitString(..., "text ")],
            //       $0v->visitXhp(..., nameof :foo-child, ...),
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
                            let dict_key = Expr::new(
                                (),
                                attr_name_pos,
                                Expr_::String(BString::from(attr_name)),
                            );

                            let rewritten_attr_expr = self.rewrite_expr(xs.expr, visitor_name);
                            desugar_attrs.push((dict_key, rewritten_attr_expr.desugar_expr));
                            virtual_attrs.push(aast::XhpAttribute::XhpSimple(aast::XhpSimple {
                                expr: rewritten_attr_expr.virtual_expr,
                                ..xs
                            }))
                        }
                        aast::XhpAttribute::XhpSpread(e) => {
                            self.errors.push((
                                e.1,
                                "Expression trees do not support attribute spread syntax.".into(),
                            ));
                        }
                    }
                }

                let (virtual_children, desugar_children) =
                    self.rewrite_exprs(children, visitor_name);

                // Construct nameof :foo.
                let hint_pos = hint.0.clone();
                let hint_class = Expr_::mk_nameof(ClassId(
                    (),
                    hint_pos.clone(),
                    ClassId_::CIexpr(Expr::new(
                        (),
                        hint_pos.clone(),
                        Expr_::mk_id(ast_defs::Id(hint_pos.clone(), hint.1.clone())),
                    )),
                ));

                let virtual_expr = Expr(
                    (),
                    pos.clone(),
                    Expr_::mk_xml(hint, virtual_attrs, virtual_children),
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
            // Source: MyDsl`ClientMap {...}`
            // Virtualized: ClientMap::__make(...)
            // Desugared: $0v->visitKeyedCollection('ClientMap', ...)
            Collection(box (ast::Id(name_pos, name), targ, fields)) => {
                let mut rewritten_fields_virtual = vec![];
                let mut rewritten_fields_desugar = vec![];
                for kv in fields {
                    match kv {
                        Afield::AFvalue(v) => {
                            self.rewrite_expr(v, visitor_name);
                            self.errors.push((
                                pos.clone(),
                                "Collections in expression trees must have `key => value` entries"
                                    .into(),
                            ))
                        }
                        Afield::AFkvalue(k, v) => {
                            let pos = match Pos::merge(&k.1, &v.1) {
                                Ok(pos) => pos,
                                _ => k.1.clone(),
                            };
                            let k_rewr = self.rewrite_expr(k, visitor_name);
                            let v_rewr = self.rewrite_expr(v, visitor_name);
                            rewritten_fields_virtual.push(Expr::new(
                                (),
                                pos.clone(),
                                Expr_::Tuple(vec![k_rewr.virtual_expr, v_rewr.virtual_expr]),
                            ));
                            rewritten_fields_desugar.push(Expr::new(
                                (),
                                pos,
                                Expr_::Tuple(vec![k_rewr.desugar_expr, v_rewr.desugar_expr]),
                            ))
                        }
                    }
                }
                let mut args = vec![pos_expr, string_literal(name_pos.clone(), &name)];
                args.append(&mut rewritten_fields_desugar);
                let desugar_expr = v_meth_call(et::VISIT_KEYED_COLLECTION, args, &pos);
                let virtual_expr = static_meth_call_with_meth_pos(
                    &name,
                    et::MAKE_KEYED_COLLECTION_TYPE,
                    &name_pos,
                    rewritten_fields_virtual,
                    &pos,
                );
                match targ {
                    None => {}
                    Some(_) => self.errors.push((
                        pos.clone(),
                        "Collections in expression trees must not have explicit type hints".into(),
                    )),
                }
                RewriteResult {
                    virtual_expr,
                    desugar_expr,
                }
            }
            // Source: MyDsl`shape('key1' => value1, 'key2' => value2)`
            // Virtualized: shape('key1' => virtualized(value1), 'key2' => virtualized(value2))
            // Desugared: $0v->visitShape(pos, vec[tuple($0->visitString(pos, 'key1'), desugared(value1))])
            Shape(fields) => {
                let mut virtual_shape_fields = vec![];
                let mut desugar_shape_fields = vec![];
                for (shape_key, shape_value) in fields {
                    if let ShapeFieldName::SFlitStr((shape_key_pos, shape_key_name)) = &shape_key {
                        let value_expr = self.rewrite_expr(shape_value, visitor_name);
                        // we virtualize using the original Hack shape key expression...
                        virtual_shape_fields.push((shape_key.clone(), value_expr.virtual_expr));
                        desugar_shape_fields.push(Expr::new(
                            (),
                            shape_key_pos.clone(),
                            Expr_::Tuple(vec![
                                // ...however, we desuger the key into a string expression, to keep the key desugaring
                                // generic and more akin to other collections.
                                mk_visit_string(
                                    shape_key_pos,
                                    Expr_::String(shape_key_name.clone()),
                                ),
                                value_expr.desugar_expr,
                            ]),
                        ));
                    } else {
                        self.errors.push((
                            pos.clone(),
                            "Expression trees only support string literal shape field names."
                                .into(),
                        ));
                    }
                }
                let virtual_expr = static_meth_call(
                    visitor_name,
                    et::SHAPE_TYPE,
                    vec![Expr((), pos.clone(), Shape(virtual_shape_fields))],
                    &pos,
                );
                let desugar_expr = v_meth_call(
                    et::VISIT_SHAPE,
                    vec![pos_expr, vec_literal(desugar_shape_fields)],
                    &pos,
                );
                RewriteResult {
                    virtual_expr,
                    desugar_expr,
                }
            }
            ClassConst(_) => {
                self.errors.push((
                pos,
                "Expression trees do not support directly referencing class consts. Consider splicing values defined outside the scope of an Expression Tree using ${...}.".into(),
            ));
                unchanged_result
            }
            Efun(_) => {
                self.errors.push((
                pos,
                "Expression trees do not support PHP lambdas. Consider using Hack lambdas `() ==> {}` instead.".into(),
            ));
                unchanged_result
            }
            ExpressionTree(_) => {
                self.errors.push((
                    pos,
                    "Expression trees may not be used directly inside of other expression trees."
                        .into(),
                ));
                unchanged_result
            }
            Yield(_) => {
                self.errors
                    .push((pos, "`yield` is not supported in expression trees.".into()));
                unchanged_result
            }
            _ => {
                self.errors
                    .push((pos, "Unsupported expression tree syntax.".into()));
                unchanged_result
            }
        }
    }

    fn rewrite_exprs(&mut self, exprs: Vec<Expr>, visitor_name: &str) -> (Vec<Expr>, Vec<Expr>) {
        let mut virtual_results = Vec::with_capacity(exprs.len());
        let mut desugar_results = Vec::with_capacity(exprs.len());
        for expr in exprs {
            let rewritten_expr = self.rewrite_expr(expr, visitor_name);
            virtual_results.push(rewritten_expr.virtual_expr);
            desugar_results.push(rewritten_expr.desugar_expr);
        }
        (virtual_results, desugar_results)
    }

    fn rewrite_stmts(&mut self, stmts: Vec<Stmt>, visitor_name: &str) -> (Vec<Stmt>, Vec<Expr>) {
        let mut virtual_results = Vec::with_capacity(stmts.len());
        let mut desugar_results = Vec::with_capacity(stmts.len());
        for stmt in stmts {
            let (virtual_stmt, desugared_expr) = self.rewrite_stmt(stmt, visitor_name);
            virtual_results.push(virtual_stmt);
            if let Some(desugared_expr) = desugared_expr {
                desugar_results.push(desugared_expr);
            }
        }
        (virtual_results, desugar_results)
    }

    fn rewrite_stmt(&mut self, s: Stmt, visitor_name: &str) -> (Stmt, Option<Expr>) {
        use aast::Stmt_::*;

        let unchanged_result = (s.clone(), None);

        let Stmt(pos, stmt_) = s;
        let pos_expr = exprpos(&pos);

        match stmt_ {
            Expr(e) => {
                let result = self.rewrite_expr(*e, visitor_name);
                (
                    Stmt(pos, Stmt_::mk_expr(result.virtual_expr)),
                    Some(result.desugar_expr),
                )
            }
            Return(e) => match *e {
                // Source: MyDsl`return ...;`
                // Virtualized: return ...;
                // Desugared: $0v->visitReturn(new ExprPos(...), $0v->...)
                Some(e) => {
                    let result = self.rewrite_expr(e, visitor_name);
                    let desugar_expr =
                        v_meth_call(et::VISIT_RETURN, vec![pos_expr, result.desugar_expr], &pos);
                    let virtual_stmt = Stmt(pos, Stmt_::mk_return(Some(result.virtual_expr)));
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

                    let virtual_void_expr =
                        static_meth_call(visitor_name, et::VOID_TYPE, vec![], &pos);
                    let virtual_stmt = Stmt(pos, Stmt_::mk_return(Some(virtual_void_expr)));
                    (virtual_stmt, Some(desugar_expr))
                }
            },
            // Source: MyDsl`if (...) {...} else {...}`
            // Virtualized: if (...->__bool())) {...} else {...}
            // Desugared: $0v->visitIf(new ExprPos(...), $0v->..., vec[...], vec[...])
            If(if_stmt) => {
                let (cond_expr, then_block, else_block) = *if_stmt;

                let rewritten_cond = self.rewrite_expr(cond_expr, visitor_name);
                let (virtual_then_stmts, desugar_then) =
                    self.rewrite_stmts(then_block.0, visitor_name);
                let (virtual_else_stmts, desugar_else) =
                    self.rewrite_stmts(else_block.0, visitor_name);

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
                    Stmt_::mk_if(
                        boolify(rewritten_cond.virtual_expr),
                        ast::Block(virtual_then_stmts),
                        ast::Block(virtual_else_stmts),
                    ),
                );
                (virtual_stmt, Some(desugar_expr))
            }
            // Source: MyDsl`while (...) {...}`
            // Virtualized: while (...->__bool()) {...}
            // Desugared: $0v->visitWhile(new ExprPos(...), $0v->..., vec[...])
            While(w) => {
                let (cond, body) = *w;

                let rewritten_cond = self.rewrite_expr(cond, visitor_name);
                let (virtual_body_stmts, desugar_body) = self.rewrite_stmts(body.0, visitor_name);

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
                    Stmt_::mk_while(
                        boolify(rewritten_cond.virtual_expr),
                        ast::Block(virtual_body_stmts),
                    ),
                );
                (virtual_stmt, Some(desugar_expr))
            }
            // Source: MyDsl`for (...; ...; ...) {...}`
            // Virtualized: for (...; ...->__bool(); ...) {...}
            // Desugared: $0v->visitFor(new ExprPos(...), vec[...], ..., vec[...], vec[...])
            For(w) => {
                let (init, cond, incr, body) = *w;

                let (virtual_init_exprs, desugar_init_exprs) =
                    self.rewrite_exprs(init, visitor_name);

                let (virtual_cond_option, desugar_cond_expr) = match cond {
                    Some(cond) => {
                        let rewritten_cond = self.rewrite_expr(cond, visitor_name);
                        (
                            Some(boolify(rewritten_cond.virtual_expr)),
                            rewritten_cond.desugar_expr,
                        )
                    }
                    None => (None, null_literal(pos.clone())),
                };

                let (virtual_incr_exprs, desugar_incr_exprs) =
                    self.rewrite_exprs(incr, visitor_name);

                let (virtual_body_stmts, desugar_body) = self.rewrite_stmts(body.0, visitor_name);

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
                    Stmt_::mk_for(
                        virtual_init_exprs,
                        virtual_cond_option,
                        virtual_incr_exprs,
                        ast::Block(virtual_body_stmts),
                    ),
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
                self.errors.push((
                pos,
                "Expression trees do not support `do while` loops. Consider using a `while` loop instead.".into(),
            ));
                unchanged_result
            }
            Switch(_) => {
                self.errors.push((
                pos,
                "Expression trees do not support `switch` statements. Consider using `if`/`else if`/`else` instead.".into(),
            ));
                unchanged_result
            }
            Foreach(_) => {
                self.errors.push((
                pos,
                "Expression trees do not support `foreach` loops. Consider using a `for` loop or a `while` loop instead.".into(),
            ));
                unchanged_result
            }
            _ => {
                self.errors.push((
                    pos,
                    "Expression trees do not support this statement syntax.".into(),
                ));
                unchanged_result
            }
        }
    }
}

fn immediately_invoked_lambda(
    async_: bool,
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
                pos,
                name,
                info: ast::FunParamInfo::ParamRequired,
                callconv: ParamKind::Pnormal,
                readonly: None,
                splat: None,
                user_attributes: Default::default(),
                visibility: None,
            }
        })
        .collect();

    let call_args = call_args
        .into_iter()
        .map(|e: Expr| -> ast::Argument { ast::Argument::Anormal(e) })
        .collect();

    let func_body = ast::FuncBody {
        fb_ast: ast::Block(stmts),
    };
    let fun_ = wrap_fun_(async_, func_body, fun_params, pos.clone());
    let lambda_expr = Expr::new((), pos.clone(), Expr_::mk_lfun(fun_, vec![]));

    let call = Expr::new(
        (),
        pos.clone(),
        Expr_::mk_call(ast::CallExpr {
            func: lambda_expr,
            targs: vec![],
            args: call_args,
            unpacked_arg: None,
        }),
    );
    if async_ {
        Expr::new((), pos.clone(), Expr_::mk_await(call))
    } else {
        call
    }
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

fn _virtualize_call(e: Expr, pos: &Pos) -> Expr {
    meth_call(e, "__unwrap", vec![], pos)
}

fn _virtualize_lambda(visitor_name: &str, e: Expr, pos: &Pos) -> Expr {
    static_meth_call(visitor_name, et::LAMBDA_TYPE, vec![e], pos)
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
    is_typecheck: bool,
    virtual_expr: Expr,
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
    let mut fields = vec![
        ("splices", splices_dict),
        ("functions", functions_vec),
        ("static_methods", static_method_vec),
    ];
    if is_typecheck {
        fields.push(("type", virtual_expr))
    }
    shape_literal(pos, fields)
}

fn global_func_ptr(sid: &Sid) -> Expr {
    let pos = sid.0.clone();
    Expr::new(
        (),
        pos,
        Expr_::mk_function_pointer(ast::FunctionPtrId::FPId(sid.clone()), vec![]),
    )
}

fn static_meth_ptr(pos: &Pos, cid: &ClassId, meth: &Pstring) -> Expr {
    Expr::new(
        (),
        pos.clone(),
        Expr_::mk_function_pointer(
            aast::FunctionPtrId::FPClassConst(cid.clone(), meth.clone()),
            vec![],
        ),
    )
}

fn mk_visit_string(pos: &Pos, str: Expr_) -> Expr {
    v_meth_call(
        et::VISIT_STRING,
        vec![exprpos(pos), Expr((), pos.clone(), str)],
        pos,
    )
}

// Live variable calculation, only applying to what is allowed in expression trees
// Tests are in desugar_expression_tree_tests.rs
#[derive(PartialEq, Debug)]
pub struct LiveVars {
    // The variables that might be used in a block, not following an assignment
    // to that variable in the block
    pub used: HashMap<LocalId, Pos>,
    // The variables that are definitely assigned in a block
    pub assigned: HashMap<LocalId, Pos>,
}

impl LiveVars {
    // Compute the live variables at the entry to a list of statements,
    // assuming nothing is live after
    pub fn new_from_statement(stmts: &[Stmt]) -> Self {
        let mut lvs = LiveVars {
            used: HashMap::default(),
            assigned: HashMap::default(),
        };
        lvs.update_for_stmts(stmts);
        lvs
    }

    // Compute the live variables right before the given expression,
    // assuming that nothing is live after
    pub fn new_from_expression(expr: &Expr) -> Self {
        let mut lvs = LiveVars {
            used: HashMap::default(),
            assigned: HashMap::default(),
        };
        lvs.visit_expr(&mut (), expr).unwrap();
        lvs
    }

    // Update self with the live variable information for the stmts preceding
    // it in control-flow order
    fn update_for_stmts(&mut self, stmts: &[Stmt]) {
        for Stmt(_pos, stmt) in stmts.iter().rev() {
            match stmt {
                Stmt_::Expr(box e) | Stmt_::Return(box Some(e)) => {
                    self.visit_expr(&mut (), e).unwrap();
                }
                Stmt_::If(box (cond, ast::Block(then), ast::Block(els))) => {
                    let then_lvs = Self::new_from_statement(then);
                    let els_lvs = Self::new_from_statement(els);
                    let combined_lvs = then_lvs.merge(els_lvs);
                    self.update_for_more_live_vars(combined_lvs);
                    self.visit_expr(&mut (), cond).unwrap();
                }
                Stmt_::While(box (test, body)) => {
                    // We don't need to iterate to a fixed point, because we are
                    // only looking at the used variables on entry to the loop,
                    // rather than trying to use them at program points inside
                    // the loop. Any live variable at the start of the loop
                    // either comes from after the loop and it's not definitely
                    // assigned in the body, or it's used in the body, in which
                    // case it's used in the first iteration.

                    let mut body_lvs = Self::new_from_statement(body);
                    // The assigned vars don't matter because we might not enter the body
                    body_lvs.assigned = HashMap::default();
                    self.update_for_more_live_vars(body_lvs);
                    // We relay on there being no assignments in the expr
                    self.visit_expr(&mut (), test).unwrap();
                }
                Stmt_::For(box (init, test, inc, body)) => {
                    let mut body_lvs = LiveVars {
                        used: HashMap::default(),
                        assigned: HashMap::default(),
                    };
                    for e in inc.iter().rev() {
                        body_lvs.visit_expr(&mut (), e).unwrap();
                    }
                    body_lvs.update_for_stmts(body);
                    body_lvs.assigned = HashMap::default();
                    self.update_for_more_live_vars(body_lvs);
                    for e in test.iter().rev() {
                        self.visit_expr(&mut (), e).unwrap();
                    }
                    for e in init.iter().rev() {
                        self.visit_expr(&mut (), e).unwrap();
                    }
                }
                aast::Stmt_::Noop
                | aast::Stmt_::Fallthrough
                | aast::Stmt_::Break
                | aast::Stmt_::Continue
                | aast::Stmt_::Throw(_)
                | aast::Stmt_::Return(_)
                | aast::Stmt_::YieldBreak
                | aast::Stmt_::Awaitall(_)
                | aast::Stmt_::Concurrent(_)
                | aast::Stmt_::Do(_)
                | aast::Stmt_::Using(_)
                | aast::Stmt_::Switch(_)
                | aast::Stmt_::Match(_)
                | aast::Stmt_::Foreach(_)
                | aast::Stmt_::Try(_)
                | aast::Stmt_::DeclareLocal(_)
                | aast::Stmt_::Block(_)
                | aast::Stmt_::Markup(_) => {}
            }
        }
    }

    // Update self with the live variable info in update, assuming that self
    // follows update in the control flow
    fn update_for_more_live_vars(&mut self, update: Self) {
        for (v, pos) in update.assigned {
            self.used.remove(&v);
            self.assigned.insert(v, pos);
        }
        for (v, pos) in update.used {
            self.used.insert(v, pos);
        }
    }

    // Compute new live variable information where self and lvs come from two
    // branches of a control-flow split
    fn merge(self, lvs: Self) -> Self {
        let assigned = self
            .assigned
            .into_iter()
            .filter(|(v, _)| lvs.assigned.contains_key(v))
            .collect();
        let mut used = self.used;
        for (v, pos) in lvs.used {
            used.insert(v, pos);
        }
        LiveVars { assigned, used }
    }

    // Add an addignment of Lid to the live variable information. It is removed
    // from used, since the subsequent use will now see the assignment.
    fn add_assign(&mut self, lid: &ast::Lid) {
        if lid.as_local_id().1 != "$this" {
            let loc = lid.as_local_id();
            self.used.remove(loc);
            self.assigned.insert(loc.clone(), lid.0.clone());
        }
    }

    fn add_use(&mut self, lid: &ast::Lid) {
        if lid.as_local_id().1 != "$this" {
            self.used.insert(lid.as_local_id().clone(), lid.0.clone());
        }
    }

    // Update the live variable info in self to reflect the assignment to an
    // lvalue
    fn visit_lvalue(&mut self, env: &mut (), Expr(_, _, e): &Expr) {
        match e {
            Expr_::List(lv) => lv.iter().for_each(|e| self.visit_lvalue(env, e)),
            Expr_::Lvar(box lid) => self.add_assign(lid),
            _ => {
                self.visit_expr_(env, e).unwrap();
            }
        }
    }
}

// Visit an expression, updating the live variable information assumed to follow
// the expression in control-flow order
impl<'ast> Visitor<'ast> for LiveVars {
    type Params = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
        self
    }

    // We won't visit lids in lvalues because assignments get treated specially
    // in visit_expr_ (and the other lvalue position (foreach) isn't allowed in
    // ETs)
    fn visit_lid(&mut self, _env: &mut (), lid: &ast::Lid) -> Result<(), ()> {
        self.add_use(lid);
        Ok(())
    }

    fn visit_et_splice(&mut self, _env: &mut (), e: &ast::EtSplice) -> Result<(), ()> {
        if let Some(vars) = &e.macro_variables {
            for v in vars {
                self.add_use(v);
            }
        }
        Ok(())
    }

    fn visit_expr_(&mut self, env: &mut (), e: &Expr_) -> Result<(), ()> {
        match e {
            Expr_::Assign(box (lhs, bop, rhs)) => {
                if bop.is_none() {
                    self.visit_lvalue(env, lhs);
                    rhs.recurse(env, self.object())
                } else {
                    lhs.recurse(env, self.object())?;
                    rhs.recurse(env, self.object())?;
                    self.visit_lvalue(env, lhs);
                    Ok(())
                }
            }
            Expr_::ExpressionTree(_) => Ok(()),
            Expr_::ETSplice(box EtSplice {
                macro_variables, ..
            }) => {
                if let Some(vars) = macro_variables {
                    for v in vars {
                        self.add_use(v);
                    }
                }
                Ok(())
            }
            Expr_::Lfun(box (f, _idl)) => {
                // functions create a new scope
                let ast::Block(stmts) = &f.body.fb_ast;
                let mut lv = LiveVars::new_from_statement(stmts);
                for p in &f.params {
                    lv.used.remove(&local_id::make_unscoped(&p.name));
                }
                for p in &f.params {
                    if let ast::FunParamInfo::ParamOptional(Some(e)) = &p.info {
                        lv.visit_expr(&mut (), e).unwrap();
                    }
                }
                for (v, pos) in lv.used {
                    self.used.insert(v, pos);
                }
                Ok(())
            }
            _ => e.recurse(env, self.object()),
        }
    }
}

// There is a set of static functions that have special meaning in the
// typechecking process.
// If this function returns something, the returned expression will be used
// as virtualization, instead of the call that would otherwise be generated
fn handle_reserved_call(
    state: &mut RewriteState,
    visitor_name: &str,
    pos: &Pos,
    receiver: &Expr_,
    args: &[Expr],
) -> Option<Expr> {
    match receiver {
        aast::Expr_::ClassConst(cc) => {
            if let box (
                ClassId(_, _, ClassId_::CIexpr(Expr(_, _, aast::Expr_::Id(clz_box)))),
                (_, method),
            ) = cc
            {
                if clz_box.1 == visitor_name && method == et::BUILTIN_SHAPE_AT {
                    return handle_shapes_at(state, visitor_name, pos, args);
                }
                if clz_box.1 == visitor_name && method == et::BUILTIN_SHAPE_PUT {
                    return handle_shapes_put(state, visitor_name, pos, args);
                }
            }
            None
        }
        _ => None,
    }
}

/**
 * During virtualization, handle_shapes_at unwraps the DSL Shape type into a Hack shape tape,
 * and virtualizes the function call into a call from DSL::shapeAt into Shapes::at, to perform the type-checking.
 *
 * Note that this isn't used for desugaring, so in the desugared code an actual method call to DSL::shapeAt is emitted.
 */
fn handle_shapes_at(
    state: &mut RewriteState,
    visitor_name: &str,
    pos: &Pos,
    args: &[Expr],
) -> Option<Expr> {
    let mut args = args.to_vec();
    if !args.is_empty() {
        // Nit: we are actually only interested in the shape's virtual_expr
        // so we unwrap the first argument here.
        let shape = state.rewrite_expr(args[0].clone(), visitor_name);
        args[0] = _virtualize_call(shape.virtual_expr, pos)
    }

    Some(static_meth_call("Shapes", "at", args.to_vec(), pos))
}

fn handle_shapes_put(
    state: &mut RewriteState,
    visitor_name: &str,
    pos: &Pos,
    args: &[Expr],
) -> Option<Expr> {
    let mut args = args.to_vec();
    if args.len() > 2 {
        let shape = state.rewrite_expr(args[0].clone(), visitor_name);
        let value = state.rewrite_expr(args[2].clone(), visitor_name);
        args[0] = _virtualize_call(shape.virtual_expr, pos);
        args[2] = value.virtual_expr;
    }

    Some(static_meth_call(
        visitor_name,
        et::SHAPE_TYPE,
        vec![static_meth_call("Shapes", "put", args, pos)],
        pos,
    ))
}
