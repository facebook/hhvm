// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(box_patterns)]

use std::sync::Arc;

use core_utils_rust as core_utils;
use hash::HashSet;
use namespaces_rust as namespaces;
use naming_special_names_rust as sn;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::NodeMut;
use oxidized::aast_visitor::VisitorMut;
use oxidized::ast::*;
use oxidized::namespace_env;

fn is_special_identifier(name: &str) -> bool {
    use lazy_static::lazy_static;

    lazy_static! {
        static ref SPECIAL_IDENTIFIERS: HashSet<&'static str> = vec![
            sn::members::M_CLASS,
            sn::classes::PARENT,
            sn::classes::SELF,
            sn::classes::STATIC,
            sn::special_idents::THIS,
            sn::special_idents::DOLLAR_DOLLAR,
            sn::typehints::WILDCARD,
        ]
        .into_iter()
        .collect();
    }

    SPECIAL_IDENTIFIERS.contains(&name)
}

#[derive(Clone)]
struct Env {
    // TODO(hrust) I wanted to make namespace and type_params str's references but couldn't
    // find a way to specify that the lifetimes of these outlived the node I was taking them from
    namespace: std::sync::Arc<namespace_env::Env>,
    type_params: HashSet<String>,
}

impl Env {
    fn make(namespace: std::sync::Arc<namespace_env::Env>) -> Self {
        Self {
            namespace,
            type_params: HashSet::default(),
        }
    }

    // TODO: While elaboration for codegen and typing is similar, there are currently a
    // couple differences between the two and are toggled by this flag (XHP).
    // It would be nice to eventually eliminate the discrepancies between the two.
    pub fn in_codegen(&self) -> bool {
        self.namespace.is_codegen
    }

    fn extend_tparams(&mut self, tparaml: &[Tparam]) {
        for tparam in tparaml {
            self.type_params.insert(tparam.name.1.clone());
        }
    }

    fn elaborate_type_name(&self, id: &mut Id) {
        let name = &id.1;
        if !self.type_params.contains::<str>(name)
            && !is_special_identifier(name)
            && !name.starts_with('$')
        {
            namespaces::elaborate_id(&self.namespace, namespaces::ElaborateKind::Class, id);
        }
    }

    fn handle_special_calls(&self, call: &mut Expr_) {
        if let Expr_::Call(box CallExpr {
            func: Expr(_, _, Expr_::Id(id)),
            args,
            ..
        }) = call
        {
            if !self.in_codegen()
                && args.len() == 2
                && id.1 == sn::autoimported_functions::METH_CALLER
            {
                if let Expr(_, p, Expr_::String(fn_name)) = &args[0].1 {
                    let fn_name = core_utils::add_ns_bstr(fn_name);
                    args[0].1 = Expr((), p.clone(), Expr_::String(fn_name.into_owned().into()));
                }
            }
        }
    }

    fn with_ns(&self, namespace: Arc<namespace_env::Env>) -> Self {
        Self {
            namespace,
            type_params: self.type_params.clone(),
        }
    }
}

fn contexts_ns() -> Arc<namespace_env::Env> {
    Arc::new(namespace_env::Env {
        name: Some(sn::coeffects::CONTEXTS.to_owned()),
        ..namespace_env::Env::empty(
            vec![],
            // These default values seem obviously wrong, but they're what's
            // used in the OCaml visitor at the moment. Since we don't elaborate
            // context names in codegen, perhaps these settings don't end up
            // being relevant.
            oxidized::global_options::GlobalOptions::default().po_codegen,
            oxidized::global_options::GlobalOptions::default().po_disable_xhp_element_mangling,
        )
    })
}

fn unsafe_contexts_ns() -> Arc<namespace_env::Env> {
    Arc::new(namespace_env::Env {
        name: Some(sn::coeffects::UNSAFE_CONTEXTS.to_owned()),
        ..namespace_env::Env::empty(
            vec![],
            // As above, these look wrong, but may be irrelevant.
            oxidized::global_options::GlobalOptions::default().po_codegen,
            oxidized::global_options::GlobalOptions::default().po_disable_xhp_element_mangling,
        )
    })
}

fn is_reserved_type_hint(name: &str) -> bool {
    let base_name = core_utils::strip_ns(name);
    sn::typehints::is_reserved_type_hint(base_name)
}

struct ElaborateNamespacesVisitor {}

impl ElaborateNamespacesVisitor {
    fn on_class_ctx_const(&mut self, env: &mut Env, kind: &mut ClassTypeconst) -> Result<(), ()> {
        match kind {
            ClassTypeconst::TCConcrete(ClassConcreteTypeconst { c_tc_type }) => {
                self.on_ctx_hint_ns(&contexts_ns(), env, c_tc_type)
            }
            ClassTypeconst::TCAbstract(ClassAbstractTypeconst {
                as_constraint,
                super_constraint,
                default,
            }) => {
                let ctx_ns = contexts_ns();
                if let Some(as_constraint) = as_constraint {
                    self.on_ctx_hint_ns(&ctx_ns, env, as_constraint)?;
                }
                if let Some(super_constraint) = super_constraint {
                    self.on_ctx_hint_ns(&ctx_ns, env, super_constraint)?;
                }
                if let Some(default) = default {
                    self.on_ctx_hint_ns(&ctx_ns, env, default)?;
                }
                Ok(())
            }
        }
    }

    fn on_contexts_ns(
        &mut self,
        ctx_ns: &Arc<namespace_env::Env>,
        env: &mut Env,
        ctxs: &mut Contexts,
    ) -> Result<(), ()> {
        for ctx in &mut ctxs.1 {
            self.on_ctx_hint_ns(ctx_ns, env, ctx)?;
        }
        Ok(())
    }

    fn on_ctx_hint_ns(
        &mut self,
        ctx_ns: &Arc<namespace_env::Env>,
        env: &mut Env,
        h: &mut Context,
    ) -> Result<(), ()> {
        let ctx_env = &mut env.with_ns(Arc::clone(ctx_ns));
        fn is_ctx_user_defined(ctx: &str) -> bool {
            match ctx.rsplit('\\').next() {
                Some(ctx_name) if !ctx_name.is_empty() => {
                    ctx_name.chars().next().unwrap().is_uppercase()
                }
                _ => false,
            }
        }
        match h {
            Hint(_, box Hint_::Happly(ctx, hl)) if !is_reserved_type_hint(&ctx.1) => {
                if is_ctx_user_defined(&ctx.1) {
                    env.elaborate_type_name(ctx)
                } else {
                    ctx_env.elaborate_type_name(ctx)
                };
                hl.recurse(env, self.object())?;
            }
            Hint(_, box Hint_::Hintersection(ctxs)) => {
                for ctx in ctxs {
                    self.on_ctx_hint_ns(ctx_ns, env, ctx)?;
                }
            }
            Hint(_, box Hint_::Haccess(root, _)) => root.recurse(env, self.object())?,
            _ => h.recurse(ctx_env, self.object())?,
        }
        Ok(())
    }
}

impl<'ast> VisitorMut<'ast> for ElaborateNamespacesVisitor {
    type Params = AstParams<Env, ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, Params = Self::Params> {
        self
    }

    // Namespaces were already precomputed by ElaborateDefs
    // The following functions just set the namespace env correctly
    fn visit_class_(&mut self, env: &mut Env, cd: &mut Class_) -> Result<(), ()> {
        let env = &mut env.clone();
        env.namespace = Arc::clone(&cd.namespace);
        env.extend_tparams(&cd.tparams);
        cd.recurse(env, self.object())
    }

    fn visit_ctx_refinement(&mut self, env: &mut Env, cr: &mut CtxRefinement) -> Result<(), ()> {
        if env.in_codegen() {
            return cr.recurse(env, self.object());
        }
        match cr {
            CtxRefinement::CRexact(h) => self.on_ctx_hint_ns(&contexts_ns(), env, h),
            CtxRefinement::CRloose(CtxRefinementBounds { lower, upper }) => {
                let ctx_ns = contexts_ns();
                if let Some(lower) = lower {
                    self.on_ctx_hint_ns(&ctx_ns, env, lower)?;
                }
                if let Some(upper) = upper {
                    self.on_ctx_hint_ns(&ctx_ns, env, upper)?;
                }
                Ok(())
            }
        }
    }

    fn visit_class_typeconst_def(
        &mut self,
        env: &mut Env,
        tc: &mut ClassTypeconstDef,
    ) -> Result<(), ()> {
        if !env.in_codegen() && tc.is_ctx {
            self.on_class_ctx_const(env, &mut tc.kind)?;
        }
        tc.recurse(env, self.object())
    }

    fn visit_typedef(&mut self, env: &mut Env, td: &mut Typedef) -> Result<(), ()> {
        let env = &mut env.clone();
        env.namespace = Arc::clone(&td.namespace);
        env.extend_tparams(&td.tparams);
        if !env.in_codegen() && td.is_ctx {
            let ctx_ns = &contexts_ns();
            if let Some(as_constraint) = &mut td.as_constraint {
                self.on_ctx_hint_ns(ctx_ns, env, as_constraint)?;
            }
            if let Some(super_constraint) = &mut td.super_constraint {
                self.on_ctx_hint_ns(ctx_ns, env, super_constraint)?;
            }
            self.on_ctx_hint_ns(ctx_ns, env, &mut td.kind)?;
        }
        td.recurse(env, self.object())
    }

    fn visit_def(&mut self, env: &mut Env, def: &mut Def) -> Result<(), ()> {
        if let Def::SetNamespaceEnv(nsenv) = def {
            env.namespace = Arc::clone(nsenv);
        }
        def.recurse(env, self.object())
    }

    // Difference between FunDef and Fun_ is that Fun_ is also lambdas
    fn visit_fun_def(&mut self, env: &mut Env, fd: &mut FunDef) -> Result<(), ()> {
        let env = &mut env.clone();
        env.namespace = Arc::clone(&fd.namespace);
        env.extend_tparams(&fd.tparams);
        fd.recurse(env, self.object())
    }

    fn visit_fun_(&mut self, env: &mut Env, f: &mut Fun_) -> Result<(), ()> {
        if let Some(ctxs) = &mut f.ctxs {
            self.on_contexts_ns(&contexts_ns(), env, ctxs)?;
        }
        if let Some(ctxs) = &mut f.unsafe_ctxs {
            self.on_contexts_ns(&unsafe_contexts_ns(), env, ctxs)?;
        }
        f.recurse(env, self.object())
    }

    fn visit_method_(&mut self, env: &mut Env, m: &mut Method_) -> Result<(), ()> {
        let env = &mut env.clone();
        env.extend_tparams(&m.tparams);
        if let Some(ctxs) = &mut m.ctxs {
            self.on_contexts_ns(&contexts_ns(), env, ctxs)?;
        }
        if let Some(ctxs) = &mut m.unsafe_ctxs {
            self.on_contexts_ns(&unsafe_contexts_ns(), env, ctxs)?;
        }
        m.recurse(env, self.object())
    }

    fn visit_tparam(&mut self, env: &mut Env, tparam: &mut Tparam) -> Result<(), ()> {
        let env = &mut env.clone();
        env.extend_tparams(&tparam.parameters);
        tparam.recurse(env, self.object())
    }

    fn visit_gconst(&mut self, env: &mut Env, gc: &mut Gconst) -> Result<(), ()> {
        let env = &mut env.clone();
        env.namespace = Arc::clone(&gc.namespace);
        gc.recurse(env, self.object())
    }

    fn visit_file_attribute(&mut self, env: &mut Env, fa: &mut FileAttribute) -> Result<(), ()> {
        let env = &mut env.clone();
        env.namespace = Arc::clone(&fa.namespace);
        fa.recurse(env, self.object())
    }

    // I don't think we need to visit blocks because we got rid of let bindings :)

    fn visit_catch(&mut self, env: &mut Env, catch: &mut Catch) -> Result<(), ()> {
        let exception_sid = &mut catch.0;
        env.elaborate_type_name(exception_sid);
        catch.recurse(env, self.object())
    }

    // I don't think we need to visit stmts because we got rid of let bindings :)

    // Lfun and Efun as Expr_:: nodes so update the env in visit_expr_

    // Actually rewrites the names
    fn visit_expr_(&mut self, env: &mut Env, e: &mut Expr_) -> Result<(), ()> {
        // Sets env for lambdas
        match e {
            Expr_::Collection(box (id, c_targ_opt, flds)) if !env.in_codegen() => {
                namespaces::elaborate_id(&env.namespace, namespaces::ElaborateKind::Class, id);
                c_targ_opt.accept(env, self.object())?;
                flds.accept(env, self.object())?;
            }
            Expr_::Call(box CallExpr {
                func,
                targs,
                args,
                unpacked_arg,
            }) => {
                // Recurse first due to borrow order
                targs.accept(env, self.object())?;
                args.accept(env, self.object())?;
                unpacked_arg.accept(env, self.object())?;

                if let Some(sid) = func.2.as_id_mut() {
                    if !sn::special_functions::is_special_function(&sid.1) {
                        namespaces::elaborate_id(
                            &env.namespace,
                            namespaces::ElaborateKind::Fun,
                            sid,
                        );
                        env.handle_special_calls(e);
                    }
                } else {
                    func.accept(env, self.object())?;
                }
            }
            Expr_::FunctionPointer(box (fpid, targs)) => {
                if let Some(sid) = fpid.as_fpid_mut() {
                    namespaces::elaborate_id(&env.namespace, namespaces::ElaborateKind::Fun, sid);
                } else if let Some(cc) = fpid.as_fpclass_const_mut() {
                    let type_ = cc.0;
                    if let Some(e) = type_.2.as_ciexpr_mut() {
                        if let Some(sid) = e.2.as_id_mut() {
                            env.elaborate_type_name(sid);
                        } else {
                            e.accept(env, self.object())?;
                        }
                    }
                } else {
                    fpid.accept(env, self.object())?;
                }
                targs.accept(env, self.object())?;
            }
            Expr_::ObjGet(box (obj, expr, nullsafe, _)) => {
                if let Expr_::Id(..) = expr.2 {
                } else {
                    expr.accept(env, self.object())?;
                }

                obj.accept(env, self.object())?;
                nullsafe.accept(env, self.object())?;
            }
            Expr_::Id(sid) if !((sid.1 == "NAN" || sid.1 == "INF") && env.in_codegen()) => {
                namespaces::elaborate_id(&env.namespace, namespaces::ElaborateKind::Const, sid);
            }
            Expr_::New(box (class_id, targs, args, unpacked_el, _)) => {
                if let Some(e) = class_id.2.as_ciexpr_mut() {
                    if let Some(sid) = e.2.as_id_mut() {
                        env.elaborate_type_name(sid);
                    } else {
                        e.accept(env, self.object())?;
                    }
                } else {
                    class_id.accept(env, self.object())?;
                }
                targs.accept(env, self.object())?;
                args.accept(env, self.object())?;
                unpacked_el.accept(env, self.object())?;
            }
            Expr_::ClassConst(box (type_, _)) => {
                if let Some(e) = type_.2.as_ciexpr_mut() {
                    if let Some(sid) = e.2.as_id_mut() {
                        env.elaborate_type_name(sid);
                    } else {
                        e.accept(env, self.object())?;
                    }
                } else {
                    type_.accept(env, self.object())?;
                }
            }
            Expr_::ClassGet(box (class_id, class_get_expr, _)) => {
                if let Some(e) = class_id.2.as_ciexpr_mut() {
                    if let Some(sid) = e.2.as_id_mut() {
                        env.elaborate_type_name(sid);
                    } else {
                        e.accept(env, self.object())?;
                    }
                } else {
                    class_id.accept(env, self.object())?;
                }
                class_get_expr.accept(env, self.object())?;
            }
            Expr_::Xml(box (xml_id, attributes, el)) => {
                /* if XHP element mangling is disabled, namespaces are supported */
                if !env.in_codegen() || env.namespace.disable_xhp_element_mangling {
                    env.elaborate_type_name(xml_id);
                }
                attributes.recurse(env, self.object())?;
                el.recurse(env, self.object())?;
            }
            Expr_::EnumClassLabel(box (Some(sid), _)) if !env.in_codegen() => {
                env.elaborate_type_name(sid);
            }
            Expr_::Nameof(box target) => {
                if let Some(e) = target.2.as_ciexpr_mut() {
                    if let Some(sid) = e.2.as_id_mut() {
                        env.elaborate_type_name(sid);
                    } else {
                        e.accept(env, self.object())?;
                    }
                }
            }
            _ => e.recurse(env, self.object())?,
        }
        Ok(())
    }

    fn visit_hint_(&mut self, env: &mut Env, hint: &mut Hint_) -> Result<(), ()> {
        fn is_xhp_screwup(x: &str) -> bool {
            x == "Xhp" || x == ":Xhp" || x == "XHP"
        }
        match hint {
            Hint_::Happly(sid, _) if is_xhp_screwup(&sid.1) => {}
            Hint_::Happly(sid, _) if is_reserved_type_hint(&sid.1) && !env.in_codegen() => {}
            Hint_::Happly(sid, _) => {
                env.elaborate_type_name(sid);
            }
            _ => {}
        }
        hint.recurse(env, self.object())
    }

    fn visit_hint_fun(&mut self, env: &mut Env, hf: &mut HintFun) -> Result<(), ()> {
        if let Some(ctxs) = &mut hf.ctxs {
            self.on_contexts_ns(&contexts_ns(), env, ctxs)?;
        }
        hf.recurse(env, self.object())
    }

    fn visit_shape_field_name(
        &mut self,
        env: &mut Env,
        sfn: &mut ShapeFieldName,
    ) -> Result<(), ()> {
        match sfn {
            ShapeFieldName::SFclassConst(id, _) => {
                env.elaborate_type_name(id);
            }
            _ => {}
        }
        sfn.recurse(env, self.object())
    }

    fn visit_user_attribute(&mut self, env: &mut Env, ua: &mut UserAttribute) -> Result<(), ()> {
        if !sn::user_attributes::is_reserved(&ua.name.1) {
            env.elaborate_type_name(&mut ua.name);
        }
        ua.recurse(env, self.object())
    }

    fn visit_xhp_child(&mut self, env: &mut Env, child: &mut XhpChild) -> Result<(), ()> {
        match child {
            XhpChild::ChildName(sid)
                if !env.in_codegen()
                    && !sn::xhp::is_reserved(&sid.1)
                    && !sn::xhp::is_xhp_category(&sid.1) =>
            {
                env.elaborate_type_name(sid);
            }
            _ => {}
        }
        child.recurse(env, self.object())
    }
}

pub fn elaborate_program(e: std::sync::Arc<namespace_env::Env>, defs: &mut Program) {
    let mut env = Env::make(e);
    let mut visitor = ElaborateNamespacesVisitor {};
    for def in defs {
        visitor.visit_def(&mut env, def).unwrap();
    }
}

pub fn elaborate_fun_def(e: std::sync::Arc<namespace_env::Env>, fd: &mut FunDef) {
    let mut env = Env::make(e);
    let mut visitor = ElaborateNamespacesVisitor {};
    visitor.visit_fun_def(&mut env, fd).unwrap();
}

pub fn elaborate_class_(e: std::sync::Arc<namespace_env::Env>, c: &mut Class_) {
    let mut env = Env::make(e);
    let mut visitor = ElaborateNamespacesVisitor {};
    visitor.visit_class_(&mut env, c).unwrap();
}

pub fn elaborate_module_def(e: std::sync::Arc<namespace_env::Env>, m: &mut ModuleDef) {
    let mut env = Env::make(e);
    let mut visitor = ElaborateNamespacesVisitor {};
    visitor.visit_module_def(&mut env, m).unwrap();
}

pub fn elaborate_gconst(e: std::sync::Arc<namespace_env::Env>, cst: &mut Gconst) {
    let mut env = Env::make(e);
    let mut visitor = ElaborateNamespacesVisitor {};
    visitor.visit_gconst(&mut env, cst).unwrap();
}

pub fn elaborate_typedef(e: std::sync::Arc<namespace_env::Env>, td: &mut Typedef) {
    let mut env = Env::make(e);
    let mut visitor = ElaborateNamespacesVisitor {};
    visitor.visit_typedef(&mut env, td).unwrap();
}

pub fn elaborate_stmt(e: std::sync::Arc<namespace_env::Env>, stmt: &mut Stmt) {
    let mut env = Env::make(e);
    let mut visitor = ElaborateNamespacesVisitor {};
    visitor.visit_stmt(&mut env, stmt).unwrap();
}
