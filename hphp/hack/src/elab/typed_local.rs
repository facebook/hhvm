// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::BTreeMap;
use std::collections::HashSet;

#[allow(unused_imports)]
use hack_macros::hack_stmts;
use nast::AsExpr;
use nast::Binop;
use nast::Block;
use nast::CaptureLid;
use nast::Expr;
use nast::Expr_;
use nast::Hint;
use nast::Hint_;
use nast::Lid;
use nast::NastShapeInfo;
use nast::Pos;
use nast::ReifyKind;
use nast::ShapeFieldInfo;
use nast::Stmt;
use nast::Stmt_;
use nast::UsingStmt;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::NodeMut;
use oxidized::aast_visitor::VisitorMut;
use oxidized::ast_defs::Bop;
use oxidized::local_id;
use oxidized::naming_error::NamingError;
use oxidized::nast;

use crate::Env;

#[derive(Debug, Clone, Default)]
pub struct TypedLocal {
    // The locals in scope. The hint is present iff this is a typed local.
    // Union at join points to keep the locals that might be in scope.
    // The position is where the local was first declared. The bool if true
    // if the hint could potentially be a shape or a tuple. If there is an
    // assignment like `$x[e1] = e2` than we should enforce after (instead of on the rhs)
    // in case the assignment changed the shape or tuple type at runtime. Shapes and tuples are
    // special because their contents are enforced.
    locals: BTreeMap<String, (Option<(Hint, bool)>, Pos)>,
    // The subset of typed locals declared in the current scope, rather than preceding it.
    // declared_ids and assigned_ids should be disjoint
    declared_ids: HashSet<String>,
    // The subset of untyped locals first assigned in the current scope, rather than preceding it.
    assigned_ids: HashSet<String>,
    // The in-scope erased generics
    erased_generics: HashSet<String>,
    should_elab: bool,
}

impl TypedLocal {
    fn add_local(&mut self, id: String, hint: Option<(Hint, bool)>, pos: &Pos) {
        self.locals.insert(id, (hint, pos.clone()));
    }

    fn add_assigned_id(&mut self, id: String) {
        self.assigned_ids.insert(id);
    }
    fn add_declared_id(&mut self, id: &str) {
        self.declared_ids.insert(id.to_string());
    }

    fn get_local(&self, id: &str) -> Option<&(Option<(Hint, bool)>, Pos)> {
        self.locals.get(id)
    }

    fn get_local_pos(&self, id: &str) -> Pos {
        if let Some((_, pos)) = self.get_local(id) {
            pos.clone()
        } else {
            Pos::NONE
        }
    }

    fn new_block_env(&self) -> Self {
        TypedLocal {
            locals: self.locals.clone(),
            erased_generics: self.erased_generics.clone(),
            should_elab: self.should_elab,
            ..Default::default()
        }
    }

    fn restrict_env(&self, ids: &Vec<CaptureLid>) -> TypedLocal {
        let mut new_env = TypedLocal {
            erased_generics: self.erased_generics.clone(),
            should_elab: self.should_elab,
            ..Default::default()
        };
        for cid in ids {
            let id = &cid.1.1.1;
            if let Some((hint, pos)) = self.get_local(id) {
                new_env.add_local(id.to_string(), hint.clone(), pos);
            }
            if self.declared_ids.contains(id) {
                new_env.add_declared_id(id);
            }
            if self.assigned_ids.contains(id) {
                new_env.add_assigned_id(id.clone());
            }
        }
        new_env
    }

    fn clear(&mut self) {
        self.assigned_ids.clear();
        self.declared_ids.clear();
        self.locals.clear();
    }

    // Add the new_env into self, updating the error map
    fn join2(&mut self, new_env: &mut Self, error_map: &mut BTreeMap<String, (Pos, Pos)>) {
        for id in new_env.assigned_ids.intersection(&self.declared_ids) {
            let assign_pos = new_env.get_local_pos(id);
            let decl_pos = self.get_local_pos(id);
            error_map.insert(id.clone(), (decl_pos, assign_pos));
        }
        for id in new_env.declared_ids.intersection(&self.assigned_ids) {
            let assign_pos = self.get_local_pos(id);
            let decl_pos = new_env.get_local_pos(id);
            error_map.insert(id.clone(), (decl_pos, assign_pos));
        }
        for id in new_env.declared_ids.intersection(&self.declared_ids) {
            let assign_pos = self.get_local_pos(id);
            let decl_pos = new_env.get_local_pos(id);
            error_map.insert(id.clone(), (decl_pos, assign_pos));
        }
        // self.locals contains the cumulative bindings down the self branch.
        // For the bindings newly defined in new_env branch, insert them into
        // the self locals if they aren't already declared there
        let diff = new_env.assigned_ids.difference(&self.declared_ids);
        for id in new_env.declared_ids.iter().chain(diff) {
            if let Some(x) = new_env.get_local(id) {
                self.locals.insert(id.to_string(), x.clone());
            }
        }
        let declared_ids = std::mem::take(&mut new_env.declared_ids);
        let assigned_ids = std::mem::take(&mut new_env.assigned_ids);
        self.declared_ids.extend(declared_ids);
        self.assigned_ids.extend(assigned_ids);
        // If the id was declared then we need to remove it from the assigned
        // list to keep them disjoint. This situation could arise when
        // self has a declaration and new_env and assignment (or vice versa).
        for id in &self.declared_ids {
            self.assigned_ids.remove(id);
        }
    }

    // join a list of TypedLocals together. self should be the TypedLocal from
    // before the split
    fn join(&mut self, envs: &mut [Self], env: &mut Env) {
        let mut error_map = BTreeMap::default();
        let before_declared_ids = std::mem::take(&mut self.declared_ids);
        let before_assigned_ids = std::mem::take(&mut self.assigned_ids);
        for env in envs.iter_mut().rev() {
            self.join2(env, &mut error_map);
        }
        for (id, (decl_pos, assign_pos)) in error_map {
            env.emit_error(NamingError::IllegalTypedLocal {
                join: true,
                id_pos: decl_pos,
                id_name: id.clone(),
                def_pos: assign_pos,
            })
        }
        self.declared_ids.extend(before_declared_ids);
        self.assigned_ids.extend(before_assigned_ids);
    }

    // find the hint for the id, or if it hasn't been assigned, mark it as assigned with None
    fn get_hint_or_add_assign(&mut self, lid: &Lid, pos: &Pos) -> Option<(Hint, bool)> {
        let name = local_id::get_name(&lid.1);
        if let Some((hint_opt, _pos)) = self.get_local(name) {
            hint_opt.clone()
        } else {
            self.add_local(name.to_string(), None, pos);
            self.add_assigned_id(name.to_string());
            None
        }
    }

    // Get the hint from a lvalue expression if it is a variable. Report an error
    // if the lvalue might need further processing, which is the case with list(..) and
    // $x[e] lvalues. Other lvalues won't need enforcement, so just return None
    fn get_lvar_hint(&mut self, expr: &Expr) -> Result<Option<(Hint, bool)>, ()> {
        match expr {
            Expr(_, pos, Expr_::Lvar(box lid)) => Ok(self.get_hint_or_add_assign(lid, pos)),
            Expr(_, _pos, Expr_::List(..)) | Expr(_, _pos, Expr_::ArrayGet(box (_, Some(_)))) => {
                Err(())
            }
            _ => Ok(None),
        }
    }

    fn wrap_rhs_with_as(&self, rhs: &mut Expr, hint: Hint, pos: &Pos) {
        if self.should_elab {
            let mut init_expr = Expr((), Pos::NONE, Expr_::Null);
            std::mem::swap(rhs, &mut init_expr);
            let as_expr_ = Expr_::As(Box::new((init_expr, hint, false)));
            let as_expr = Expr((), pos.clone(), as_expr_);
            *rhs = as_expr;
        }
    }

    // if the expression is an assignment where the type of the lhs can be enforced
    // on the rhs, update the expression. Otherwise if it's an assignment that will
    // need enforcement that cannot be done on the rhs, return an error
    fn enforce_assign_expr_rhs(&mut self, expr: &mut Expr) -> Result<(), ()> {
        match expr {
            Expr(
                (),
                pos,
                Expr_::Binop(box Binop {
                    bop: Bop::Eq(None),
                    lhs,
                    rhs,
                }),
            ) => match self.get_lvar_hint(lhs) {
                Ok(Some((hint, _))) => {
                    self.wrap_rhs_with_as(rhs, hint, pos);
                    Ok(())
                }
                Ok(None) => Ok(()),
                Err(()) => Err(()),
            },
            Expr(
                (),
                _pos,
                Expr_::Binop(box Binop {
                    bop: Bop::Eq(Some(_)),
                    lhs: _,
                    rhs: _,
                }),
            ) => Err(()),
            _ => Ok(()),
        }
    }

    // Collect all of the variables a lhs might need to enforce and put them in hints as 'as' expressions
    // These include list() variables, and $x[e] array index where the hint to enforce might be a shape or tuple
    fn get_vars_to_enforce_lhs(&mut self, expr: &Expr, in_array_get: bool, hints: &mut Vec<Expr>) {
        match expr {
            Expr(_, pos, Expr_::Lvar(box lid)) => {
                if let Some((hint, could_be_shape_or_tuple)) = self.get_hint_or_add_assign(lid, pos)
                {
                    if !in_array_get || could_be_shape_or_tuple {
                        let mut expr = expr.clone();
                        self.wrap_rhs_with_as(&mut expr, hint, pos);
                        hints.push(expr);
                    }
                }
            }
            Expr(_, _pos, Expr_::List(exprs)) => {
                for expr in exprs {
                    self.get_vars_to_enforce_lhs(expr, in_array_get, hints)
                }
            }
            Expr(_, _pos, Expr_::ArrayGet(box (lhs, Some(_)))) => {
                self.get_vars_to_enforce_lhs(lhs, true, hints)
            }
            _ => {}
        }
    }

    fn get_vars_to_enforce(&mut self, expr: &Expr, hints: &mut Vec<Expr>) {
        match expr {
            Expr(
                (),
                _pos,
                Expr_::Binop(box Binop {
                    bop: Bop::Eq(_),
                    lhs,
                    rhs: _,
                }),
            ) => self.get_vars_to_enforce_lhs(lhs, false, hints),
            _ => {}
        }
    }

    fn visit_fun_helper(&mut self, env: &mut Env, elem: &mut nast::Fun_) -> Result<(), ()> {
        for param in elem.params.iter() {
            self.add_local(param.name.clone(), None, &param.pos);
            self.add_assigned_id(param.name.clone());
        }
        elem.body.fb_ast.recurse(env, self.object())
    }

    fn add_enforcement_exprs_in_list(
        &mut self,
        env: &mut Env,
        init_exprs: &mut Vec<Expr>,
    ) -> Result<Vec<Expr>, ()> {
        let mut new_init_expr = vec![];
        for mut init_expr in init_exprs.drain(..) {
            init_expr.recurse(env, self.object())?;
            match self.enforce_assign_expr_rhs(&mut init_expr) {
                Ok(()) => new_init_expr.push(init_expr),
                Err(()) => {
                    let mut exprs = vec![];
                    self.get_vars_to_enforce(&init_expr, &mut exprs);
                    new_init_expr.push(init_expr);
                    if self.should_elab {
                        for expr in exprs.drain(..) {
                            new_init_expr.push(expr);
                        }
                    }
                }
            }
        }
        Ok(new_init_expr)
    }

    // Replaces definitely unenforceable parts of the hint with _
    fn simplify_hint(&self, hint: &mut Hint) -> bool {
        let Hint(_, box hint_) = hint;
        match hint_ {
            Hint_::Hoption(h) | Hint_::Hlike(h) | Hint_::HclassArgs(h) => self.simplify_hint(h),
            Hint_::Htuple(hints) => {
                for h in hints {
                    self.simplify_hint(h);
                }
                true
            }
            Hint_::Happly(cn, hints) => match cn.1.as_str() {
                "\\HH\\void"
                | "\\HH\\int"
                | "\\HH\\bool"
                | "\\HH\\float"
                | "\\HH\\string"
                | "\\HH\\resource"
                | "\\HH\\num"
                | "\\HH\\noreturn"
                | "\\HH\\arraykey"
                | "\\HH\\mixed"
                | "\\HH\\dict"
                | "\\HH\\vec"
                | "\\HH\\keyset"
                | "\\HH\\vec_or_dict"
                | "\\HH\\nonnull"
                | "\\HH\\darray"
                | "\\HH\\varray"
                | "\\HH\\varray_or_darray"
                | "\\HH\\anyarray"
                | "\\HH\\null"
                | "\\HH\\nothing" => {
                    if self.should_elab {
                        for Hint(_, box hint_) in hints {
                            let _ = std::mem::replace(hint_, Hint_::Hwildcard);
                        }
                    }
                    false
                }
                "\\HH\\dynamic" => {
                    if self.should_elab {
                        let _ = std::mem::replace(hint_, Hint_::Hwildcard);
                    }
                    false
                }
                s => {
                    if self.erased_generics.contains(s) {
                        if self.should_elab {
                            let _ = std::mem::replace(hint_, Hint_::Hwildcard);
                        }
                        false
                    } else {
                        if self.should_elab {
                            for h in hints {
                                self.simplify_hint(h);
                            }
                        }
                        true
                    }
                }
            },
            Hint_::Hshape(NastShapeInfo {
                allows_unknown_fields: _,
                field_map,
            }) => {
                for ShapeFieldInfo {
                    optional: _,
                    hint,
                    name: _,
                } in field_map
                {
                    self.simplify_hint(hint);
                }
                true
            }
            Hint_::Haccess(h, _) => {
                self.simplify_hint(h);
                true
            }
            Hint_::HvecOrDict(None, Hint(_, box hint_)) => {
                if self.should_elab {
                    let _ = std::mem::replace(hint_, Hint_::Hwildcard);
                }
                false
            }
            Hint_::HvecOrDict(Some(Hint(_, box hint_1)), Hint(_, box hint_2)) => {
                if self.should_elab {
                    let _ = std::mem::replace(hint_1, Hint_::Hwildcard);
                    let _ = std::mem::replace(hint_2, Hint_::Hwildcard);
                }
                false
            }
            Hint_::Hnonnull | Hint_::Hprim(_) | Hint_::Hthis | Hint_::Hnothing | Hint_::Hmixed => {
                false
            }
            // The following are unenforced, so we replace with a wildcard
            Hint_::Hfun(_)
            | Hint_::Hany
            | Hint_::Herr
            | Hint_::Hwildcard
            | Hint_::Hdynamic
            | Hint_::Hunion(_)
            | Hint_::Hintersection(_)
            | Hint_::Hrefinement(_, _)
            | Hint_::Hsoft(_)
            | Hint_::HfunContext(_)
            | Hint_::Hvar(_)
            | Hint_::Habstr(_, _) => {
                if self.should_elab {
                    let _ = std::mem::replace(hint_, Hint_::Hwildcard);
                }
                false
            }
        }
    }
}

fn exprs_to_stmts(expr: Option<Expr>, mut exprs: Vec<Expr>) -> Vec<Stmt> {
    let mut stmts = vec![];
    if let Some(expr) = expr {
        stmts.push(Stmt(expr.1.clone(), Stmt_::Expr(Box::new(expr))));
    }
    for expr in exprs.drain(..) {
        stmts.push(Stmt(expr.1.clone(), Stmt_::Expr(Box::new(expr))));
    }
    stmts
}

fn add_to_block(mut stmts: Vec<Stmt>, block: &mut Block) {
    match block {
        Block(stmts2) => {
            stmts.append(stmts2);
            std::mem::swap(&mut stmts, stmts2);
        }
    }
}

impl<'a> VisitorMut<'a> for TypedLocal {
    type Params = AstParams<Env, ()>;
    fn object(&mut self) -> &mut dyn VisitorMut<'a, Params = Self::Params> {
        self
    }

    fn visit_stmt(&mut self, env: &mut Env, elem: &mut nast::Stmt) -> Result<(), ()> {
        let Stmt(pos, stmt_) = elem;
        match stmt_ {
            Stmt_::DeclareLocal(box (lid, hint, expr)) => {
                expr.recurse(env, self.object())?;
                let name = local_id::get_name(&lid.1);
                if let Some((_, def_pos)) = self.get_local(name) {
                    env.emit_error(NamingError::IllegalTypedLocal {
                        join: false,
                        id_pos: pos.clone(),
                        id_name: name.clone(),
                        def_pos: def_pos.clone(),
                    });
                } else {
                    let mut as_hint = Hint(Pos::NONE, Box::new(Hint_::Hnothing));
                    std::mem::swap(hint, &mut as_hint);
                    let maybe_shape_or_tuple = self.simplify_hint(&mut as_hint);
                    self.add_local(
                        name.to_string(),
                        Some((as_hint.clone(), maybe_shape_or_tuple)),
                        pos,
                    );
                    self.add_declared_id(name);
                    if self.should_elab && let Some(expr) = expr {
                        self.wrap_rhs_with_as(expr, as_hint, pos);
                        let mut init_lid = Lid(Pos::NONE, (0, "".to_string()));
                        std::mem::swap(&mut init_lid, lid);
                        let mut init_expr = Expr((), Pos::NONE, Expr_::Null);
                        std::mem::swap(expr, &mut init_expr);
                        let assign_expr_ = Expr_::Binop(Box::new(Binop {
                            bop: Bop::Eq(None),
                            lhs: Expr((), pos.clone(), Expr_::Lvar(Box::new(init_lid))),
                            rhs: init_expr,
                        }));
                        let assign_expr = Expr((), pos.clone(), assign_expr_);
                        *stmt_ = Stmt_::Expr(Box::new(assign_expr));
                    } else if self.should_elab {
                        *stmt_ = Stmt_::Noop;
                    } else{
                        std::mem::swap(hint, &mut as_hint);
                    }
                };
                Ok(())
            }
            Stmt_::Expr(box expr) => {
                expr.recurse(env, self.object())?;
                if let Err(()) = self.enforce_assign_expr_rhs(expr) {
                    let mut exprs = vec![];
                    self.get_vars_to_enforce(expr, &mut exprs);
                    if !exprs.is_empty() && self.should_elab {
                        let mut new_expr = Expr((), Pos::NONE, Expr_::Null);
                        std::mem::swap(expr, &mut new_expr);
                        let stmts = exprs_to_stmts(Some(new_expr), exprs);
                        let mut block = Stmt_::Block(Box::new((None, Block(stmts))));
                        std::mem::swap(stmt_, &mut block);
                    }
                }
                Ok(())
            }
            Stmt_::If(box (cond, then_block, else_block)) => {
                cond.recurse(env, self.object())?;
                let mut then_env = self.new_block_env();
                let mut else_env = self.new_block_env();
                then_block.recurse(env, then_env.object())?;
                else_block.recurse(env, else_env.object())?;
                self.join(&mut vec![then_env, else_env], env);
                Ok(())
            }
            Stmt_::For(box (init_exprs, cond, update_exprs, body)) => {
                let mut new_init_exprs = self.add_enforcement_exprs_in_list(env, init_exprs)?;
                std::mem::swap(&mut new_init_exprs, init_exprs);
                cond.recurse(env, self.object())?;
                body.recurse(env, self.object())?;
                let mut new_update_exprs = self.add_enforcement_exprs_in_list(env, update_exprs)?;
                std::mem::swap(&mut new_update_exprs, update_exprs);
                Ok(())
            }
            Stmt_::Using(box UsingStmt {
                is_block_scoped: _,
                has_await: _,
                exprs,
                block,
            }) => {
                for expr in exprs.1.iter_mut() {
                    expr.recurse(env, self.object())?;
                    if let Err(()) = self.enforce_assign_expr_rhs(expr) {
                        // This shouldn't happen. If the expression in the using is not
                        // an assignment to a variable, then the general prohibition on
                        // assignments in expressions should kick in.
                    }
                }
                block.recurse(env, self.object())
            }
            Stmt_::Switch(box (cond, cases, default)) => {
                cond.recurse(env, self.object())?;
                let mut envs = cases
                    .iter_mut()
                    .map(|nast::Case(ref mut expr, ref mut block)| {
                        let _ = expr.recurse(env, self.object());
                        let mut new_env = self.new_block_env();
                        let _ = block.recurse(env, new_env.object());
                        new_env
                    })
                    .collect::<Vec<_>>();
                if let Some(default_block) = default {
                    let mut default_env = self.new_block_env();
                    let _ = default_block.recurse(env, default_env.object());
                    envs.push(default_env);
                }
                self.join(&mut envs, env);
                Ok(())
            }
            Stmt_::Match(box nast::StmtMatch { expr, arms }) => {
                expr.recurse(env, self.object())?;
                let mut envs = arms
                    .iter_mut()
                    .map(|nast::StmtMatchArm { pat, body }| {
                        let _ = pat.recurse(env, self.object());
                        let mut new_env = self.new_block_env();
                        let _ = body.recurse(env, new_env.object());
                        new_env
                    })
                    .collect::<Vec<_>>();
                self.join(&mut envs, env);
                Ok(())
            }
            Stmt_::Foreach(box (expr, as_expr, block)) => {
                expr.recurse(env, self.object())?;
                let mut hints = vec![];
                match as_expr {
                    AsExpr::AsV(e) | AsExpr::AwaitAsV(_, e) => {
                        e.recurse(env, self.object())?;
                        self.get_vars_to_enforce_lhs(e, false, &mut hints)
                    }
                    AsExpr::AsKv(e1, e2) | AsExpr::AwaitAsKv(_, e1, e2) => {
                        e1.recurse(env, self.object())?;
                        e2.recurse(env, self.object())?;
                        self.get_vars_to_enforce_lhs(e1, false, &mut hints);
                        self.get_vars_to_enforce_lhs(e2, false, &mut hints);
                    }
                }
                block.recurse(env, self.object())?;
                if self.should_elab {
                    let stmts = exprs_to_stmts(None, hints);
                    add_to_block(stmts, block);
                }
                Ok(())
            }
            Stmt_::Try(box (try_block, catches, finally_block)) => {
                try_block.recurse(env, self.object())?;
                let mut envs = catches
                    .iter_mut()
                    .map(|nast::Catch(_cn, Lid(pos, name), ref mut block)| {
                        if !self.locals.contains_key(&name.1) {
                            self.add_local(name.1.to_string(), None, pos);
                        }
                        let mut new_env = self.new_block_env();
                        let _ = block.recurse(env, new_env.object());
                        new_env
                    })
                    .collect::<Vec<_>>();
                self.join(&mut envs, env);
                finally_block.recurse(env, self.object())?;
                Ok(())
            }
            // Just need to visit these, no additional logic is required
            Stmt_::Fallthrough
            | Stmt_::Awaitall(_)
            | Stmt_::Concurrent(_)
            | Stmt_::Break
            | Stmt_::Continue
            | Stmt_::Throw(_)
            | Stmt_::Return(_)
            | Stmt_::YieldBreak
            | Stmt_::Do(_)
            | Stmt_::While(_)
            | Stmt_::Noop
            | Stmt_::Block(_)
            | Stmt_::Markup(_)
            | Stmt_::AssertEnv(_) => elem.recurse(env, self.object()),
        }
    }

    fn visit_fun_(&mut self, env: &mut Env, elem: &mut nast::Fun_) -> Result<(), ()> {
        let old = self.clone();
        self.visit_fun_helper(env, elem)?;
        // ensure that checking a lammbda expression doesn't change self, so
        // that we know that recursing on an expr will have no effect
        *self = old;
        Ok(())
    }

    fn visit_efun(&mut self, env: &mut Env, elem: &mut nast::Efun) -> Result<(), ()> {
        let old = self.clone();
        let mut fun_env = self.restrict_env(&elem.use_);
        fun_env.visit_fun_helper(env, &mut elem.fun)?;
        *self = old;
        Ok(())
    }

    fn visit_fun_def(&mut self, env: &mut Env, elem: &mut nast::FunDef) -> Result<(), ()> {
        self.clear();
        let mut generics = self.erased_generics.clone();
        for tp in &elem.tparams {
            if tp.reified != ReifyKind::Reified {
                generics.insert(tp.name.1.clone());
            }
        }
        std::mem::swap(&mut self.erased_generics, &mut generics);
        self.visit_fun_helper(env, &mut elem.fun)?;
        std::mem::swap(&mut self.erased_generics, &mut generics);
        Ok(())
    }

    fn visit_method_(&mut self, env: &mut Env, elem: &mut nast::Method_) -> Result<(), ()> {
        self.clear();
        for param in elem.params.iter() {
            self.add_local(param.name.clone(), None, &param.pos);
            self.add_assigned_id(param.name.clone());
        }
        let mut generics = self.erased_generics.clone();
        for tp in &elem.tparams {
            if tp.reified != ReifyKind::Reified {
                generics.insert(tp.name.1.clone());
            }
        }
        std::mem::swap(&mut self.erased_generics, &mut generics);
        elem.body.fb_ast.recurse(env, self.object())?;
        std::mem::swap(&mut self.erased_generics, &mut generics);
        Ok(())
    }

    fn visit_class_(&mut self, env: &mut Env, elem: &mut nast::Class_) -> Result<(), ()> {
        let mut generics = HashSet::<String>::default();
        for tp in &elem.tparams {
            if tp.reified != ReifyKind::Reified {
                generics.insert(tp.name.1.clone());
            }
        }
        let _ = std::mem::replace(&mut self.erased_generics, generics);
        elem.methods.recurse(env, self.object())?;
        Ok(())
    }
}

pub fn elaborate_program(env: &mut Env, program: &mut nast::Program, should_elab: bool) {
    let mut tl = TypedLocal {
        should_elab,
        ..Default::default()
    };
    tl.visit_program(env, program).unwrap();
}

pub fn elaborate_fun_def(env: &mut Env, f: &mut nast::FunDef, should_elab: bool) {
    let mut tl = TypedLocal {
        should_elab,
        ..Default::default()
    };
    tl.visit_fun_def(env, f).unwrap();
}

pub fn elaborate_class_(env: &mut Env, c: &mut nast::Class_, should_elab: bool) {
    let mut tl = TypedLocal {
        should_elab,
        ..Default::default()
    };
    tl.visit_class_(env, c).unwrap();
}

#[cfg(test)]
mod tests {

    use nast::Block;
    use nast::Def;
    use nast::Program;

    use super::*;

    fn build_program(stmts: Vec<Stmt>) -> Program {
        nast::Program(vec![Def::Stmt(Box::new(Stmt(
            Pos::NONE,
            Stmt_::Block(Box::new((None, Block(stmts)))),
        )))])
    }

    #[test]
    fn test_init() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x: int = 1;"));
        let res = build_program(hack_stmts!("$x = 1 as int;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_init2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x: int;"));
        let res = build_program(hack_stmts!(""));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x: vec<int> = 1; $x = vec[];"));
        let res = build_program(hack_stmts!("$x = 1 as vec<int>; $x = vec[] as vec<int>;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_seq2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x: vec<int>; $x = vec[];"));
        let noop = Stmt(Pos::NONE, Stmt_::Noop);
        let res = build_program(hack_stmts!("#noop; $x = vec[] as vec<int>;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_if1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "let $x: vec<string> = 1; if ($b) {$x = vec[];} else {$x = 1;}; $x = 4;"
        ));
        let res = build_program(hack_stmts!(
            "$x = 1 as vec<string>; if ($b) {$x = vec[] as vec<string>;} else {$x = 1 as vec<string>;}; $x = 4 as vec<string>;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_if2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "if ($b) {let $x: t = vec[]; $x = 1;} else {let $y: t2 = vec[]; $y = 1;}; $x = 4; $y = 44;"
        ));
        let res = build_program(hack_stmts!(
            "if ($b) {$x = vec[] as t; $x = 1 as t;} else {$y = vec[] as t2; $y = 1 as t2;}; $x = 4 as t; $y = 44 as t2;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_for1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "let $x: t = 1; let $y: t2 = 2; for ($x = 1, $y = 2; ; $x = 1, $y = 2) { $x = 1; }; $y = 3;"
        ));
        let res = build_program(hack_stmts!(
            "$x = 1 as t; $y = 2 as t2; for ($x = 1 as t, $y = 2 as t2; ; $x= 1 as t, $y = 2 as t2) { $x = 1 as t; }; $y = 3 as t2;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_for2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "for (; ; $x = 1) { let $x: t = 1; $x = 22;} $x = 3;"
        ));
        let res = build_program(hack_stmts!(
            "for (; ; $x = 1 as t) { $x = 1 as t; $x = 22 as t;} $x = 3 as t;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_for3() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "let $x:t = 1; for (list($x, $y) = vec[1,2]; ; $x[0] = 1, $y = 0) { }"
        ));
        let res = build_program(hack_stmts!(
            "$x = 1 as t; for (list($x, $y) = vec[1,2], $x as t; ; $x[0] = 1, $x as t, $y = 0) { }"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_simplify_hint1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "let $x:\\HH\\vec<\\HH\\int> = vec[]; $x = vec[];"
        ));
        let res = build_program(hack_stmts!(
            "$x = vec[] as \\HH\\vec<_>; $x = vec[] as \\HH\\vec<_>;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_simplify_hint2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "let $x:C<\\HH\\vec<\\HH\\int>> = vec[]; $x = vec[];"
        ));
        let res = build_program(hack_stmts!(
            "$x = vec[] as C<\\HH\\vec<_>>; $x = vec[] as C<\\HH\\vec<_>>;"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_simplify_hint3() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x:C<T> = vec[]; $x = vec[];"));
        let res = build_program(hack_stmts!("$x = vec[] as C<T>; $x = vec[] as C<T>;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_simplify_hint4() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x:C<T> = vec[]; $x = vec[];"));
        let res = build_program(hack_stmts!("$x = vec[] as C<_>; $x = vec[] as C<_>;"));
        let mut tl = TypedLocal {
            should_elab: true,
            ..Default::default()
        };
        tl.erased_generics.insert("T".to_string());
        tl.visit_program(&mut env, &mut orig).unwrap();
        assert_eq!(orig, res);
    }

    #[test]
    fn test_list1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x:int = 1; list($x, $y) = vec[1, 2];"));
        let res = build_program(hack_stmts!(
            "$x = 1 as int; {list($x, $y) = vec[1, 2]; $x as int;}"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_list2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "let $x:int = 1; let $y: int = 1; list($x, list ($z, $y)) = vec[1, vec[2, 3]];"
        ));
        let res = build_program(hack_stmts!(
            "$x = 1 as int; $y = 1as int; {list($x, list ($z, $y)) = vec[1, vec[2, 3]]; $x as int; $y as int;}"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_array1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x:\\HH\\vec<int> = vec[1]; $x[0] = 1;"));
        let res = build_program(hack_stmts!("$x = vec[1] as \\HH\\vec<_>; $x[0] = 1;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_array2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x:t = vec[1]; $x[0] = 1;"));
        let res = build_program(hack_stmts!("$x = vec[1] as t; {$x[0] = 1; $x as t;}"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_array3() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x:t = vec[1]; $x[] = 1;"));
        let res = build_program(hack_stmts!("$x = vec[1] as t; $x[] = 1;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_array4() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x:t = vec[1]; $x[0][] = 1;"));
        let res = build_program(hack_stmts!("$x = vec[1] as t; $x[0][] = 1;"));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_foreach1() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!("let $x:int = 1; foreach (e as $x) {  }"));
        let res = build_program(hack_stmts!(
            "$x = 1 as int; foreach (e as $x) { $x as int; }"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_foreach2() {
        let mut env = Env::default();
        let mut orig = build_program(hack_stmts!(
            "let $x:t = 1; let $y:t = 1; foreach (e as $x[0] => $y[0]) { 1; }"
        ));
        let res = build_program(hack_stmts!(
            "$x = 1 as t; $y = 1 as t; foreach (e as $x[0] => $y[0]) { $x as t; $y as t; 1;}"
        ));
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }

    #[test]
    fn test_eqop() {
        let mut env = Env::default();
        // hack_stmts! macro brolen on += operator
        let mut orig = build_program(vec![
            Stmt(
                Pos::NONE,
                Stmt_::DeclareLocal(Box::new((
                    Lid(Pos::NONE, (0, "$x".to_string())),
                    Hint(
                        Pos::NONE,
                        Box::new(Hint_::Happly(nast::Id(Pos::NONE, "t".to_string()), vec![])),
                    ),
                    Some(Expr((), Pos::NONE, Expr_::Int("1".to_string()))),
                ))),
            ),
            Stmt(
                Pos::NONE,
                Stmt_::Expr(Box::new(Expr(
                    (),
                    Pos::NONE,
                    Expr_::Binop(Box::new(Binop {
                        bop: Bop::Eq(Some(Box::new(Bop::Plus))),
                        lhs: Expr(
                            (),
                            Pos::NONE,
                            Expr_::Lvar(Box::new(Lid(Pos::NONE, (0, "$x".to_string())))),
                        ),
                        rhs: Expr((), Pos::NONE, Expr_::Int("1".to_string())),
                    })),
                ))),
            ),
        ]);
        let res = build_program(vec![
            Stmt(
                Pos::NONE,
                Stmt_::Expr(Box::new(Expr(
                    (),
                    Pos::NONE,
                    Expr_::Binop(Box::new(Binop {
                        bop: Bop::Eq(None),
                        lhs: Expr(
                            (),
                            Pos::NONE,
                            Expr_::Lvar(Box::new(Lid(Pos::NONE, (0, "$x".to_string())))),
                        ),
                        rhs: Expr(
                            (),
                            Pos::NONE,
                            Expr_::As(Box::new((
                                Expr((), Pos::NONE, Expr_::Int("1".to_string())),
                                Hint(
                                    Pos::NONE,
                                    Box::new(Hint_::Happly(
                                        nast::Id(Pos::NONE, "t".to_string()),
                                        vec![],
                                    )),
                                ),
                                false,
                            ))),
                        ),
                    })),
                ))),
            ),
            Stmt(
                Pos::NONE,
                Stmt_::Block(Box::new((
                    None,
                    Block(vec![
                        Stmt(
                            Pos::NONE,
                            Stmt_::Expr(Box::new(Expr(
                                (),
                                Pos::NONE,
                                Expr_::Binop(Box::new(Binop {
                                    bop: Bop::Eq(Some(Box::new(Bop::Plus))),
                                    lhs: Expr(
                                        (),
                                        Pos::NONE,
                                        Expr_::Lvar(Box::new(Lid(
                                            Pos::NONE,
                                            (0, "$x".to_string()),
                                        ))),
                                    ),
                                    rhs: Expr((), Pos::NONE, Expr_::Int("1".to_string())),
                                })),
                            ))),
                        ),
                        Stmt(
                            Pos::NONE,
                            Stmt_::Expr(Box::new(Expr(
                                (),
                                Pos::NONE,
                                Expr_::As(Box::new((
                                    Expr(
                                        (),
                                        Pos::NONE,
                                        Expr_::Lvar(Box::new(Lid(
                                            Pos::NONE,
                                            (0, "$x".to_string()),
                                        ))),
                                    ),
                                    Hint(
                                        Pos::NONE,
                                        Box::new(Hint_::Happly(
                                            nast::Id(Pos::NONE, "t".to_string()),
                                            vec![],
                                        )),
                                    ),
                                    false,
                                ))),
                            ))),
                        ),
                    ]),
                ))),
            ),
        ]);
        self::elaborate_program(&mut env, &mut orig, true);
        assert_eq!(orig, res);
    }
}
