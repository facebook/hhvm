// Copyright (c) 2020, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{
    aast,
    aast_visitor::{visit, AstParams, Node, Visitor},
    ast_defs::*,
    pos::Pos,
};
use parser_core_types::syntax_error::SyntaxError;

struct Context {
    in_expression_tree: bool,
}

struct Checker {
    errors: Vec<SyntaxError>,
}

impl Checker {
    fn syntax_error_at(&mut self, p: &Pos, msg: &'static str) {
        let (start_offset, end_offset) = p.info_raw();
        self.errors
            .push(SyntaxError::make(start_offset, end_offset, msg.into()));
    }
}

impl<'ast> Visitor<'ast> for Checker {
    type P = AstParams<Context, ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_stmt(
        &mut self,
        c: &mut Context,
        s: &'ast aast::Stmt<Pos, (), (), ()>,
    ) -> Result<(), ()> {
        use aast::Stmt_::*;

        match &s.1 {
            Expr(_) => {}
            Return(_) => {}
            If(_) => {}
            While(_) => {}
            For(_) => {}
            Break => {}
            Continue => {}
            // Primarily used for if without else, but also used for standalone ;.
            Noop => {}
            // Ban any other statement syntax inside expression trees.
            _ => {
                if c.in_expression_tree {
                    self.syntax_error_at(
                        &s.0,
                        "Expression trees do not support this statement syntax.",
                    );
                    // Don't recurse, so we don't also produce syntax errors for child nodes.
                    return Ok(());
                }
            }
        }

        s.recurse(c, self)
    }

    fn visit_expr(&mut self, c: &mut Context, e: &aast::Expr<Pos, (), (), ()>) -> Result<(), ()> {
        use aast::Expr_::*;

        // Ensure the context tracks whether we're in a backtick.
        match &e.1 {
            ExpressionTree(et) => {
                c.in_expression_tree = true;

                // Only run the syntax check on the original syntax
                // from the user. Ignore the desugared expression.
                let res = &et.src_expr.accept(c, self);

                c.in_expression_tree = false;
                return *res;
            }
            ETSplice(e) => {
                let previous_state = c.in_expression_tree;
                c.in_expression_tree = false;
                let res = e.accept(c, self);
                c.in_expression_tree = previous_state;
                return res;
            }
            _ => {
                // If we're not in a backtick, all syntax is allowed.
                if !c.in_expression_tree {
                    return e.recurse(c, self);
                }
            }
        }

        if c.in_expression_tree {
            let syntax_err_msg = match &e.1 {
                // Allow integer, float, string, boolean and null literals.
                Int(_) => None,
                Float(_) => None,
                String(_) => None,
                True | False | Null => None,
                // Allow local variables $foo.
                Lvar(_) => None,
                Binop(bop) => match bop.0 {
                    // Allow arithmetic operators
                    Bop::Plus | Bop::Minus | Bop::Star | Bop::Slash => None,
                    // Allow $x = 1, but not $x += 1.
                    Bop::Eq(None) => None,
                    // Allow boolean &&, || operators
                    Bop::Ampamp | Bop::Barbar => None,
                    // Allow comparison operators
                    Bop::Lt | Bop::Lte | Bop::Gt | Bop::Gte | Bop::Eqeqeq | Bop::Diff2 => None,
                    _ => Some(
                        "Expression trees only support comparison (`<`, `===` etc) and basic arithmetic operators (`+` etc).",
                    ),
                },
                Unop(uop) => match **uop {
                    // Allow boolean not operator !$x
                    (Uop::Unot, _) => None,
                    _ => Some("Expression trees do not support this operator."),
                },
                // Allow ternary _ ? _ : _, but not Elvis operator _ ?: _
                Eif(eif) if eif.1.is_some() => None,
                // Allow simple function calls.
                Call(call) => match &**call {
                    // Ban variadic calls foo(...$x);
                    (_, _, _, Some(_)) => Some("Expression trees do not support variadic calls."),
                    // Ban generic type arguments foo<X, Y>();
                    (_, targs, _, _) if !targs.is_empty() => {
                        Some("Expression trees do not support function calls with generics.")
                    }
                    (recv, _targs, args, _variadic) => {
                        // Only allow direct function calls, so allow
                        // foo(), but don't allow (foo())().
                        match &recv.1 {
                            Id(_) => {
                                // Recurse on the arguments manually,
                                // so we don't end up visiting the
                                // Id() node. This is awkward because
                                // Id nodes can represent global
                                // constants or function names.
                                args.accept(c, self)?;
                                return Ok(());
                            }
                            ClassConst(cc) => match (cc.0).1 {
                                aast::ClassId_::CIexpr(aast::Expr(_, Id(ref id))) => {
                                    if id.1 == naming_special_names_rust::classes::PARENT
                                        || id.1 == naming_special_names_rust::classes::SELF
                                        || id.1 == naming_special_names_rust::classes::STATIC
                                    {
                                        Some(
                                            "Static method calls in expression trees require explicit class names.",
                                        )
                                    } else {
                                        args.accept(c, self)?;
                                        return Ok(());
                                    }
                                }
                                _ => Some(
                                    "Expression trees only support function calls and static method calls on named classes.",
                                ),
                            },
                            _ => {
                                // Recurse on the callee and only allow calls to valid expressions
                                recv.accept(c, self)?;
                                args.accept(c, self)?;
                                return Ok(());
                            }
                        }
                    }
                },
                // Allow lambdas () ==> { ... } but not PHP-style function() { ... }
                Lfun(lf) => {
                    // Don't allow parameters with default values.
                    for param in &lf.0.params {
                        if param.expr.is_some() {
                            self.syntax_error_at(
                                &param.pos,
                                "Expression trees do not support parameters with default values.",
                            );
                            return Ok(());
                        }
                    }
                    None
                }
                // Ban all other expression syntax possibilities.
                _ => Some("Unsupported syntax for expression trees."),
            };

            match syntax_err_msg {
                None => e.recurse(c, self),
                Some(msg) => {
                    self.syntax_error_at(&e.0, msg);
                    // Don't recurse, so we don't also produce syntax errors for child nodes.
                    Ok(())
                }
            }
        } else {
            e.recurse(c, self)
        }
    }
}

pub fn check_program(program: &aast::Program<Pos, (), (), ()>) -> Vec<SyntaxError> {
    let mut checker = Checker { errors: vec![] };
    let mut context = Context {
        in_expression_tree: false,
    };

    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
