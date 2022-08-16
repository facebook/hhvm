// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::MemoProvider;
use oxidized::aast;
use oxidized::ast;

struct Infer<'arena, 'decl> {
    _decl_provider: &'arena MemoProvider<'decl>,
}

impl<'arena, 'decl> Infer<'arena, 'decl> {
    fn type_expr(&self, expr: &ast::Expr) -> ast::Expr {
        let aast::Expr(_, _, exp) = expr;
        use aast::Expr_::*;

        match exp {
            Darray(_b) => {}
            Varray(_b) => {}
            Shape(_fields) => {}
            ValCollection(_b) => {}
            KeyValCollection(_b) => {}
            Null => {}
            This => {}
            True => {}
            False => {}
            Omitted => {}
            Id(_b) => {}
            Lvar(_b) => {}
            Dollardollar(_b) => {}
            Clone(_b) => {}
            ArrayGet(_b) => {}
            ObjGet(_b) => {}
            ClassGet(_b) => {}
            ClassConst(_b) => {}
            Call(_b) => {}
            FunctionPointer(_b) => {}
            Int(_s) => {}
            Float(_s) => {}
            String(_bstring) => {}
            String2(_v) => {}
            PrefixedString(_b) => {}
            Yield(_b) => {}
            Await(_b) => {}
            ReadonlyExpr(_b) => {}
            Tuple(_v) => {}
            List(_v) => {}
            Cast(_b) => {}
            Unop(_b) => {}
            Binop(_b) => {}
            Pipe(_b) => {}
            Eif(_b) => {}
            Is(_b) => {}
            As(_b) => {}
            Upcast(_b) => {}
            New(_b) => {}
            Efun(_b) => {}
            Lfun(_b) => {}
            Xml(_b) => {}
            Import(_b) => {}
            Collection(_b) => {}
            ExpressionTree(_b) => {}
            Lplaceholder(_b) => {}
            FunId(_b) => {}
            MethodId(_b) => {}
            MethodCaller(_b) => {}
            SmethodId(_b) => {}
            Pair(_b) => {}
            ETSplice(_b) => {}
            EnumClassLabel(_b) => {}
            Hole(_b) => {}
        }
        expr.clone()
    }

    fn type_stmt(&self, stmt: &ast::Stmt) -> ast::Stmt {
        let aast::Stmt(pos, st) = stmt;
        use aast::Stmt_::*;

        let new_stmt = match st {
            Fallthrough => Fallthrough,
            Expr(box expr) => {
                let new_expr = self.type_expr(expr);
                Expr(Box::new(new_expr))
            }
            Break => Break,
            Continue => Continue,
            Throw(box expr) => {
                let new_expr = self.type_expr(expr);
                Expr(Box::new(new_expr))
            }
            Return(box opt_expr) => match opt_expr {
                None => Return(Box::new(None)),
                Some(expr) => {
                    let new_expr = self.type_expr(expr);
                    Return(Box::new(Some(new_expr)))
                }
            },
            YieldBreak => YieldBreak,
            Awaitall(box (assigns, block)) => Awaitall(Box::new((assigns.clone(), block.clone()))),
            If(box (expr, stmt1, stmt2)) => {
                let new_expr = self.type_expr(expr);
                If(Box::new((new_expr, stmt1.clone(), stmt2.clone())))
            }
            Do(b) => Do(b.clone()),
            While(b) => While(b.clone()),
            Using(b) => Using(b.clone()),
            For(b) => For(b.clone()),
            Switch(b) => Switch(b.clone()),
            Foreach(b) => Foreach(b.clone()),
            Try(b) => Try(b.clone()),
            Noop => Noop,
            Block(b) => Block(b.clone()),
            Markup(b) => Markup(b.clone()),
            AssertEnv(b) => AssertEnv(b.clone()),
        };
        ast::Stmt(pos.clone(), new_stmt)
    }

    fn type_fun(&self, fun: &ast::Fun_) -> ast::Fun_ {
        ast::Fun_ {
            body: ast::FuncBody {
                fb_ast: fun.body.fb_ast.iter().map(|s| self.type_stmt(s)).collect(),
            },
            ..fun.clone()
        }
    }

    fn type_fun_def(&self, fun: &ast::FunDef) -> ast::FunDef {
        ast::FunDef {
            fun: self.type_fun(&fun.fun),
            ..fun.clone()
        }
    }

    fn type_class_(&self, class: &ast::Class_) -> ast::Class_ {
        class.clone()
    }

    fn type_typedef(&self, td: &ast::Typedef) -> ast::Typedef {
        td.clone()
    }

    fn type_gconst(&self, gc: &ast::Gconst) -> ast::Gconst {
        gc.clone()
    }

    fn type_def(&self, d: &ast::Def) -> ast::Def {
        use aast::Def::*;
        match d {
            Fun(box fd) => Fun(Box::new(self.type_fun_def(fd))),
            Class(box c_) => Class(Box::new(self.type_class_(c_))),
            Stmt(box stmt) => Stmt(Box::new(self.type_stmt(stmt))),
            Typedef(box td) => Typedef(Box::new(self.type_typedef(td))),
            Constant(box gconst) => Constant(Box::new(self.type_gconst(gconst))),
            Namespace(b) => Namespace(b.clone()),
            NamespaceUse(b) => NamespaceUse(b.clone()),
            SetNamespaceEnv(b) => SetNamespaceEnv(b.clone()),
            FileAttributes(b) => FileAttributes(b.clone()),
            Module(b) => Module(b.clone()),
            SetModule(b) => SetModule(b.clone()),
        }
    }
}

pub fn type_program(prog: &ast::Program, decl_provider: &'_ MemoProvider<'_>) -> ast::Program {
    let infer = Infer {
        _decl_provider: decl_provider,
    };
    ast::Program(
        prog.iter()
            .map(|def| infer.type_def(def))
            .collect::<Vec<_>>(),
    )
}
