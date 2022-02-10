// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_body::AstBody;
use ffi::{Just, Maybe};
use hash::HashSet;
use hhas_param::HhasParam;
use label::Label;
use naming_special_names_rust::{emitter_special_functions, special_idents};
use oxidized::{
    aast,
    aast_visitor::{visit, AstParams, Node, Visitor},
    ast::*,
};
use unique_id_builder::SSet;

use unique_list::UniqueList;

struct DeclvarVisitorContext<'a> {
    explicit_use_set_opt: Option<&'a SSet>,
}

struct DeclvarVisitor<'a> {
    // set of locals used inside the functions
    locals: UniqueList<String>,
    context: DeclvarVisitorContext<'a>,
}

impl<'a> DeclvarVisitor<'a> {
    fn new(explicit_use_set_opt: Option<&'a SSet>) -> Self {
        Self {
            locals: UniqueList::new(),
            context: DeclvarVisitorContext {
                explicit_use_set_opt,
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
            self.locals.add(name.into())
        }
    }

    fn on_class_get(
        &mut self,
        cid: &ClassId,
        cge: &ClassGetExpr,
        is_call_target: bool,
    ) -> Result<(), String> {
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
                        if is_call_target {
                            self.add_local(&pstr.1)
                        }
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
            _ => s.recurse(env, self),
        }
    }

    fn visit_expr_(&mut self, env: &mut (), e: &Expr_) -> Result<(), String> {
        use aast::Expr_;
        match e {
            Expr_::ObjGet(x) if x.as_ref().3 == PropOrMethod::IsProp => {
                let (receiver_e, prop_e) = (&x.0, &x.1);
                match &receiver_e.2 {
                    Expr_::Lvar(id) if id.name() == "$this" => {}
                    _ => receiver_e.recurse(env, self.object())?,
                }
                match &prop_e.2 {
                    Expr_::Lvar(id) => self.add_local(id.name()),
                    _ => prop_e.recurse(env, self.object())?,
                }
                Ok(())
            }

            Expr_::Binop(x) => {
                let (binop, e1, e2) = (&x.0, &x.1, &x.2);
                match (binop, &e2.2) {
                    (Bop::Eq(_), Expr_::Await(_)) | (Bop::Eq(_), Expr_::Yield(_)) => {
                        // Visit e2 before e1. The ordering of declvars in async
                        // expressions matters to HHVM. See D5674623.
                        self.visit_expr(env, e2)?;
                        self.visit_expr(env, e1)
                    }
                    _ => e.recurse(env, self.object()),
                }
            }

            Expr_::Lvar(x) => Ok(self.add_local(x.name())),
            Expr_::ClassGet(x) if x.as_ref().2 == PropOrMethod::IsProp => {
                self.on_class_get(&x.0, &x.1, false)
            }
            // For an Lfun, we don't want to recurse, because it's a separate scope.
            Expr_::Lfun(_) => Ok(()),
            Expr_::Efun(x) => {
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
                let (fun_, use_list) = (&x.0, &x.1);
                let fn_name = &fun_.name.1;
                let has_use_list = self
                    .context
                    .explicit_use_set_opt
                    .map_or(false, |s| s.contains(fn_name));
                if has_use_list {
                    for id in use_list {
                        self.add_local(id.name())
                    }
                }
                Ok(())
            }
            Expr_::Call(x) => {
                let (func_e, pos_args, unpacked_arg) = (&x.0, &x.2, &x.3);
                if let Expr_::Id(x) = &func_e.2 {
                    let call_name = &x.1;
                    if call_name == emitter_special_functions::SET_FRAME_METADATA {
                        self.add_local("$86metadata");
                    }
                }
                let on_arg = |self_: &mut Self, env: &mut (), x: &Expr| match &x.2 {
                    // Only add $this to locals if it's bare
                    Expr_::Lvar(id) if &(id.1).1 == "$this" => Ok(()),
                    _ => self_.visit_expr(env, x),
                };
                match &func_e.2 {
                    Expr_::ClassGet(x) if x.as_ref().2 == PropOrMethod::IsMethod => {
                        let (id, prop) = (&x.0, &x.1);
                        self.on_class_get(id, prop, true)?
                    }
                    _ => self.visit_expr(env, func_e)?,
                }
                // Calling convention doesn't matter here: we're just trying to figure out what
                // variables are declared in this scope.
                for (_, arg) in pos_args {
                    on_arg(self, env, arg)?
                }
                if let Some(arg) = unpacked_arg {
                    on_arg(self, env, arg)?
                }
                Ok(())
            }
            Expr_::New(x) => {
                let (exprs1, expr2) = (&x.2, &x.3);

                let add_bare_expr = |self_: &mut Self, env: &mut (), expr: &Expr| match &expr.2 {
                    Expr_::Lvar(x) if &(x.1).1 == "$this" => Ok(()),
                    _ => self_.visit_expr(env, expr),
                };
                for expr in exprs1 {
                    add_bare_expr(self, env, expr)?;
                }
                if let Some(expr) = expr2 {
                    add_bare_expr(self, env, expr)?
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
    b: &AstBody<'_>,
) -> Result<impl Iterator<Item = String>, String>
where
    F1: Fn(&P) -> &str,
    F2: Fn(&P) -> Maybe<&Expr>,
{
    let mut visitor = DeclvarVisitor::new(explicit_use_set_opt);

    for p in params {
        if let Just(e) = get_param_default_value(p) {
            visitor.visit_expr(&mut (), e)?;
        }
    }
    visit(&mut visitor, &mut (), b)?;
    for param in params {
        visitor.locals.remove(get_param_name(param))
    }
    Ok(visitor.locals.into_iter())
}

pub fn from_ast<'arena>(
    params: &[(HhasParam<'arena>, Option<(Label, Expr)>)],
    body: &AstBody<'_>,
    explicit_use_set: &SSet,
) -> Result<Vec<String>, String> {
    let decl_vars = uls_from_ast(
        params,
        |(param, _)| param.name.unsafe_as_str(),
        |(_, default_value)| Maybe::from(default_value.as_ref().map(|x| &x.1)),
        Some(explicit_use_set),
        body,
    )?;
    Ok(decl_vars.collect())
}

pub fn vars_from_ast(params: &[FunParam], b: &AstBody<'_>) -> Result<HashSet<String>, String> {
    let decl_vars = uls_from_ast(
        params,
        |p| &p.name,                      // get_param_name
        |p| Maybe::from(p.expr.as_ref()), // get_param_default_value
        None,                             // explicit_use_set_opt
        b,
    )?;
    Ok(decl_vars.collect())
}
