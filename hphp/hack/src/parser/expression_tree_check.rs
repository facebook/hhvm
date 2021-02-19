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

#[derive(Copy, Clone)]
enum Context {
    Normal,
    InExpressionTree,
    InSplice,
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
                if let Context::InExpressionTree = c {
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

        match &e.1 {
            ExpressionTree(et) => {
                let prev_ctx = *c;

                match *c {
                    Context::InExpressionTree => {
                        // Ban: Foo` Foo`1` `;
                        self.syntax_error_at(&e.0, "Expression trees may not be nested.");
                        return Ok(());
                    }
                    Context::InSplice => {
                        // Ban: Foo` ${ Foo`1` } `;
                        self.syntax_error_at(&e.0, "Splices cannot contain expression tree literals. Use a local variable.");
                        return Ok(());
                    }
                    Context::Normal => {}
                }

                // Ensure the context tracks whether we're in a backtick.
                *c = Context::InExpressionTree;

                // Only run the syntax check on the original syntax
                // from the user. Ignore the desugared expression.
                let res = &et.src_expr.accept(c, self);

                *c = prev_ctx;
                return *res;
            }
            ETSplice(e) => {
                if let Context::Normal | Context::InSplice = c {
                    self.syntax_error_at(
                        &e.0,
                        "Splice syntax ${...} can only occur inside expression trees Foo``...``.",
                    );
                    // Don't recurse, so we don't also produce syntax errors for child nodes.
                    return Ok(());
                }

                let prev_ctx = *c;
                *c = Context::InSplice;

                let res = e.accept(c, self);

                *c = prev_ctx;
                return res;
            }
            _ => {
                // If we're not in an expression tree or in a splice,
                // allow all syntax.
                if let Context::Normal | Context::InSplice = *c {
                    return e.recurse(c, self);
                }
            }
        }

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
                // Allow negation -$x (required for supporting negative literals -123)
                (Uop::Uminus, _) => None,
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
    }
}

pub fn check_program(program: &aast::Program<Pos, (), (), ()>) -> Vec<SyntaxError> {
    let mut checker = Checker { errors: vec![] };
    let mut context = Context::Normal;

    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
