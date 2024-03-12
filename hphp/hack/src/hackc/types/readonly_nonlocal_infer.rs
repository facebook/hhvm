// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//// Design:
//// - This module is concerned with inserting `readonly`s where needed at *call sites*, for which it uses decls and inference.
//// - For propagating `readonly`s within a function, there should be no change in the logic in ./readonly_check.rs as compared to what is in the parser today.
//// - The code is biased toward speed of development to get signal about what we can infer.
use std::borrow::Cow;
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use std::env;
use std::sync::Arc;

use decl_provider::DeclProvider;
use decl_provider::Error;
use itertools::Itertools;
use naming_special_names_rust;
use oxidized::aast;
use oxidized::ast;
use oxidized::ast_defs;
use oxidized::ast_defs::Bop;
use oxidized::ast_defs::Pos;
use oxidized_by_ref::shallow_decl_defs::ShallowClass;
use oxidized_by_ref::typing_defs::FunElt;

use crate::subtype;
use crate::tyx;
use crate::tyx::Tyx;

struct Infer<'d> {
    decl_provider: Arc<dyn DeclProvider<'d> + 'd>,
    stats: Stats,
}

#[derive(Debug, Default)]
struct Stats {
    original_readonlys: u64,
    redundant_readonlys: u64,
    added_readonlys: u64,
}

#[derive(Debug, Copy, Clone)]
enum ClassScopeKind {
    Static,
    Nonstatic,
}

type Ctx = HashMap<String, Tyx>;

#[derive(Copy, Clone, Debug, Default)]
struct Where<'c> {
    arg_of_readonly_expr: bool,
    under_try: bool,
    this_class_name: Option<&'c str>,
}

macro_rules! box_tup {
    ($($e:expr),* $(,)?) => {
        Box::new(($($e,)*))
    }
}

impl<'d> Infer<'d> {
    #[allow(clippy::todo)]
    fn infer_expr(
        &mut self,
        expr: &ast::Expr,
        ctx: Ctx,
        where_: Where<'_>,
    ) -> (ast::Expr, Tyx, Ctx) {
        let aast::Expr(ex, pos, exp) = expr;
        use aast::Expr_::*;
        let next_where = Where {
            arg_of_readonly_expr: matches!(exp, ReadonlyExpr(_)),
            ..where_
        };

        let (exp, ty, ctx) = match exp {
            Darray(box (ty_args_opt, kvs)) => {
                let mut kvs_out = Vec::with_capacity(kvs.len());
                let mut ctx = ctx;
                for (k, v) in kvs.iter() {
                    let (k, _k_ty, k_ctx) = self.infer_expr(k, ctx, next_where);
                    ctx = k_ctx;
                    let (v, _v_ty, v_ctx) = self.infer_expr(v, ctx, next_where);
                    ctx = v_ctx;
                    kvs_out.push((k, v))
                }
                (
                    Darray(box_tup!(ty_args_opt.clone(), kvs_out)),
                    Tyx::Todo,
                    ctx,
                )
            }
            Varray(box (ty_args_opt, es)) => {
                let mut es_out = Vec::with_capacity(es.len());
                let mut ctx = ctx;
                for e in es.iter() {
                    let (e, _e_ty, e_ctx) = self.infer_expr(e, ctx, next_where);
                    ctx = e_ctx;
                    es_out.push(e)
                }
                (
                    Varray(box_tup!(ty_args_opt.clone(), es_out)),
                    Tyx::Todo,
                    ctx,
                )
            }
            Shape(fields) => {
                let field_values = fields.iter().map(|field| &field.1).cloned().collect_vec();
                let (field_exprs, _field_tys, ctx) =
                    self.infer_exprs(&field_values, ctx, next_where);
                let fields: Vec<_> = fields
                    .iter()
                    .map(|field| &field.0)
                    .cloned()
                    .zip(field_exprs)
                    .collect();

                (Shape(fields), Tyx::Todo, ctx)
            }
            ValCollection(box (vc_kind, ty_var_opt, es)) => {
                let (es, _tys, ctx) = self.infer_exprs(es, ctx, next_where);
                (
                    ValCollection(box_tup!(vc_kind.clone(), ty_var_opt.clone(), es)),
                    Tyx::Todo,
                    ctx,
                )
            }
            KeyValCollection(box (kvc_kind, ty_var_opt, fields)) => {
                let mut fields_out = Vec::with_capacity(fields.len());
                let mut ctx = ctx;
                for ast::Field(k, v) in fields.iter() {
                    let (k, _k_ty, k_ctx) = self.infer_expr(k, ctx, next_where);
                    ctx = k_ctx;
                    let (v, _v_ty, v_ctx) = self.infer_expr(v, ctx, next_where);
                    ctx = v_ctx;
                    fields_out.push(ast::Field(k, v))
                }
                (
                    KeyValCollection(box_tup!(kvc_kind.clone(), ty_var_opt.clone(), fields_out)),
                    Tyx::Todo,
                    ctx,
                )
            }
            This => (exp.clone(), Tyx::Todo, ctx),
            Null | True | False => (exp.clone(), Tyx::Primitive, ctx),
            Omitted | Invalid(_) => (exp.clone(), Tyx::GiveUp, ctx),
            Id(box id) => {
                let ty = self
                    .get_fun_decl(id, pos)
                    .map_or(Tyx::GiveUp, |ft| Tyx::Fun(Box::new(ft)));
                (exp.clone(), ty, ctx)
            }
            Lvar(box ast::Lid(_, (_, var))) => {
                let ty = ctx.get(var).cloned().unwrap_or(Tyx::GiveUp);
                (exp.clone(), ty, ctx)
            }
            Dollardollar(_b) => (exp.clone(), Tyx::Todo, ctx),
            Clone(box e) => {
                let (e, ty, ctx) = self.infer_expr(e, ctx, next_where);
                (Clone(Box::new(e)), ty, ctx)
            }
            ArrayGet(box (arr, index_opt)) => {
                let (arr, _arr_ty, ctx) = self.infer_expr(arr, ctx, next_where);
                let (index_opt, _index_ty_opt, ctx) =
                    self.infer_expr_opt(index_opt.as_ref(), ctx, next_where);

                (ArrayGet(box_tup!(arr, index_opt)), Tyx::Todo, ctx)
            }
            ObjGet(box (e1, e2, og_null_flavor, prop_or_method)) => {
                let (e1, e1_ty, ctx) = self.infer_expr(e1, ctx, next_where);
                let (e2, _e2_ty, ctx) = self.infer_expr(e2, ctx, next_where);
                let ty = match (&e1_ty, prop_or_method, &e2) {
                    (
                        Tyx::Object { class_name },
                        ast_defs::PropOrMethod::IsMethod,
                        ast::Expr(_, _, aast::Expr_::Id(box ast_defs::Id(_, member_name))),
                    ) => match &self.get_method_type(
                        class_name,
                        member_name,
                        ClassScopeKind::Nonstatic,
                        pos,
                    ) {
                        Some(ft) => Tyx::Fun(Box::new(ft.clone())),
                        None => Tyx::Todo,
                    },
                    _ => Tyx::Todo,
                };
                let obj_get = ObjGet(box_tup!(
                    e1,
                    e2,
                    og_null_flavor.clone(),
                    prop_or_method.clone()
                ));
                (obj_get, ty, ctx)
            }
            ClassGet(box (class_id, get_expr, prop_or_meth)) => {
                let (get_expr, ctx, prop_name_opt) = match get_expr {
                    ClassGetExpr::CGstring((_, prop_name)) => {
                        (get_expr.clone(), ctx, Some(prop_name))
                    }
                    ClassGetExpr::CGexpr(e) => {
                        let (e, _, ctx) = self.infer_expr(e, ctx, next_where);
                        (ClassGetExpr::CGexpr(e), ctx, None)
                    }
                };
                let ty = match (prop_name_opt, class_id_to_name(class_id, where_)) {
                    (Some(prop_name), Some(class_name)) => self
                        .get_prop_type(class_name, prop_name, ClassScopeKind::Static, pos)
                        .unwrap_or(Tyx::GiveUp),
                    _ => Tyx::GiveUp,
                };
                use aast::ClassGetExpr;
                let class_get =
                    ClassGet(box_tup!(class_id.clone(), get_expr, prop_or_meth.clone()));
                let class_get = match &ty {
                    Tyx::Readonly(_) => readonly_wrap_expr_(pos.clone(), class_get),
                    _ => class_get,
                };
                (class_get, ty, ctx)
            }
            ClassConst(box (class_id, (pos, member_name))) => {
                let ty = match class_id_to_name(class_id, where_) {
                    Some(class_name) => self
                        .get_method_type(class_name, member_name, ClassScopeKind::Static, pos)
                        .map_or(Tyx::GiveUp, |ft| Tyx::Fun(Box::new(ft))),
                    None => Tyx::GiveUp,
                };
                let class_const = ClassConst(box_tup!(
                    class_id.clone(),
                    (pos.clone(), member_name.clone())
                ));
                (class_const, ty, ctx)
            }
            Call(box aast::CallExpr {
                func: e1,
                targs,
                args,
                unpacked_arg: expr2_opt,
            }) => {
                let (e1, e1_ty, ctx) = self.infer_expr(e1, ctx, next_where);
                let (param_kinds, param_exprs): (Vec<ast::ParamKind>, Vec<ast::Expr>) =
                    args.iter().cloned().unzip();
                let (param_exprs, _param_tys, ctx) =
                    self.infer_exprs(&param_exprs, ctx, next_where);
                let (expr2_opt, _expr2_opt_ty, ctx) =
                    self.infer_expr_opt(expr2_opt.as_ref(), ctx, next_where);
                let args = param_kinds.into_iter().zip(param_exprs).collect();
                let mut call = Call(Box::new(aast::CallExpr {
                    func: e1,
                    targs: targs.clone(),
                    args,
                    unpacked_arg: expr2_opt,
                }));
                match &e1_ty {
                    Tyx::Fun(box ft) if returns_readonly(ft) => {
                        if where_.arg_of_readonly_expr {
                            self.stats.redundant_readonlys += 1;
                        } else {
                            call = readonly_wrap_expr_(pos.clone(), call);
                            self.stats.added_readonlys += 1;
                        }
                    }
                    _ => (),
                }
                (call, Tyx::Todo, ctx)
            }
            FunctionPointer(box (ptr, _ty_args)) => {
                let ty = match ptr {
                    aast::FunctionPtrId::FPId(id) => match self.get_fun_decl(id, pos) {
                        Some(ft) => Tyx::Fun(Box::new(ft)),
                        None => Tyx::GiveUp,
                    },
                    aast::FunctionPtrId::FPClassConst(class_id, (pos, member_name)) => {
                        class_id_to_name(class_id, where_)
                            .and_then(|class_name| {
                                self.get_method_type(
                                    class_name,
                                    member_name,
                                    ClassScopeKind::Static,
                                    pos,
                                )
                            })
                            .map_or(Tyx::Todo, |ft| Tyx::Fun(Box::new(ft)))
                    }
                };
                (exp.clone(), ty, ctx)
            }
            Int(_s) => (exp.clone(), Tyx::Todo, ctx),
            Float(_s) => (exp.clone(), Tyx::Todo, ctx),
            String(_bstring) => (exp.clone(), Tyx::Todo, ctx),
            String2(es) => {
                let (es, _tys, ctx) = self.infer_exprs(es, ctx, next_where);
                (String2(es), Tyx::Todo, ctx)
            }
            PrefixedString(box (str, e)) => {
                let (e, _ty, ctx) = self.infer_expr(e, ctx, next_where);
                (PrefixedString(box_tup!(str.clone(), e)), Tyx::Todo, ctx)
            }
            Nameof(_) => (exp.clone(), Tyx::Todo, ctx), // behaves like string literal
            Yield(box field) => {
                let (field, _field_ty, ctx) = self.infer_a_field(field, ctx, next_where);
                (Yield(Box::new(field)), Tyx::Todo, ctx)
            }
            Await(e) => {
                let (e, _ty, ctx) = self.infer_expr(e, ctx, next_where);
                (Await(Box::new(e)), Tyx::Todo, ctx)
            }
            ReadonlyExpr(box e) => {
                self.stats.original_readonlys += 1;
                let (e, _ty, ctx) = self.infer_expr(e, ctx, next_where);
                (ReadonlyExpr(Box::new(e)), Tyx::Todo, ctx)
            }
            Tuple(exprs) => {
                let (exprs, _tys, ctx) = self.infer_exprs(exprs, ctx, next_where);
                (Tuple(exprs), Tyx::Todo, ctx)
            }
            List(exprs) => {
                let (exprs, _tys, ctx) = self.infer_exprs(exprs, ctx, next_where);
                (List(exprs), Tyx::Todo, ctx)
            }
            Cast(box (hint, e)) => {
                let (e, _ty, ctx) = self.infer_expr(e, ctx, next_where);
                (Cast(box_tup!(hint.clone(), e)), Tyx::Todo, ctx)
            }
            Unop(box (unop, e)) => {
                let (e, _ty, ctx) = self.infer_expr(e, ctx, next_where);
                (Unop(box_tup!(unop.clone(), e)), Tyx::Todo, ctx)
            }
            // Does not yet handle `list`
            Binop(box aast::Binop {
                bop: eq @ Bop::Eq(bop_opt),
                lhs: lhs @ ast::Expr(_, _, Lvar(box ast::Lid(_, (_, var_name)))),
                rhs,
            }) => {
                let (rhs, r_ty, ctx) = self.infer_expr(rhs, ctx, next_where);
                let (lhs, _l_ty, mut ctx) = self.infer_expr(lhs, ctx, next_where);
                match bop_opt {
                    None => {
                        {
                            let ty_to_insert = match ctx.get(var_name) {
                                Some(existing_ty) if where_.under_try => {
                                    subtype::join(Cow::Borrowed(existing_ty), Cow::Owned(r_ty))
                                        .into_owned()
                                }
                                _ => r_ty,
                            };
                            ctx.insert(var_name.clone(), ty_to_insert);
                        }
                        (
                            Binop(Box::new(aast::Binop {
                                bop: eq.clone(),
                                lhs,
                                rhs,
                            })),
                            Tyx::GiveUp, // hhvm doesn't actually allow assignments to be used as expressions
                            ctx,
                        )
                    }
                    // no special handling for `+=` etc. yet
                    Some(_) => (
                        Binop(Box::new(aast::Binop {
                            bop: eq.clone(),
                            lhs,
                            rhs,
                        })),
                        Tyx::GiveUp,
                        ctx,
                    ),
                }
            }
            Binop(box ast::Binop { bop, lhs, rhs }) => {
                let (lhs, _l_ty, ctx) = self.infer_expr(lhs, ctx, next_where);
                let (rhs, _r_ty, ctx) = self.infer_expr(rhs, ctx, next_where);
                (
                    Binop(Box::new(ast::Binop {
                        bop: bop.clone(),
                        lhs,
                        rhs,
                    })),
                    Tyx::Todo,
                    ctx,
                )
            }
            Pipe(box (lid, lhs, rhs)) => {
                let (lhs, _l_ty, ctx) = self.infer_expr(lhs, ctx, next_where);
                let (rhs, _r_ty, ctx) = self.infer_expr(rhs, ctx, next_where);
                (Pipe(box_tup!(lid.clone(), lhs, rhs)), Tyx::Todo, ctx)
            }
            Eif(box (e1, e2_opt, e3)) => {
                let (e1, _e1_ty, ctx) = self.infer_expr(e1, ctx, next_where);
                let (e2_opt, _index_ty_opt, ctx) =
                    self.infer_expr_opt(e2_opt.as_ref(), ctx, next_where);
                let (e3, _e3_ty, ctx) = self.infer_expr(e3, ctx, next_where);
                (Eif(box_tup!(e1, e2_opt, e3)), Tyx::Todo, ctx)
            }
            Is(box (e, hint)) => {
                let (e, _e_ty, ctx) = self.infer_expr(e, ctx, next_where);
                (Is(box_tup!(e, hint.clone())), Tyx::Todo, ctx)
            }
            As(box ast::As_ {
                expr: e,
                hint,
                is_nullable,
                enforce_deep,
            }) => {
                let (e, _e_ty, mut ctx) = self.infer_expr(e, ctx, next_where);
                match &e {
                    ast::Expr(_, _, ast::Expr_::Lvar(box ast::Lid(_, (_, var)))) => {
                        // There's demo readonly code that does something like this:
                        // `$var as dynamic; $_ = $var->method_that_returns_readonly()`
                        // To ensure that we don't change the bytecode emitted in such cases, remove
                        // the variable from the context. TODO(T131219582): use type information from the hint
                        ctx.remove(var);
                    }
                    _ => (),
                }
                (
                    As(Box::new(ast::As_ {
                        expr: e,
                        hint: hint.clone(),
                        is_nullable: *is_nullable,
                        enforce_deep: *enforce_deep,
                    })),
                    Tyx::Todo,
                    ctx,
                )
            }
            Upcast(box (e, hint)) => {
                let (e, _e_ty, ctx) = self.infer_expr(e, ctx, next_where);
                (Upcast(box_tup!(e, hint.clone())), Tyx::Todo, ctx)
            }
            New(box (class_id, ty_args, es, e_opt, ex)) => {
                let (es, _es_tys, ctx) = self.infer_exprs(es, ctx, next_where);
                let (e_opt, _e_ty_opt, ctx) = self.infer_expr_opt(e_opt.as_ref(), ctx, next_where);
                let new = New(box_tup!(
                    class_id.clone(),
                    ty_args.clone(),
                    es,
                    e_opt,
                    ex.clone()
                ));
                let ty = class_id_to_name(class_id, where_).map_or(Tyx::Todo, |class_name| {
                    Tyx::Object {
                        class_name: class_name.to_string(),
                    }
                });
                (new, ty, ctx)
            }
            Efun(efun) => {
                let (fun, _fun_ty, ctx) = self.infer_fun(&efun.fun, ctx, next_where);
                (
                    Efun(Box::new(ast::Efun {
                        fun,
                        use_: efun.use_.clone(),
                        closure_class_name: efun.closure_class_name.clone(),
                    })),
                    Tyx::Todo,
                    ctx,
                )
            }
            Lfun(box (fun, lid)) => {
                let (fun, _fun_ty, ctx) = self.infer_fun(fun, ctx, next_where);
                (Lfun(box_tup!(fun, lid.clone())), Tyx::Todo, ctx)
            }
            Xml(box (class_name, attrs, es)) => {
                let (es, _tys, mut ctx) = self.infer_exprs(es, ctx, next_where);
                use ast::XhpAttribute;
                let mut attrs_out = Vec::with_capacity(attrs.len());
                for attr in attrs.iter() {
                    match attr {
                        XhpAttribute::XhpSimple(ast::XhpSimple { name, type_, expr }) => {
                            let (expr, _ty, expr_ctx) = self.infer_expr(expr, ctx, next_where);
                            ctx = expr_ctx;
                            attrs_out.push(XhpAttribute::XhpSimple(ast::XhpSimple {
                                name: name.clone(),
                                type_: type_.clone(),
                                expr,
                            }))
                        }
                        XhpAttribute::XhpSpread(e) => {
                            let (e, _ty, e_ctx) = self.infer_expr(e, ctx, next_where);
                            ctx = e_ctx;
                            attrs_out.push(XhpAttribute::XhpSpread(e))
                        }
                    }
                }
                let xml = Xml(box_tup!(class_name.clone(), attrs_out, es));
                (xml, Tyx::Todo, ctx)
            }
            Import(_b) => (exp.clone(), Tyx::Todo, ctx),
            Collection(box (id, opt_ty_args, a_fields)) => {
                let mut a_fields_out = Vec::with_capacity(a_fields.len());
                let mut ctx = ctx;
                for a_field in a_fields.iter() {
                    let (a_field, _field_ty, field_ctx) =
                        self.infer_a_field(a_field, ctx, next_where);
                    ctx = field_ctx;
                    a_fields_out.push(a_field);
                }
                (
                    Collection(box_tup!(id.clone(), opt_ty_args.clone(), a_fields_out)),
                    Tyx::Todo,
                    ctx,
                )
            }
            ExpressionTree(box et) => {
                let (virtualized_expr, _virtualized_ty, ctx) =
                    self.infer_expr(&et.virtualized_expr, ctx, next_where);
                let (runtime_expr, _runtime_ty, ctx) =
                    self.infer_expr(&et.runtime_expr, ctx, next_where);
                let (splices, ctx) = self.infer_stmts_block(&et.splices, ctx, next_where);
                let (function_pointers, ctx) =
                    self.infer_stmts_block(&et.function_pointers, ctx, next_where);
                let splices = splices.0; // we want Vec<Stmt> rather than Block
                let function_pointers = function_pointers.0; // we want Vec<Stmt> rather than Block
                let et = ExpressionTree(Box::new(ast::ExpressionTree {
                    hint: et.hint.clone(),
                    splices,
                    function_pointers,
                    virtualized_expr,
                    runtime_expr,
                    dollardollar_pos: et.dollardollar_pos.clone(),
                }));
                (et, Tyx::Todo, ctx)
            }
            // The runtime iuc treats $_ like a normal variable, but
            // hh` doesn't allow such code:
            // $_ = 3; echo $_; // prints `3`, doesn't type-check
            // Soundly approximate by treating such code as ill-formed.
            Lplaceholder(_b) => (exp.clone(), Tyx::GiveUp, ctx),
            MethodCaller(_b) => (exp.clone(), Tyx::Todo, ctx),
            Pair(box (ty_args, e1, e2)) => {
                let (e1, _ty1, ctx) = self.infer_expr(e1, ctx, next_where);
                let (e2, _ty2, ctx) = self.infer_expr(e2, ctx, next_where);
                let pair = Pair(box_tup!(ty_args.clone(), e1, e2));
                (pair, Tyx::Todo, ctx)
            }
            ETSplice(box e) => {
                let (e, _ty, ctx) = self.infer_expr(e, ctx, next_where);
                (ETSplice(Box::new(e)), Tyx::Todo, ctx)
            }
            EnumClassLabel(_b) => (exp.clone(), Tyx::Todo, ctx),
            Hole(_b) => (exp.clone(), Tyx::Todo, ctx),
            Package(_) => todo!(),
        };

        let expr = aast::Expr(ex.clone(), pos.clone(), exp);
        (expr, ty, ctx)
    }

    fn infer_exprs(
        &mut self,
        exprs: &[ast::Expr],
        mut ctx: Ctx,
        where_: Where<'_>,
    ) -> (Vec<ast::Expr>, Vec<Tyx>, Ctx) {
        let mut es = Vec::with_capacity(exprs.len());
        let mut tys = Vec::with_capacity(exprs.len());
        for e in exprs.iter() {
            let (e, ty, e_ctx) = self.infer_expr(e, ctx, where_);
            ctx = e_ctx;
            es.push(e);
            tys.push(ty);
        }
        (es, tys, ctx)
    }

    fn infer_expr_opt(
        &mut self,
        expr_opt: Option<&ast::Expr>,
        ctx: Ctx,
        where_: Where<'_>,
    ) -> (Option<ast::Expr>, Option<Tyx>, Ctx) {
        match expr_opt {
            None => (None, None, ctx),
            Some(e) => {
                let (e, ty, ctx) = self.infer_expr(e, ctx, where_);
                (Some(e), Some(ty), ctx)
            }
        }
    }

    #[allow(clippy::todo)]
    fn infer_stmt(&mut self, stmt: &ast::Stmt, ctx: Ctx, where_: Where<'_>) -> (ast::Stmt, Ctx) {
        let aast::Stmt(pos, st) = stmt;
        use aast::Stmt_::*;
        let next_where = Where {
            arg_of_readonly_expr: false,
            ..where_
        };

        let (new_stmt, ctx) = match st {
            Fallthrough => (Fallthrough, ctx),
            Expr(box expr) => {
                let (new_expr, _expr_ty, ctx) = self.infer_expr(expr, ctx, next_where);
                (Expr(Box::new(new_expr)), ctx)
            }
            Break => (Break, ctx),
            Continue => (Continue, ctx),
            Throw(box e) => {
                let (e, _e_ty, ctx) = self.infer_expr(e, ctx, next_where);
                (Throw(Box::new(e)), ctx)
            }
            Return(box opt_expr) => match opt_expr {
                None => (Return(Box::new(None)), ctx),
                Some(e) => {
                    let (e, _e_ty, ctx) = self.infer_expr(e, ctx, next_where);
                    (Return(Box::new(Some(e))), ctx)
                }
            },
            YieldBreak => (YieldBreak, ctx),
            Awaitall(box (assigns, block)) => {
                let lids: Vec<ast::Lid> = assigns.iter().map(|(lid, _)| lid).cloned().collect();
                let assigns_exprs: Vec<_> =
                    assigns.iter().map(|(_, assign)| assign).cloned().collect();
                let (assigns_exprs, _assigns_tys, ctx) =
                    self.infer_exprs(&assigns_exprs, ctx, next_where);
                let (block, ctx) = self.infer_stmts_block(block, ctx, next_where);
                let assigns: Vec<_> = lids.into_iter().zip(assigns_exprs.into_iter()).collect();
                let await_all = Awaitall(Box::new((assigns, block)));
                (await_all, ctx)
            }
            // The lowerer converts concurrent statements to Awaitall when hackc
            // is going to generate bytecode (instead of type checking/TAST
            // generating)
            Concurrent(_) => panic!("Concurrent statement in readonly_nonlocal_infer"),
            If(box (expr, stmts1, stmts2)) => {
                let (e, _e_ty, ctx) = self.infer_expr(expr, ctx, next_where);
                let (stmts1, child_ctx_1) = self.infer_stmts_block(stmts1, ctx.clone(), next_where);
                let (stmts2, child_ctx_2) = self.infer_stmts_block(stmts2, ctx.clone(), next_where);
                let ctx = merge_ctxs(ctx, vec![child_ctx_1, child_ctx_2]);
                let if_ = If(box_tup!(e, stmts1, stmts2));
                (if_, ctx)
            }
            Do(box (stmts, e)) => {
                let (stmts, child_ctx) = self.infer_stmts_block(stmts, ctx.clone(), next_where);
                let ctx = merge_ctxs(ctx, vec![child_ctx]);
                let (e, _e_ty, ctx) = self.infer_expr(e, ctx, next_where);
                (Do(box_tup!(stmts, e)), ctx)
            }
            While(box (cond, block)) => {
                let (cond, _cond_ty, ctx) = self.infer_expr(cond, ctx, next_where);
                let (block, child_ctx) = self.infer_stmts_block(block, ctx.clone(), next_where);
                let ctx = merge_ctxs(ctx, vec![child_ctx]);
                (While(box_tup!(cond, block)), ctx)
            }
            Using(box ast::UsingStmt {
                is_block_scoped,
                has_await,
                exprs,
                block,
            }) => {
                let (pos, es) = exprs;
                let (es, _es_tys, ctx) = self.infer_exprs(es, ctx, next_where);
                let (block, ctx) = self.infer_stmts_block(block, ctx, next_where);
                let using = Using(Box::new(ast::UsingStmt {
                    is_block_scoped: *is_block_scoped,
                    has_await: *has_await,
                    exprs: (pos.clone(), es),
                    block,
                }));
                (using, ctx)
            }
            For(box (e1s, e2_opt, e3s, stmts)) => {
                let (e1s, _e1s_tys, ctx) = self.infer_exprs(e1s, ctx, next_where);
                let (e2_opt, _e2_ty_opt, ctx) =
                    self.infer_expr_opt(e2_opt.as_ref(), ctx, next_where);
                let (e3s, _e3s_tys, e3s_ctx) = self.infer_exprs(e3s, ctx.clone(), next_where);
                let (stmts, stmt_ctx) = self.infer_stmts_block(stmts, ctx.clone(), next_where);
                let ctx = merge_ctxs(ctx, vec![e3s_ctx, stmt_ctx]);
                let for_ = For(box_tup!(e1s, e2_opt, e3s, stmts));
                (for_, ctx)
            }
            Switch(box (e, cases, default_opt)) => {
                let (e, _e_ty, ctx) = self.infer_expr(e, ctx, next_where);
                let mut cases_out = Vec::with_capacity(cases.len());
                let mut child_ctxs = Vec::with_capacity(cases.len());
                for ast::Case(e, block) in cases {
                    let (e, _e_ty, e_ctx) = self.infer_expr(e, ctx.clone(), next_where);
                    let (block, child_ctx) = self.infer_stmts_block(block, e_ctx, next_where);
                    cases_out.push(ast::Case(e, block));
                    child_ctxs.push(child_ctx);
                }
                let default_opt = default_opt.as_ref().map(|ast::DefaultCase(pos, block)| {
                    let (block, ctx) = self.infer_stmts_block(block, ctx.clone(), next_where);
                    child_ctxs.push(ctx);
                    ast::DefaultCase(pos.clone(), block)
                });
                let ctx = merge_ctxs(ctx, child_ctxs);
                let switch = Switch(box_tup!(e, cases_out, default_opt));
                (switch, ctx)
            }
            Foreach(box (e, as_, stmts)) => {
                let (e, _e_ty, ctx) = self.infer_expr(e, ctx, next_where);
                let (as_, _as_ty, ctx) = self.infer_as_expr(as_, ctx, next_where);
                let (stmts, child_ctx) = self.infer_stmts_block(stmts, ctx.clone(), next_where);
                let ctx = merge_ctxs(ctx, vec![child_ctx]);
                (Foreach(box_tup!(e, as_, stmts)), ctx)
            }
            Try(box (stmts, catches, finally)) => {
                let (stmts, ctx) = self.infer_stmts_block(
                    stmts,
                    ctx,
                    Where {
                        under_try: true,
                        ..next_where
                    },
                );
                let mut catches_out = Vec::with_capacity(catches.len());
                let mut child_ctxs = Vec::with_capacity(catches.len());
                for ast::Catch(name, lid, block) in catches.iter() {
                    let (block, catch_ctx) = self.infer_stmts_block(block, ctx.clone(), next_where);
                    let catch = ast::Catch(name.clone(), lid.clone(), block);
                    catches_out.push(catch);
                    child_ctxs.push(catch_ctx);
                }
                let (finally, finally_ctx) =
                    self.infer_stmts_finally_block(finally, ctx.clone(), next_where);
                child_ctxs.push(finally_ctx);
                let ctx = merge_ctxs(ctx, child_ctxs);
                (Try(box_tup!(stmts, catches_out, finally)), ctx)
            }
            Noop => (Noop, ctx),
            DeclareLocal(box (id, h, expr)) => {
                if let Some(expr) = expr {
                    let (new_expr, _expr_ty, ctx) = self.infer_expr(expr, ctx, next_where);
                    (
                        DeclareLocal(box_tup!(id.clone(), h.clone(), Some(new_expr))),
                        ctx,
                    )
                } else {
                    (st.clone(), ctx)
                }
            }
            Block(box (vars, stmts)) => {
                let (stmts, ctx) = self.infer_stmts_block(stmts, ctx, next_where);
                (Block(Box::new((vars.clone(), stmts))), ctx)
            }
            Markup(_) => (st.clone(), ctx),
            Match(..) => todo!("TODO(jakebailey): match statements"),
        };
        (ast::Stmt(pos.clone(), new_stmt), ctx)
    }

    fn infer_stmts_block(
        &mut self,
        stmts: &[ast::Stmt],
        mut ctx: Ctx,
        where_: Where<'_>,
    ) -> (ast::Block, Ctx) {
        let mut out = Vec::with_capacity(stmts.len());
        for stmt in stmts.iter() {
            let (s, s_ctx) = self.infer_stmt(stmt, ctx, where_);
            out.push(s);
            ctx = s_ctx;
        }
        (ast::Block(out), ctx)
    }

    fn infer_stmts_finally_block(
        &mut self,
        stmts: &[ast::Stmt],
        mut ctx: Ctx,
        where_: Where<'_>,
    ) -> (ast::FinallyBlock, Ctx) {
        let mut out = Vec::with_capacity(stmts.len());
        for stmt in stmts.iter() {
            let (s, s_ctx) = self.infer_stmt(stmt, ctx, where_);
            out.push(s);
            ctx = s_ctx;
        }
        (ast::FinallyBlock(out), ctx)
    }

    fn infer_a_field(
        &mut self,
        a_field: &ast::Afield,
        ctx: Ctx,
        where_: Where<'_>,
    ) -> (ast::Afield, Tyx, Ctx) {
        match a_field {
            aast::Afield::AFvalue(v) => {
                let (v, _v_ty, ctx) = self.infer_expr(v, ctx, where_);
                let a_field = aast::Afield::AFvalue(v);
                (a_field, Tyx::Todo, ctx)
            }
            aast::Afield::AFkvalue(k, v) => {
                let (k, _k_ty, ctx) = self.infer_expr(k, ctx, where_);
                let (v, _v_ty, ctx) = self.infer_expr(v, ctx, where_);
                let a_field = aast::Afield::AFkvalue(k, v);
                (a_field, Tyx::Todo, ctx)
            }
        }
    }

    fn infer_as_expr(
        &mut self,
        as_: &ast::AsExpr,
        ctx: Ctx,
        where_: Where<'_>,
    ) -> (ast::AsExpr, Tyx, Ctx) {
        use ast::AsExpr;
        match as_ {
            AsExpr::AsV(e) => {
                let (e, _ty, ctx) = self.infer_expr(e, ctx, where_);
                (AsExpr::AsV(e), Tyx::Todo, ctx)
            }
            AsExpr::AsKv(k, v) => {
                let (k, _k_ty, ctx) = self.infer_expr(k, ctx, where_);
                let (v, _v_ty, ctx) = self.infer_expr(v, ctx, where_);
                (AsExpr::AsKv(k, v), Tyx::Todo, ctx)
            }
            AsExpr::AwaitAsV(pos, v) => {
                let (v, _v_ty, ctx) = self.infer_expr(v, ctx, where_);
                (AsExpr::AwaitAsV(pos.clone(), v), Tyx::Todo, ctx)
            }
            AsExpr::AwaitAsKv(pos, k, v) => {
                let (k, _k_ty, ctx) = self.infer_expr(k, ctx, where_);
                let (v, _v_ty, ctx) = self.infer_expr(v, ctx, where_);
                (AsExpr::AwaitAsKv(pos.clone(), k, v), Tyx::Todo, ctx)
            }
        }
    }

    fn infer_fun(&mut self, fun: &ast::Fun_, ctx: Ctx, where_: Where<'_>) -> (ast::Fun_, Tyx, Ctx) {
        // it's safe to ignore any `use` clause, since we treat undefined variables as of unknown type
        let (fb_ast, _) = self.infer_stmts_block(&fun.body.fb_ast, ctx.clone(), where_);
        let fun = ast::Fun_ {
            body: ast::FuncBody { fb_ast },
            ..fun.clone()
        };
        (fun, Tyx::Todo, ctx)
    }

    fn infer_fun_def(&mut self, fun_def: &ast::FunDef) -> ast::FunDef {
        let (fun, _fun_ty, _ctx) =
            self.infer_fun(&fun_def.fun, Default::default(), Default::default());
        ast::FunDef {
            fun,
            ..fun_def.clone()
        }
    }

    fn infer_class(&mut self, class: &ast::Class_) -> ast::Class_ {
        let where_ = Where {
            this_class_name: Some(&class.name.1), // potential optimization: reference the class rather than just the name
            ..Default::default()
        };
        let consts = class
            .consts
            .iter()
            .map(|const_| {
                use aast::ClassConstKind::*;
                let kind = match &const_.kind {
                    CCAbstract(e_opt) => {
                        let (e_opt, _e_opt_ty, _ctx) = self.infer_expr_opt(
                            e_opt.as_ref(),
                            Default::default(),
                            Default::default(),
                        );
                        CCAbstract(e_opt)
                    }
                    CCConcrete(e) => {
                        let (e, _e_ty, _ctx) = self.infer_expr(e, Default::default(), where_);
                        CCConcrete(e)
                    }
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
                let (fb_ast, _ctx) =
                    self.infer_stmts_block(&meth.body.fb_ast, Default::default(), where_);
                let body = ast::FuncBody { fb_ast };
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
                let (expr, _expr_ty, _ctx) =
                    self.infer_expr_opt(var.expr.as_ref(), Default::default(), Default::default());
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

    fn type_gconst(&mut self, gc: &ast::Gconst) -> ast::Gconst {
        // future-proofing: currently has no effect, since const expressions are limited
        let (value, _v_ty, _ctx) =
            self.infer_expr(&gc.value, Default::default(), Default::default());
        ast::Gconst {
            value,
            ..gc.clone()
        }
    }

    fn infer_type_def(&mut self, d: &ast::Def) -> ast::Def {
        use aast::Def::*;
        match d {
            Fun(box fd) => Fun(Box::new(self.infer_fun_def(fd))),
            Class(box c_) => Class(Box::new(self.infer_class(c_))),
            Stmt(box stmt) => {
                let (stmt, _ctx) = self.infer_stmt(stmt, Default::default(), Default::default());
                Stmt(Box::new(stmt))
            }
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

    fn get_prop_type(
        &self,
        class_name: &str,
        prop_name: &str,
        prop_scope: ClassScopeKind,
        pos: &Pos,
    ) -> Option<Tyx> {
        let shallow_class = self.get_shallow_class(class_name, pos)?;
        let props = match prop_scope {
            ClassScopeKind::Static => shallow_class.sprops,
            ClassScopeKind::Nonstatic => shallow_class.props,
        };
        let prop = props.iter().find(|prop| prop.name.1 == prop_name)?;
        let ty = tyx::convert(&prop.type_.1);
        let ty = if prop.flags.is_readonly() {
            Tyx::Readonly(Box::new(ty))
        } else {
            ty
        };
        Some(ty)
    }

    fn get_method_type(
        &self,
        class_name: &str,
        method_name: &str,
        method_scope: ClassScopeKind,
        pos: &Pos,
    ) -> Option<tyx::FunType> {
        let shallow_class = self.get_shallow_class(class_name, pos)?;
        let methods = match method_scope {
            ClassScopeKind::Static => shallow_class.static_methods,
            ClassScopeKind::Nonstatic => shallow_class.methods,
        };
        let method = methods.iter().find(|method| method.name.1 == method_name)?;
        tyx::ty_to_fun_type_opt(&method.type_.1)
    }

    // TODO: look up in parent classes using folded decls
    fn get_shallow_class(&self, class_name: &str, pos: &Pos) -> Option<&'_ ShallowClass<'_>> {
        match self.decl_provider.type_decl(class_name, 1) {
            Ok(decl_provider::TypeDecl::Class(shallow_class)) => Some(shallow_class),
            Ok(_) => None, // reachable if TypeDecl::Typedef, which is currently not allowed to be an alias for a class
            Err(Error::NotFound) => None,
            Err(err @ Error::Bincode(_)) => {
                panic!(
                    "Internal error when attempting to read the type of class '{class_name}' at {pos:?}. {err}"
                )
            }
        }
    }

    fn get_fun_decl(&self, id: &ast::Id, pos: &Pos) -> Option<tyx::FunType> {
        let name = id.name();
        match self.decl_provider.func_decl(name) {
            Ok(FunElt {
                type_: oxidized_by_ref::typing_defs::Ty(_, ty_),
                ..
            }) => tyx::ty_to_fun_type_opt(ty_),
            Err(Error::NotFound) => None,
            Err(err @ Error::Bincode(_)) => {
                panic!(
                    "Internal error when attempting to read the type of callable '{name}' at {pos:?}. {err}"
                )
            }
        }
    }
}

fn merge_ctxs(mut parent: Ctx, children: Vec<Ctx>) -> Ctx {
    for child in children {
        for (var, ty) in child.into_iter() {
            match parent.entry(var) {
                Entry::Occupied(mut entry) => {
                    let existing_ty = entry.get_mut();
                    *existing_ty =
                        subtype::join(Cow::Borrowed(existing_ty), Cow::Owned(ty)).into_owned();
                }
                Entry::Vacant(entry) => {
                    entry.insert(ty);
                }
            }
        }
    }
    parent
}

fn readonly_wrap_expr_(pos: Pos, expr_: ast::Expr_) -> ast::Expr_ {
    aast::Expr_::ReadonlyExpr(Box::new(ast::Expr((), pos, expr_)))
}

fn returns_readonly(ft: &tyx::FunType) -> bool {
    matches!(ft.ret, Tyx::Readonly(_))
}

fn class_id_to_name<'c>(class_id: &'c aast::ClassId<(), ()>, where_: Where<'c>) -> Option<&'c str> {
    use aast::ClassId_::*;
    let self_class_name = where_.this_class_name;
    let class_name: Option<&str> = match &class_id.2 {
        CI(ast_defs::Id(_, class_name)) => Some(class_name),
        CIexpr(ast::Expr(_, _, aast::Expr_::Id(box ast_defs::Id(_, class_name)))) => {
            Some(class_name) // might be something magic like "self"
        }
        CIself => self_class_name,
        _ => None, // TODO: handle more cases, such as CIParent
    };
    match class_name? {
        naming_special_names_rust::classes::SELF => self_class_name,
        // TODO: handle more cases
        _ => class_name,
    }
}

/// # Panics
/// Panics if a decl provider returns any error other than [`decl_provider::Error::NotFound`];
pub fn infer<'d>(
    prog: &ast::Program,
    decl_provider: Arc<dyn DeclProvider<'d> + 'd>,
) -> ast::Program {
    let mut infer: Infer<'d> = Infer {
        decl_provider,
        stats: Default::default(),
    };
    let res = ast::Program(
        prog.iter()
            .map(|def| infer.infer_type_def(def))
            .collect::<Vec<_>>(),
    );
    if env::var("HACKC_INTERNAL_LOG_READONLY_TDB_DIAGNOSTICS")
        .map(|s| s.parse::<u8>().unwrap_or_default())
        .unwrap_or_default()
        == 1
    {
        let Stats {
            original_readonlys,
            redundant_readonlys,
            added_readonlys,
        } = infer.stats;
        eprintln!(
            "original_readonlys: {original_readonlys} redundant_readonlys: {redundant_readonlys} added_readonlys: {added_readonlys}",
        );
    }
    res
}
