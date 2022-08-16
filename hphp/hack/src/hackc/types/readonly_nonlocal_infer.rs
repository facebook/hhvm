// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::DeclProvider;
use decl_provider::Error;
use decl_provider::MemoProvider;
use oxidized::aast;
use oxidized::ast;
use oxidized::ast_defs::Pos;

struct Infer<'arena, 'decl> {
    decl_provider: &'arena MemoProvider<'decl>,
}

macro_rules! box_tup {
    ($($e:expr),* $(,)?) => {
        Box::new(($($e,)*))
    }
}

impl<'arena, 'decl> Infer<'arena, 'decl> {
    fn infer_expr(&self, expr: &ast::Expr, in_readonly: bool) -> ast::Expr {
        let aast::Expr(ex, pos, exp) = expr;
        use aast::Expr_::*;

        let exp = match exp {
            Darray(box (ty_args_opt, kvs)) => {
                let kvs = kvs
                    .iter()
                    .map(|(k, v)| {
                        let k = self.infer_expr(k, false);
                        let v = self.infer_expr(v, false);
                        (k, v)
                    })
                    .collect();
                Darray(box_tup!(ty_args_opt.clone(), kvs))
            }
            Varray(box (ty_args_opt, es)) => {
                let es = self.infer_exprs(es, false);
                Varray(box_tup!(ty_args_opt.clone(), es))
            }
            Shape(fields) => {
                let fields = fields
                    .iter()
                    .map(|(name, e)| (name.clone(), self.infer_expr(e, false)))
                    .collect();
                Shape(fields)
            }
            ValCollection(box (vc_kind, ty_var_opt, es)) => {
                let es = self.infer_exprs(es, false);
                ValCollection(box_tup!(vc_kind.clone(), ty_var_opt.clone(), es))
            }
            KeyValCollection(box (kvc_kind, ty_var_opt, fields)) => {
                let fields = fields
                    .iter()
                    .map(|ast::Field(e1, e2)| {
                        ast::Field(self.infer_expr(e1, false), self.infer_expr(e2, false))
                    })
                    .collect();
                KeyValCollection(box_tup!(kvc_kind.clone(), ty_var_opt.clone(), fields))
            }
            Null => exp.clone(),
            This => exp.clone(),
            True => exp.clone(),
            False => exp.clone(),
            Omitted => exp.clone(),
            Id(_b) => exp.clone(),
            Lvar(_b) => exp.clone(),
            Dollardollar(_b) => exp.clone(),
            Clone(box e) => Clone(Box::new(self.infer_expr(e, false))),
            ArrayGet(box (arr, index_opt)) => {
                let arr = self.infer_expr(arr, false);
                let index_opt = index_opt
                    .as_ref()
                    .map(|index| self.infer_expr(index, false));
                ArrayGet(box_tup!(arr, index_opt))
            }
            ObjGet(box (e1, e2, og_null_flavor, prop_or_meth)) => {
                let e1 = self.infer_expr(e1, false);
                let e2 = self.infer_expr(e2, false);
                ObjGet(box_tup!(
                    e1,
                    e2,
                    og_null_flavor.clone(),
                    prop_or_meth.clone()
                ))
            }
            ClassGet(box (class_id, get_expr, prop_or_meth)) => {
                use aast::ClassGetExpr;
                let get_expr = match get_expr {
                    ClassGetExpr::CGstring(_) => get_expr.clone(),
                    ClassGetExpr::CGexpr(e) => ClassGetExpr::CGexpr(self.infer_expr(e, false)),
                };
                ClassGet(box_tup!(class_id.clone(), get_expr, prop_or_meth.clone()))
            }
            ClassConst(_b) => exp.clone(),
            Call(box (e1, ty_args, params, expr2_opt)) => {
                let e1 = self.infer_expr(e1, false);
                let should_wrap = !in_readonly
                    && match e1 {
                        aast::Expr(pos, _, Id(box ref id)) => {
                            let name = id.name();
                            match self.decl_provider.func_decl(name) {
                                Ok(func_decl) => {
                                    use oxidized::typing_defs_flags::FunTypeFlags;
                                    use oxidized_by_ref::typing_defs_core::Ty_::*;
                                    match func_decl.type_.1 {
                                        Tfun(ft) => {
                                            ft.flags.contains(FunTypeFlags::RETURNS_READONLY)
                                        }
                                        _ => false,
                                    }
                                }
                                Err(Error::NotFound) => false,
                                Err(err @ Error::Bincode(_)) => {
                                    panic!(
                                        "Internal error when attempting to read the type of callable '{name}' at {:?}. {}",
                                        pos, err
                                    )
                                }
                            }
                        }
                        _ => false,
                    };
                let params = params
                    .iter()
                    .map(|(kind, e)| (kind.clone(), self.infer_expr(e, false)))
                    .collect();
                let expr2_opt = expr2_opt
                    .as_ref()
                    .map(|expr_2| self.infer_expr(expr_2, false));
                let call = Call(box_tup!(e1, ty_args.clone(), params, expr2_opt));
                if should_wrap {
                    readonly_wrap_expr_(pos.clone(), call)
                } else {
                    call
                }
            }
            FunctionPointer(_b) => exp.clone(),
            Int(_s) => exp.clone(),
            Float(_s) => exp.clone(),
            String(_bstring) => exp.clone(),
            String2(exprs) => String2(self.infer_exprs(exprs, false)),
            PrefixedString(box (str, e)) => {
                PrefixedString(box_tup!(str.clone(), self.infer_expr(e, false)))
            }
            Yield(box field) => {
                let field = self.infer_a_field(field);
                Yield(Box::new(field))
            }
            Await(e) => Await(Box::new(self.infer_expr(e, false))),
            ReadonlyExpr(box e) => ReadonlyExpr(Box::new(self.infer_expr(e, true))),
            Tuple(exprs) => Tuple(self.infer_exprs(exprs, false)),
            List(exprs) => List(self.infer_exprs(exprs, false)),
            Cast(box (hint, e)) => Cast(box_tup!(hint.clone(), self.infer_expr(e, false))),
            Unop(box (unop, e)) => Unop(box_tup!(unop.clone(), self.infer_expr(e, false))),
            Binop(box (bo, lhs, rhs)) => Binop(box_tup!(
                bo.clone(),
                self.infer_expr(lhs, false),
                self.infer_expr(rhs, false),
            )),
            Pipe(box (lid, lhs, rhs)) => {
                let lhs = self.infer_expr(lhs, false);
                let rhs = self.infer_expr(rhs, false);
                Pipe(box_tup!(lid.clone(), lhs, rhs))
            }
            Eif(box (e1, e2_opt, e3)) => {
                let e1 = self.infer_expr(e1, false);
                let e2_opt = e2_opt.as_ref().map(|e2| self.infer_expr(e2, false));
                let e3 = self.infer_expr(e3, false);
                Eif(box_tup!(e1, e2_opt, e3))
            }
            Is(box (e, hint)) => Is(box_tup!(self.infer_expr(e, false), hint.clone())),
            As(box (e, hint, is_nullable)) => {
                let e = self.infer_expr(e, false);
                As(box_tup!(e, hint.clone(), *is_nullable))
            }
            Upcast(box (e, hint)) => {
                let e = self.infer_expr(e, false);
                Upcast(box_tup!(e, hint.clone()))
            }
            New(box (class_id, ty_args, es, e_opt, ex)) => {
                let es = self.infer_exprs(es, false);
                let e_opt = e_opt.as_ref().map(|e| self.infer_expr(e, false));
                New(box_tup!(
                    class_id.clone(),
                    ty_args.clone(),
                    es,
                    e_opt,
                    ex.clone()
                ))
            }
            Efun(box (fun, lids)) => {
                let fun = self.infer_fun(fun);
                Efun(box_tup!(fun, lids.clone()))
            }
            Lfun(box (fun, lid)) => {
                let fun = self.infer_fun(fun);
                Lfun(box_tup!(fun, lid.clone()))
            }
            Xml(box (class_name, attrs, es)) => {
                let es = self.infer_exprs(es, false);
                let attrs: Vec<_> = attrs
                    .iter()
                    .map(|attr| {
                        use ast::XhpAttribute;
                        match attr {
                            XhpAttribute::XhpSimple(ast::XhpSimple { name, type_, expr }) => {
                                XhpAttribute::XhpSimple(ast::XhpSimple {
                                    name: name.clone(),
                                    type_: type_.clone(),
                                    expr: self.infer_expr(expr, false),
                                })
                            }
                            XhpAttribute::XhpSpread(e) => {
                                XhpAttribute::XhpSpread(self.infer_expr(e, false))
                            }
                        }
                    })
                    .collect();
                Xml(box_tup!(class_name.clone(), attrs, es))
            }
            Import(_b) => exp.clone(),
            Collection(box (id, opt_ty_args, a_fields)) => {
                let a_fields = a_fields
                    .iter()
                    .map(|a_field| self.infer_a_field(a_field))
                    .collect();
                Collection(box_tup!(id.clone(), opt_ty_args.clone(), a_fields))
            }
            ExpressionTree(box et) => ExpressionTree(Box::new(ast::ExpressionTree {
                hint: et.hint.clone(),
                splices: et
                    .splices
                    .iter()
                    .map(|splice| self.infer_stmt(splice))
                    .collect(),
                function_pointers: et
                    .function_pointers
                    .iter()
                    .map(|fp| self.infer_stmt(fp))
                    .collect(),
                virtualized_expr: self.infer_expr(&et.virtualized_expr, false),
                runtime_expr: self.infer_expr(&et.runtime_expr, false),
                dollardollar_pos: et.dollardollar_pos.clone(),
            })),
            Lplaceholder(_b) => exp.clone(),
            FunId(_b) => exp.clone(),
            MethodId(box (e, p_str)) => {
                MethodId(box_tup!(self.infer_expr(e, false), p_str.clone()))
            }
            MethodCaller(_b) => exp.clone(),
            SmethodId(_b) => exp.clone(),
            Pair(box (ty_args, e1, e2)) => {
                let e1 = self.infer_expr(e1, false);
                let e2 = self.infer_expr(e2, false);
                Pair(box_tup!(ty_args.clone(), e1, e2))
            }
            ETSplice(box e) => {
                let e = self.infer_expr(e, false);
                ETSplice(Box::new(e))
            }
            EnumClassLabel(_b) => exp.clone(),
            Hole(_b) => exp.clone(),
        };

        aast::Expr(ex.clone(), pos.clone(), exp)
    }

    fn infer_exprs(&self, exprs: &[ast::Expr], in_readonly: bool) -> Vec<ast::Expr> {
        exprs
            .iter()
            .map(|expr| self.infer_expr(expr, in_readonly))
            .collect()
    }

    fn infer_stmt(&self, stmt: &ast::Stmt) -> ast::Stmt {
        let aast::Stmt(pos, st) = stmt;
        use aast::Stmt_::*;

        let new_stmt = match st {
            Fallthrough => Fallthrough,
            Expr(box expr) => {
                let new_expr = self.infer_expr(expr, false);
                Expr(Box::new(new_expr))
            }
            Break => Break,
            Continue => Continue,
            Throw(box expr) => {
                let new_expr = self.infer_expr(expr, false);
                Throw(Box::new(new_expr))
            }
            Return(box opt_expr) => match opt_expr {
                None => Return(Box::new(None)),
                Some(expr) => {
                    let new_expr = self.infer_expr(expr, false);
                    Return(Box::new(Some(new_expr)))
                }
            },
            YieldBreak => YieldBreak,
            Awaitall(box (assigns, block)) => {
                let assigns = assigns
                    .iter()
                    .map(|(lid, e)| (lid.clone(), self.infer_expr(e, false)))
                    .collect();
                let block = self.infer_stmts(block);
                Awaitall(Box::new((assigns, block)))
            }
            If(box (expr, stmt1, stmt2)) => {
                let new_expr = self.infer_expr(expr, false);
                If(box_tup!(new_expr, stmt1.clone(), stmt2.clone()))
            }
            Do(box (stmts, e)) => {
                let stmts = self.infer_stmts(stmts);
                let e = self.infer_expr(e, false);
                Do(box_tup!(stmts, e))
            }
            While(box (cond, block)) => {
                let cond = self.infer_expr(cond, false);
                let block = self.infer_stmts(block);
                While(box_tup!(cond, block))
            }
            Using(box ast::UsingStmt {
                is_block_scoped,
                has_await,
                exprs,
                block,
            }) => {
                let (pos, es) = exprs;
                let es = self.infer_exprs(es, false);
                let block = self.infer_stmts(block);
                Using(Box::new(ast::UsingStmt {
                    is_block_scoped: *is_block_scoped,
                    has_await: *has_await,
                    exprs: (pos.clone(), es),
                    block,
                }))
            }
            For(box (e1s, e2_opt, e3s, stmts)) => {
                let e1s = self.infer_exprs(e1s, false);
                let e2_opt = e2_opt.as_ref().map(|e2| self.infer_expr(e2, false));
                let e3s = self.infer_exprs(e3s, false);
                let stmts = self.infer_stmts(stmts);
                For(box_tup!(e1s, e2_opt, e3s, stmts))
            }
            Switch(box (e, cases, default_opt)) => {
                let e = self.infer_expr(e, false);
                let cases = cases
                    .iter()
                    .map(|ast::Case(e, block)| {
                        let e = self.infer_expr(e, false);
                        let block = self.infer_stmts(block);
                        ast::Case(e, block)
                    })
                    .collect();
                let default_opt = default_opt.as_ref().map(|ast::DefaultCase(pos, block)| {
                    let block = self.infer_stmts(block);
                    ast::DefaultCase(pos.clone(), block)
                });
                Switch(box_tup!(e, cases, default_opt))
            }
            Foreach(box (e, as_, stmts)) => {
                let e = self.infer_expr(e, false);
                let as_ = self.infer_as_expr(as_);
                let stmts = self.infer_stmts(stmts);

                Foreach(box_tup!(e, as_, stmts))
            }
            Try(box (stmts, catches, finally)) => {
                let stmts = self.infer_stmts(stmts);
                let catches = catches
                    .iter()
                    .map(|ast::Catch(name, lid, block)| {
                        ast::Catch(name.clone(), lid.clone(), self.infer_stmts(block))
                    })
                    .collect();
                let finally = self.infer_stmts(finally);
                Try(box_tup!(stmts, catches, finally))
            }
            Noop => st.clone(),
            Block(stmts) => Block(self.infer_stmts(stmts)),
            Markup(_) => st.clone(),
            AssertEnv(b) => AssertEnv(b.clone()),
        };
        ast::Stmt(pos.clone(), new_stmt)
    }

    fn infer_stmts(&self, stmts: &[ast::Stmt]) -> Vec<ast::Stmt> {
        stmts.iter().map(|s| self.infer_stmt(s)).collect()
    }

    fn infer_a_field(&self, a_field: &ast::Afield) -> ast::Afield {
        match a_field {
            aast::Afield::AFvalue(v) => aast::Afield::AFvalue(self.infer_expr(v, false)),
            aast::Afield::AFkvalue(k, v) => {
                aast::Afield::AFkvalue(self.infer_expr(k, false), self.infer_expr(v, false))
            }
        }
    }

    fn infer_as_expr(&self, as_: &ast::AsExpr) -> ast::AsExpr {
        use ast::AsExpr;
        match as_ {
            AsExpr::AsV(e) => AsExpr::AsV(self.infer_expr(e, false)),
            AsExpr::AsKv(k, v) => {
                let k = self.infer_expr(k, false);
                let v = self.infer_expr(v, false);
                AsExpr::AsKv(k, v)
            }
            AsExpr::AwaitAsV(pos, v) => AsExpr::AwaitAsV(pos.clone(), self.infer_expr(v, false)),
            AsExpr::AwaitAsKv(pos, k, v) => {
                let k = self.infer_expr(k, false);
                let v = self.infer_expr(v, false);
                AsExpr::AwaitAsKv(pos.clone(), k, v)
            }
        }
    }

    fn infer_fun(&self, fun: &ast::Fun_) -> ast::Fun_ {
        ast::Fun_ {
            body: ast::FuncBody {
                fb_ast: self.infer_stmts(&fun.body.fb_ast),
            },
            ..fun.clone()
        }
    }

    fn infer_fun_def(&self, fun: &ast::FunDef) -> ast::FunDef {
        ast::FunDef {
            fun: self.infer_fun(&fun.fun),
            ..fun.clone()
        }
    }

    fn infer_class(&self, class: &ast::Class_) -> ast::Class_ {
        let consts = class
            .consts
            .iter()
            .map(|const_| {
                use aast::ClassConstKind::*;
                let kind = match &const_.kind {
                    CCAbstract(e_opt) => {
                        CCAbstract(e_opt.as_ref().map(|e| self.infer_expr(e, false)))
                    }
                    CCConcrete(e) => CCConcrete(self.infer_expr(e, false)),
                };
                ast::ClassConst {
                    kind,
                    ..const_.clone()
                }
            })
            .collect();
        let methods = class
            .methods
            .iter()
            .map(|meth| {
                let body = ast::FuncBody {
                    fb_ast: self.infer_stmts(&meth.body.fb_ast),
                };
                ast::Method_ {
                    body,
                    ..meth.clone()
                }
            })
            .collect();
        let vars = class
            .vars
            .iter()
            .map(|var| {
                let expr = var.expr.as_ref().map(|e| self.infer_expr(e, false));
                ast::ClassVar {
                    expr,
                    ..var.clone()
                }
            })
            .collect();
        ast::Class_ {
            consts,
            methods,
            vars,
            ..class.clone()
        }
    }

    fn infer_typedef(&self, td: &ast::Typedef) -> ast::Typedef {
        td.clone()
    }

    fn type_gconst(&self, gc: &ast::Gconst) -> ast::Gconst {
        // future-proofing: currently has no effect, since const expressions are limited
        let value = self.infer_expr(&gc.value, false);
        ast::Gconst {
            value,
            ..gc.clone()
        }
    }

    fn infer_type_def(&self, d: &ast::Def) -> ast::Def {
        use aast::Def::*;
        match d {
            Fun(box fd) => Fun(Box::new(self.infer_fun_def(fd))),
            Class(box c_) => Class(Box::new(self.infer_class(c_))),
            Stmt(box stmt) => Stmt(Box::new(self.infer_stmt(stmt))),
            Typedef(box td) => Typedef(Box::new(self.infer_typedef(td))),
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

fn readonly_wrap_expr_(pos: Pos, expr_: ast::Expr_) -> ast::Expr_ {
    aast::Expr_::ReadonlyExpr(Box::new(ast::Expr((), pos, expr_)))
}

/// # Panics
/// Panics if a decl provider returns any error other than [`decl_provider::Error::NotFound`];
pub fn infer(prog: &ast::Program, decl_provider: &'_ MemoProvider<'_>) -> ast::Program {
    let infer = Infer { decl_provider };
    ast::Program(
        prog.iter()
            .map(|def| infer.infer_type_def(def))
            .collect::<Vec<_>>(),
    )
}
