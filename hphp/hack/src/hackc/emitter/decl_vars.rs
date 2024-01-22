// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(box_patterns)]

use ffi::Just;
use ffi::Maybe;
use hash::IndexSet;
use hhbc::Label;
use hhbc::Param;
use naming_special_names_rust::emitter_special_functions;
use naming_special_names_rust::special_idents;
use oxidized::aast;
use oxidized::aast::Binop;
use oxidized::aast_visitor::visit;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::Node;
use oxidized::aast_visitor::Visitor;
use oxidized::ast::*;
use oxidized::ast_defs;

type SSet = std::collections::BTreeSet<String>;

struct DeclvarVisitorContext<'a> {
    explicit_use_set_opt: Option<&'a SSet>,
    capture_debugger_vars: bool,
}

struct DeclvarVisitor<'a> {
    // set of locals used inside the functions
    locals: IndexSet<String>,
    context: DeclvarVisitorContext<'a>,
}

impl<'a> DeclvarVisitor<'a> {
    fn new(explicit_use_set_opt: Option<&'a SSet>, capture_debugger_vars: bool) -> Self {
        Self {
            locals: Default::default(),
            context: DeclvarVisitorContext {
                explicit_use_set_opt,
                capture_debugger_vars,
            },
        }
    }

    fn add_local<S: Into<String> + AsRef<str>>(&mut self, name: S) {
        let name_ref = name.as_ref();
        if name_ref == special_idents::DOLLAR_DOLLAR
            || special_idents::is_tmp_var(name_ref)
            || name_ref == special_idents::THIS
        {
        } else {
            self.locals.insert(name.into());
        }
    }

    fn on_class_get(&mut self, cid: &ClassId, cge: &ClassGetExpr) -> Result<(), String> {
        use aast::ClassId_;
        match &cid.2 {
            ClassId_::CIparent | ClassId_::CIself | ClassId_::CIstatic | ClassId_::CI(_) => {
                Err("Expects CIexpr as class_id on aast where expr was on ast".into())
            }
            ClassId_::CIexpr(e) => {
                self.visit_expr(&mut (), e)?;
                match cge {
                    ClassGetExpr::CGstring(pstr) => {
                        // TODO(thomasjiang): For this to match correctly, we need to adjust
                        // ast_to_nast because it does not make a distinction between ID and Lvar,
                        // which is needed here
                        self.add_local(&pstr.1);
                        Ok(())
                    }
                    ClassGetExpr::CGexpr(e2) => self.visit_expr(&mut (), e2),
                }
            }
        }
    }
}

impl<'ast, 'a> Visitor<'ast> for DeclvarVisitor<'a> {
    type Params = AstParams<(), String>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
        self
    }

    fn visit_stmt_(&mut self, env: &mut (), s: &Stmt_) -> Result<(), String> {
        match s {
            Stmt_::Try(x) => {
                let (body, catch_list, finally) = (&x.0, &x.1, &x.2);
                visit(self, env, body)?;
                for Catch(_, id, catch_body) in catch_list {
                    self.add_local(id.name());
                    visit(self, env, catch_body)?;
                }
                visit(self, env, finally)
            }
            Stmt_::DeclareLocal(box (id, _, expr)) => {
                self.add_local(id.name());
                visit(self, env, expr)
            }
            _ => s.recurse(env, self),
        }
    }

    fn visit_expr_(&mut self, env: &mut (), e: &Expr_) -> Result<(), String> {
        use aast::Expr_;
        match e {
            Expr_::Binop(box Binop { bop, lhs, rhs }) => {
                match (bop, &rhs.2) {
                    (Bop::Eq(_), Expr_::Await(_)) | (Bop::Eq(_), Expr_::Yield(_)) => {
                        // Visit e2 before e1. The ordering of declvars in async
                        // expressions matters to HHVM. See D5674623.
                        self.visit_expr(env, rhs)?;
                        self.visit_expr(env, lhs)
                    }
                    _ => e.recurse(env, self.object()),
                }
            }

            Expr_::Lvar(x) => {
                self.add_local(x.name());
                Ok(())
            }

            // For an Lfun, we don't want to recurse, because it's a separate scope.
            Expr_::Lfun(box lfun) => {
                if self.context.capture_debugger_vars {
                    visit(self, env, &lfun.0.body)?;
                }
                Ok(())
            }

            Expr_::Efun(box efun) => {
                // at this point AST is already rewritten so use lists on EFun nodes
                // contain list of captured variables. However if use list was initially absent
                // it is not correct to traverse such nodes to collect locals because it will impact
                // the order of locals in generated .declvars section:
                // // .declvars $a, $c, $b
                // $a = () => { $b = 1 };
                // $c = 1;
                // $b = 2;
                // // .declvars $a, $b, $c
                // $a = function () use ($b) => { $b = 1 };
                // $c = 1;
                // $b = 2;
                //
                // 'explicit_use_set' is used to in order to avoid synthesized use list
                let use_list = &efun.use_;
                let fn_name = &efun.closure_class_name.clone().unwrap_or_default();
                let has_use_list = self
                    .context
                    .explicit_use_set_opt
                    .map_or(false, |s| s.contains(fn_name));
                if self.context.capture_debugger_vars || has_use_list {
                    use_list.iter().for_each(|id| self.add_local(id.1.name()));
                }
                Ok(())
            }

            Expr_::Call(box aast::CallExpr {
                func,
                args,
                unpacked_arg,
                ..
            }) => {
                match &func.2 {
                    Expr_::Id(box ast_defs::Id(_, call_name)) => {
                        if call_name == emitter_special_functions::SET_FRAME_METADATA {
                            self.add_local("$86metadata");
                        }
                        if call_name == emitter_special_functions::SET_PRODUCT_ATTRIBUTION_ID
                            || call_name
                                == emitter_special_functions::SET_PRODUCT_ATTRIBUTION_ID_DEFERRED
                        {
                            self.add_local("$86productAttributionData");
                        }
                    }
                    Expr_::ClassGet(box (id, prop, pom)) if *pom == PropOrMethod::IsMethod => {
                        self.on_class_get(id, prop)?
                    }
                    _ => self.visit_expr(env, func)?,
                }
                // Calling convention doesn't matter here: we're just trying to figure out what
                // variables are declared in this scope.
                args.recurse(env, self.object())?;
                if let Some(arg) = unpacked_arg {
                    arg.recurse(env, self.object())?;
                }
                Ok(())
            }

            e => e.recurse(env, self.object()),
        }
    }

    fn visit_class_(&mut self, _: &mut (), _: &Class_) -> Result<(), String> {
        Ok(())
    }
    fn visit_fun_(&mut self, _: &mut (), _: &Fun_) -> Result<(), String> {
        Ok(())
    }
}

fn uls_from_ast<P, F1, F2>(
    params: &[P],
    get_param_name: F1,
    get_param_default_value: F2,
    explicit_use_set_opt: Option<&SSet>,
    body: &impl Node<AstParams<(), String>>,
    capture_debugger_vars: bool,
) -> Result<impl Iterator<Item = String>, String>
where
    F1: Fn(&P) -> &str,
    F2: Fn(&P) -> Maybe<&Expr>,
{
    let mut visitor = DeclvarVisitor::new(explicit_use_set_opt, capture_debugger_vars);

    for p in params {
        if let Just(e) = get_param_default_value(p) {
            visitor.visit_expr(&mut (), e)?;
        }
    }
    visit(&mut visitor, &mut (), body)?;
    for param in params {
        visitor.locals.shift_remove(get_param_name(param));
    }
    Ok(visitor.locals.into_iter())
}

pub fn from_ast<'arena>(
    params: &[(Param<'arena>, Option<(Label, Expr)>)],
    body: &[Stmt],
    explicit_use_set: &SSet,
) -> Result<Vec<String>, String> {
    let decl_vars = uls_from_ast(
        params,
        |(param, _)| param.name.unsafe_as_str(),
        |(_, default_value)| Maybe::from(default_value.as_ref().map(|x| &x.1)),
        Some(explicit_use_set),
        &body,
        false,
    )?;
    Ok(decl_vars.collect())
}

pub fn vars_from_ast(
    params: &[FunParam],
    body: &impl Node<AstParams<(), String>>,
    capture_debugger_vars: bool,
) -> Result<IndexSet<String>, String> {
    let decl_vars = uls_from_ast(
        params,
        |p| &p.name,                      // get_param_name
        |p| Maybe::from(p.expr.as_ref()), // get_param_default_value
        None,                             // explicit_use_set_opt
        body,
        capture_debugger_vars,
    )?;
    Ok(decl_vars.collect())
}
