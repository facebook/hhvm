// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use aast::Expr_ as E_;
use hh_autoimport_rust::is_hh_autoimport_fun;
use naming_special_names_rust::special_idents;
use oxidized::{
    aast,
    aast_visitor::{visit_mut, AstParams, NodeMut, VisitorMut},
    ast::*,
    ast_defs, local_id,
    pos::Pos,
};
use parser_core_types::{
    syntax_error,
    syntax_error::{Error as ErrorMsg, SyntaxError},
};
use std::collections::{HashMap, HashSet};

// Local environment which keeps track of how many readonly values it has
#[derive(PartialEq, Clone)]
struct Lenv {
    pub lenv: HashMap<String, Rty>,
    pub num_readonly: u32,
}

impl Lenv {
    pub fn new() -> Lenv {
        Lenv {
            lenv: HashMap::new(),
            num_readonly: 0,
        }
    }
    pub fn insert(&mut self, var_name: String, rty: Rty) {
        let result = self.lenv.insert(var_name, rty);
        match (rty, result) {
            (Rty::Readonly, Some(Rty::Mutable)) => {
                self.num_readonly += 1;
            }
            (Rty::Readonly, None) => {
                self.num_readonly += 1;
            }
            (Rty::Mutable, Some(Rty::Readonly)) => {
                self.num_readonly -= 1;
            }
            _ => {} // otherwise number of readonly values does not change
        }
    }
    pub fn get<S: Sized>(&self, var_name: &S) -> Option<&Rty>
    where
        String: std::borrow::Borrow<S>,
        S: std::hash::Hash + Eq,
    {
        self.lenv.get(&var_name)
    }
}

#[derive(PartialEq, Copy, Clone)]
pub enum Rty {
    Readonly,
    Mutable,
}

struct Context {
    locals: Lenv,
    readonly_return: Rty,
    this_ty: Rty,
    inout_params: HashSet<String>,
}

impl Context {
    fn new(readonly_ret: Rty, this_ty: Rty) -> Self {
        Self {
            locals: Lenv::new(),
            readonly_return: readonly_ret,
            this_ty,
            inout_params: HashSet::new(),
        }
    }

    pub fn add_local(&mut self, var_name: &String, rty: Rty) {
        self.locals.insert(var_name.clone(), rty);
    }

    pub fn add_param(&mut self, var_name: &String, rty: Rty, inout_param: bool) {
        self.locals.insert(var_name.clone(), rty);
        if inout_param {
            self.inout_params.insert(var_name.clone());
        }
    }

    pub fn get_rty(&self, var_name: &String) -> Rty {
        match self.locals.get(var_name) {
            Some(&x) => x,
            None => Rty::Mutable,
        }
    }
}

fn ro_expr_list(context: &mut Context, exprs: &Vec<Expr>) -> Rty {
    if exprs.iter().any(|e| rty_expr(context, &e) == Rty::Readonly) {
        Rty::Readonly
    } else {
        Rty::Mutable
    }
}

fn ro_expr_list2<T>(context: &mut Context, exprs: &Vec<(T, Expr)>) -> Rty {
    if exprs
        .iter()
        .any(|e| rty_expr(context, &e.1) == Rty::Readonly)
    {
        Rty::Readonly
    } else {
        Rty::Mutable
    }
}

fn ro_kind_to_rty(ro: Option<oxidized::ast_defs::ReadonlyKind>) -> Rty {
    match ro {
        Some(oxidized::ast_defs::ReadonlyKind::Readonly) => Rty::Readonly,
        _ => Rty::Mutable,
    }
}

fn rty_expr(context: &mut Context, expr: &Expr) -> Rty {
    let aast::Expr(_, _, exp) = &*expr;
    use aast::Expr_::*;
    match exp {
        ReadonlyExpr(_) => Rty::Readonly,
        ObjGet(og) => {
            let (obj, _member_name, _null_flavor, _reffiness) = &**og;
            rty_expr(context, &obj)
        }
        Lvar(id_orig) => {
            let var_name = local_id::get_name(&id_orig.1);
            let is_this = var_name == special_idents::THIS;
            if is_this {
                context.this_ty
            } else {
                context.get_rty(var_name)
            }
        }
        Darray(d) => {
            let (_, exprs) = &**d;
            ro_expr_list2(context, exprs)
        }
        Varray(v) => {
            let (_, exprs) = &**v;
            ro_expr_list(context, exprs)
        }
        Shape(fields) => ro_expr_list2(context, fields),
        ValCollection(v) => {
            let (_, _, exprs) = &**v;
            ro_expr_list(context, exprs)
        }
        KeyValCollection(kv) => {
            let (_, _, fields) = &**kv;
            if fields
                .iter()
                .any(|f| rty_expr(context, &f.1) == Rty::Readonly)
            {
                Rty::Readonly
            } else {
                Rty::Mutable
            }
        }
        Collection(c) => {
            let (_, _, fields) = &**c;
            if fields.iter().any(|f| match f {
                aast::Afield::AFvalue(e) => rty_expr(context, &e) == Rty::Readonly,
                aast::Afield::AFkvalue(_, e) => rty_expr(context, &e) == Rty::Readonly,
            }) {
                Rty::Readonly
            } else {
                Rty::Mutable
            }
        }
        Record(r) => {
            let (_, fields) = &**r;
            ro_expr_list2(context, &fields)
        }
        Xml(_) | Efun(_) | Lfun(_) => Rty::Mutable,
        Tuple(t) => ro_expr_list(context, t),
        // Only list destructuring
        List(_) => Rty::Mutable,
        // Boolean statement always mutable
        Is(_) => Rty::Mutable,
        //
        As(a) => {
            // Readonlyness of inner expression
            let (exp, _, _) = &**a;
            rty_expr(context, &exp)
        }
        Upcast(a) => {
            // Readonlyness of inner expression
            let (exp, _) = &**a;
            rty_expr(context, &exp)
        }
        Eif(e) => {
            // $x ? a : b is readonly if either a or b are readonly
            let (_, exp1_opt, exp2) = &**e;
            if let Some(exp1) = exp1_opt {
                match (rty_expr(context, exp1), rty_expr(context, exp2)) {
                    (_, Rty::Readonly) | (Rty::Readonly, _) => Rty::Readonly,
                    (Rty::Mutable, Rty::Mutable) => Rty::Mutable,
                }
            } else {
                rty_expr(context, &exp2)
            }
        }
        Pair(p) => {
            let (_, exp1, exp2) = &**p;
            match (rty_expr(context, exp1), rty_expr(context, exp2)) {
                (_, Rty::Readonly) | (Rty::Readonly, _) => Rty::Readonly,
                (Rty::Mutable, Rty::Mutable) => Rty::Mutable,
            }
        }
        Hole(h) => {
            let (expr, _, _, _) = &**h;
            rty_expr(context, &expr)
        }
        Cast(_) => Rty::Mutable, // Casts are only valid on primitive types, so its always mutable
        New(_) => Rty::Mutable,
        // FWIW, this does not appear on the aast at this stage(it only appears after naming in typechecker),
        // but we can handle it for future in case that changes
        This => context.this_ty,
        ArrayGet(ag) => {
            let (expr, _) = &**ag;
            rty_expr(context, &expr)
        }
        Await(expr) => {
            let expr = &**expr;
            rty_expr(context, &expr)
        }
        // Primitive types are mutable
        Null | True | False | Omitted => Rty::Mutable,
        Int(_) | Float(_) | String(_) | String2(_) | PrefixedString(_) => Rty::Mutable,
        Id(_) => Rty::Mutable,
        Dollardollar(lid) => {
            let (id, dollardollar) = &lid.1;
            let var_name = format!("{}{}", dollardollar.clone(), id);
            context.get_rty(&var_name)
        }
        Clone(_) => Rty::Mutable,
        Call(c) => {
            if let (aast::Expr(_, _, Id(i)), _, args, _) = &**c {
                if is_special_builtin(&i.1) && args.len() >= 1 {
                    // Take first argument
                    let (_, expr) = &args[0];
                    rty_expr(context, &expr)
                } else {
                    Rty::Mutable
                }
            } else {
                Rty::Mutable
            }
        }
        // Mutable unless wrapped in a readonly expression
        ClassGet(_) | ClassConst(_) => Rty::Mutable,
        FunctionPointer(_) => Rty::Mutable,
        // This is really just a statement, does not have a value
        Yield(_) => Rty::Mutable,
        // Operators are all primitive in result
        Unop(_) | Binop(_) => Rty::Mutable,
        // TODO: track the left side of pipe expressions' readonlyness for $$
        Pipe(p) => {
            let (lid, left, _) = &**p;
            // The only time the id number matters is for Dollardollar
            let (_, dollardollar) = &lid.1;
            let left_rty = rty_expr(context, left);
            context.add_local(&dollardollar, left_rty);
            rty_expr(context, &p.2)
        }
        ExpressionTree(_) | EnumClassLabel(_) | ETSplice(_) => Rty::Mutable,
        Import(_) | Lplaceholder(_) => Rty::Mutable,
        // More function values which are always mutable
        MethodId(_) | MethodCaller(_) | SmethodId(_) | FunId(_) => Rty::Mutable,
    }
}

fn strip_ns(name: &str) -> &str {
    match name.chars().next() {
        Some('\\') => &name[1..],
        _ => name,
    }
}

// Special builtins that can take readonly values and return the same readonlyness back
// These are represented as bytecodes, so do not go through regular call enforcement
fn is_special_builtin(f_name: &str) -> bool {
    let stripped = strip_ns(f_name);
    let namespaced_f_name = if is_hh_autoimport_fun(stripped) {
        format!("HH\\{}", f_name)
    } else {
        stripped.to_string()
    };
    match &namespaced_f_name[..] {
        "HH\\dict" | "HH\\varray" | "HH\\darray" | "HH\\vec" | "hphp_array_idx" => true,
        /* all other special builtins listed in emit_expresion.rs return mutable:
        specifically, these:
        "array_key_exists" => Some((2, IMisc(AKExists))),
        "intval" => Some((1, IOp(CastInt))),
        "boolval" => Some((1, IOp(CastBool))),
        "strval" => Some((1, IOp(CastString))),
        "floatval" | "doubleval" => Some((1, IOp(CastDouble))),
        "HH\\global_get" => Some((1, IGet(CGetG))),
        "HH\\global_isset" => Some((1, IIsset(IssetG))),
        */
        _ => false,
    }
}

fn explicit_readonly(expr: &mut Expr) {
    match &expr.2 {
        aast::Expr_::ReadonlyExpr(_) => {}
        _ => {
            expr.2 = aast::Expr_::ReadonlyExpr(Box::new(expr.clone()));
        }
    }
}

// For assignments to nonlocals, i.e.
// $x->prop[0] = new Foo();
fn check_assignment_nonlocal(
    context: &mut Context,
    checker: &mut Checker,
    pos: &Pos,
    lhs: &mut Expr,
    rhs: &mut Expr,
) {
    match &mut lhs.2 {
        aast::Expr_::ObjGet(o) => {
            let (obj, _get, _, _) = &**o;
            // If obj is readonly, throw error
            match rty_expr(context, &obj) {
                Rty::Readonly => {
                    checker.add_error(&pos, syntax_error::assignment_to_readonly);
                }
                Rty::Mutable => {
                    match rty_expr(context, &rhs) {
                        Rty::Readonly => {
                            // make the readonly expression explicit, since if it is readonly we need to make sure the property is a readonly prop
                            explicit_readonly(rhs);
                        }
                        // Mutable case does not require special checks
                        Rty::Mutable => {}
                    }
                }
            }
        }
        // On an array get <expr>[0] = <rhs>, recurse and check compatibility of inner <expr> with <rhs>
        aast::Expr_::ArrayGet(ag) => {
            let (array, _) = &mut **ag;
            check_assignment_nonlocal(context, checker, pos, array, rhs);
        }
        _ => {
            // Base case: here we just check whether the lhs expression is readonly compared to the rhs
            match (rty_expr(context, &lhs), rty_expr(context, &rhs)) {
                (Rty::Mutable, Rty::Readonly) => {
                    // error, can't assign a readonly value to a mutable collection
                    checker.add_error(
                        &rhs.1, // Position of readonly expression
                        syntax_error::assign_readonly_to_mutable_collection,
                    )
                }
                (Rty::Readonly, Rty::Readonly) => {
                    // make rhs explicit (to make sure we are not writing a readonly value to a mutable one)
                    explicit_readonly(rhs);
                    // make lhs readonly explicitly, to check that it's a readonly copy on write array
                    // here the lefthandside is either a local variable or a class_get.
                    explicit_readonly(lhs);
                }
                (Rty::Readonly, Rty::Mutable) => {
                    explicit_readonly(lhs);
                }
                // Assigning to a mutable value always succeeds, so no explicit checks are needed
                (Rty::Mutable, Rty::Mutable) => {}
            }
        }
    }
}

// Merges two local environments together. Note that
// because of conservativeness, the resulting Lenv's num_readonly is at
// least the max of lenv1.num_readonly, lenv2.num_readonly
fn merge_lenvs(lenv1: &Lenv, lenv2: &Lenv) -> Lenv {
    let mut new_lenv = lenv1.clone();
    for (key, value) in lenv2.lenv.iter() {
        match (value, new_lenv.get(key)) {
            (_, Some(Rty::Readonly)) => {}
            (rty, _) => {
                new_lenv.insert(key.to_string(), *rty);
            }
        }
    }
    new_lenv
}

// Toplevel assignment check
fn check_assignment_validity(
    context: &mut Context,
    checker: &mut Checker,
    pos: &Pos,
    lhs: &mut Expr,
    rhs: &mut Expr,
) {
    match &mut lhs.2 {
        aast::Expr_::Lvar(id_orig) => {
            let var_name = local_id::get_name(&id_orig.1).to_string();
            let rhs_rty = rty_expr(context, &rhs);
            if context.inout_params.contains(&var_name) && rhs_rty == Rty::Readonly {
                checker.add_error(pos, syntax_error::inout_readonly_assignment);
            }
            context.add_local(&var_name, rhs_rty);
        }
        // list assignment
        aast::Expr_::List(l) => {
            let exprs = &mut **l;
            for e in exprs.iter_mut() {
                check_assignment_validity(context, checker, &e.1.clone(), e, rhs);
            }
        }
        // directly assigning to a class static is always valid (locally) as long as the rhs is explicit on its readonlyness
        aast::Expr_::ClassGet(_) => {
            let rhs_rty = rty_expr(context, &rhs);
            match rhs_rty {
                Rty::Readonly => explicit_readonly(rhs),
                _ => {}
            }
        }
        _ => {
            check_assignment_nonlocal(context, checker, pos, lhs, rhs);
        }
    }
}

struct Checker {
    errors: Vec<SyntaxError>,
}

impl Checker {
    fn new() -> Self {
        Self { errors: vec![] }
    }

    fn add_error(&mut self, pos: &Pos, msg: ErrorMsg) {
        let (start_offset, end_offset) = pos.info_raw();
        self.errors
            .push(SyntaxError::make(start_offset, end_offset, msg));
    }

    fn subtype(&mut self, pos: &Pos, r_sub: &Rty, r_sup: &Rty, reason: &str) {
        use Rty::*;
        match (r_sub, r_sup) {
            (Readonly, Mutable) => self.add_error(
                &pos,
                syntax_error::invalid_readonly("readonly", "mutable", &reason),
            ),
            _ => {}
        }
    }

    fn handle_single_block(
        &mut self,
        context: &mut Context,
        lenv: Lenv,
        b: &mut aast::Block<(), ()>,
    ) -> Lenv {
        context.locals = lenv;
        let _ = b.recurse(context, self.object());
        context.locals.clone()
    }


    // Handles analysis for a given loop
    // Will run b.recurse() up to X times, where X is the number
    // of readonly values in the local environment
    // on each loop, we check if the number of readonly values has changed
    // since each loop iteration will monotonically increase the number
    // of readonly values, the loop must end within that fixed number of iterations.
    fn handle_loop(
        &mut self,
        context: &mut Context,
        lenv: Lenv,
        b: &mut aast::Block<(), ()>,
    ) -> Lenv {
        // run the block once and merge the environment
        let new_lenv = merge_lenvs(&lenv, &self.handle_single_block(context, lenv.clone(), b));
        if new_lenv.num_readonly > lenv.num_readonly {
            self.handle_loop(context, new_lenv, b)
        } else {
            new_lenv
        }
    }
}

impl<'ast> VisitorMut<'ast> for Checker {
    type P = AstParams<Context, ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_method_(
        &mut self,
        _context: &mut Context,
        m: &mut aast::Method_<(), ()>,
    ) -> Result<(), ()> {
        let readonly_return = ro_kind_to_rty(m.readonly_ret);
        let readonly_this = if m.readonly_this {
            Rty::Readonly
        } else {
            Rty::Mutable
        };
        let mut context = Context::new(readonly_return, readonly_this);

        for p in m.params.iter() {
            let is_inout = match p.callconv {
                ast_defs::ParamKind::Pinout(_) => true,
                _ => false,
            };
            if let Some(_) = p.readonly {
                if is_inout {
                    self.add_error(&p.pos, syntax_error::inout_readonly_parameter);
                }
                context.add_param(&p.name, Rty::Readonly, is_inout)
            } else {
                context.add_param(&p.name, Rty::Mutable, is_inout)
            }
        }
        m.recurse(&mut context, self.object())
    }

    fn visit_fun_(&mut self, context: &mut Context, f: &mut aast::Fun_<(), ()>) -> Result<(), ()> {
        // Is run on every function definition and closure definition

        let readonly_return = ro_kind_to_rty(f.readonly_ret);
        let readonly_this = ro_kind_to_rty(f.readonly_this);
        let mut new_context = Context::new(readonly_return, readonly_this);
        // Add the old context's stuff into the new context, as readonly if needed
        for (local, rty) in context.locals.lenv.iter() {
            if readonly_this == Rty::Readonly {
                new_context.add_local(&local, Rty::Readonly);
            } else {
                new_context.add_local(&local, *rty);
            }
        }
        for p in f.params.iter() {
            let is_inout = match p.callconv {
                ast_defs::ParamKind::Pinout(_) => true,
                _ => false,
            };
            if let Some(_) = p.readonly {
                if is_inout {
                    self.add_error(&p.pos, syntax_error::inout_readonly_parameter)
                }
                new_context.add_param(&p.name, Rty::Readonly, is_inout)
            } else {
                new_context.add_param(&p.name, Rty::Mutable, is_inout)
            }
        }
        f.recurse(&mut new_context, self.object())
    }

    fn visit_fun_def(
        &mut self,
        _context: &mut Context,
        f: &mut aast::FunDef<(), ()>,
    ) -> Result<(), ()> {
        // Clear the context completely on a fun_def
        let mut context = Context::new(Rty::Mutable, Rty::Mutable);
        f.recurse(&mut context, self.object())
    }

    fn visit_expr(&mut self, context: &mut Context, p: &mut aast::Expr<(), ()>) -> Result<(), ()> {
        match &mut p.2 {
            aast::Expr_::Binop(x) => {
                let (bop, e_lhs, e_rhs) = x.as_mut();
                if let Bop::Eq(_) = bop {
                    check_assignment_validity(context, self, &p.1, e_lhs, e_rhs);
                }
            }
            aast::Expr_::Call(x) => {
                let (caller, _targs, params, _variadic) = &mut **x;
                match rty_expr(context, caller) {
                    Rty::Readonly => explicit_readonly(caller),
                    Rty::Mutable => {}
                };
                for (_, param) in params.iter_mut() {
                    match rty_expr(context, param) {
                        Rty::Readonly => explicit_readonly(param),
                        Rty::Mutable => {}
                    }
                }
            }
            aast::Expr_::Pipe(p) => {
                let (lid, left, _) = &**p;
                // The only time the id number matters is for Dollardollar
                let (_, dollardollar) = &lid.1;
                let left_rty = rty_expr(context, left);
                context.add_local(&dollardollar, left_rty);
            }
            _ => {}
        }


        p.recurse(context, self.object())
    }

    fn visit_stmt(
        &mut self,
        context: &mut Context,
        s: &mut aast::Stmt<(), ()>,
    ) -> std::result::Result<(), ()> {
        match &mut s.1 {
            aast::Stmt_::Return(r) => {
                if let Some(expr) = r.as_mut() {
                    self.subtype(&expr.1, &rty_expr(context, &expr), &context.readonly_return, "this function does not return readonly. Please mark it to return readonly if needed.")
                }
                s.recurse(context, self.object())
            }
            aast::Stmt_::If(i) => {
                let (condition, if_, else_) = &mut **i;
                let old_lenv = context.locals.clone();
                condition.recurse(context, self.object())?;
                let if_lenv = self.handle_single_block(context, old_lenv.clone(), if_);
                let else_lenv = self.handle_single_block(context, old_lenv.clone(), else_);
                let new_lenv = merge_lenvs(&if_lenv, &else_lenv);
                context.locals = new_lenv;
                Ok(())
            }
            aast::Stmt_::Try(t) => {
                let (try_, catches, finally_) = &mut **t;
                let old_lenv = context.locals.clone();
                try_.recurse(context, self.object())?;
                // Each catch should run with no assumptions about how much of the try ran, i.e. with
                // old_lenv. Start with the lenv right after the try block and merge
                let result_lenv =
                    catches
                        .iter_mut()
                        .fold(context.locals.clone(), |result_lenv, catch| {
                            let catch_lenv =
                                self.handle_single_block(context, old_lenv.clone(), &mut catch.2);
                            merge_lenvs(&result_lenv, &catch_lenv)
                        });
                // Update the lenv from the old lenv with the result of
                context.locals = result_lenv.clone();
                finally_.recurse(context, self.object())
            }
            aast::Stmt_::Switch(s) => {
                let (condition, cases) = &mut **s;
                condition.recurse(context, self.object())?;
                let old_lenv = context.locals.clone();
                let result_lenv =
                    cases
                        .iter_mut()
                        .fold(
                            context.locals.clone(),
                            |result_lenv, mut case| match &mut case {
                                aast::Case::Case(exp, b) => {
                                    let _ = exp.recurse(context, self.object());
                                    let case_lenv =
                                        self.handle_single_block(context, old_lenv.clone(), b);
                                    merge_lenvs(&result_lenv, &case_lenv)
                                }
                                aast::Case::Default(_, b) => {
                                    let case_lenv =
                                        self.handle_single_block(context, old_lenv.clone(), b);
                                    merge_lenvs(&result_lenv, &case_lenv)
                                }
                            },
                        );
                context.locals = result_lenv.clone();
                Ok(())
            }
            aast::Stmt_::Throw(t) => {
                let inner = &**t;
                match rty_expr(context, &inner) {
                    Rty::Readonly => {
                        self.add_error(&inner.1, syntax_error::throw_readonly_exception);
                    }
                    Rty::Mutable => {}
                }
                t.recurse(context, self.object())
            }
            aast::Stmt_::Foreach(f) => {
                let (e, as_expr, b) = &mut **f;
                match as_expr {
                    aast::AsExpr::AsV(aast::Expr(_, _, E_::Lvar(id))) => {
                        let var_name = local_id::get_name(&id.1);
                        let rty = rty_expr(context, e);
                        context.add_local(var_name, rty)
                    }
                    aast::AsExpr::AsKv(
                        _, // key is arraykey and does not need to be readonly
                        aast::Expr(_, _, E_::Lvar(value_id)),
                    ) => {
                        let var_name = local_id::get_name(&value_id.1);
                        let rty = rty_expr(context, e);
                        context.add_local(var_name, rty);
                    }
                    aast::AsExpr::AwaitAsV(_, aast::Expr(_, _, E_::Lvar(id))) => {
                        let var_name = local_id::get_name(&id.1);
                        let rty = rty_expr(context, e);
                        context.add_local(var_name, rty)
                    }
                    aast::AsExpr::AwaitAsKv(
                        _,
                        _, // key is arraykey and does not need to be readonly
                        aast::Expr(_, _, E_::Lvar(value_id)),
                    ) => {
                        let var_name = local_id::get_name(&value_id.1);
                        let rty = rty_expr(context, e);
                        context.add_local(var_name, rty);
                    }
                    // Any other Foreach here would mean no Lvar
                    // where an Lvar is needed. In those cases
                    // we will parse error, but the parse tree might still exist
                    // so we just ignore those cases and continue our analysis
                    // knowing that a parse error will show up
                    _ => {}
                };
                e.recurse(context, self.object())?;
                as_expr.recurse(context, self.object())?;
                let old_lenv = context.locals.clone();
                let new_lenv = self.handle_loop(context, old_lenv, b);
                context.locals = new_lenv;
                Ok(())
            }
            aast::Stmt_::Do(d) => {
                let (b, cond) = &mut **d;
                // loop runs at least once
                let new_lenv = self.handle_single_block(context, context.locals.clone(), b);
                let block_lenv = self.handle_loop(context, new_lenv, b);
                context.locals = block_lenv;
                cond.recurse(context, self.object())
            }
            aast::Stmt_::While(w) => {
                let (cond, b) = &mut **w;
                cond.recurse(context, self.object())?;
                let old_lenv = context.locals.clone();
                let new_lenv = self.handle_loop(context, old_lenv, b);
                context.locals = new_lenv;
                Ok(())
            }
            aast::Stmt_::For(f) => {
                let (initializers, term, increment, b) = &mut **f;
                for i in initializers {
                    i.recurse(context, self.object())?;
                }
                match term {
                    Some(t) => {
                        t.recurse(context, self.object())?;
                    }
                    None => {}
                }
                for inc in increment {
                    inc.recurse(context, self.object())?;
                }
                let old_lenv = context.locals.clone();
                let new_lenv = self.handle_loop(context, old_lenv, b);
                context.locals = new_lenv;
                Ok(())
            }
            _ => s.recurse(context, self.object()),
        }
    }
}

pub fn check_program(program: &mut aast::Program<(), ()>) -> Vec<SyntaxError> {
    let mut checker = Checker::new();
    let mut context = Context::new(Rty::Mutable, Rty::Mutable);
    visit_mut(&mut checker, &mut context, program).unwrap();
    checker.errors
}
