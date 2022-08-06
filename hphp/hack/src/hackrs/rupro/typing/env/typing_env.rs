// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cell::RefCell;
use std::rc::Rc;

use depgraph_api::DeclName;
use im::HashMap;
use pos::FunName;
use pos::TypeName;
use ty::local::ParamMode;
use ty::local::Ty;
use ty::local::Variance;
use ty::local_error::TypingError;
use ty::reason::Reason;
use utils::core::IdentGen;
use utils::core::LocalId;

use crate::inference_env::InferenceEnv;
use crate::subtyping::Subtyper;
use crate::tast::SavedEnv;
use crate::typaram_env::TyparamEnv;
use crate::typing::env::typing_env_decls::TEnvDecls;
use crate::typing::env::typing_env_decls::TEnvDeclsOracle;
use crate::typing::env::typing_genv::TGEnv;
use crate::typing::env::typing_lenv::TLEnv;
use crate::typing::env::typing_local_types::Local;
use crate::typing::env::typing_per_cont_env::TypingContKey;
use crate::typing::env::typing_return_info::TypingReturnInfo;
use crate::typing::typing_error::Result;
use crate::typing_ctx::TypingCtx;

/// The main typing environment.
#[derive(Debug)]
pub struct TEnv<R: Reason> {
    genv: Rc<TGEnv<R>>,
    lenv: Rc<TLEnv<R>>,
    inf_env: InferenceEnv<R>,
    tp_env: TyparamEnv<R>,

    idents: IdentGen,
    errors: Rc<RefCell<Vec<TypingError<R>>>>,

    dependent: DeclName,
    decls: Rc<TEnvDecls<R>>,
}

/// Functions for constructing environments.
impl<R: Reason> TEnv<R> {
    /// Create a new environment.
    ///
    /// The `dependent` is the current toplevel entity that is being type
    /// checked. It is used to do dependency tracking.
    ///
    /// This function should not directly be called. Rather, you should
    /// use auxiliary methods in `typing_env_from_def`.
    pub fn new(dependent: DeclName, ctx: Rc<TypingCtx<R>>) -> Self {
        let env = Self {
            genv: Rc::new(TGEnv::new()),
            lenv: Rc::new(TLEnv::new()),
            inf_env: InferenceEnv::default(),
            tp_env: TyparamEnv::default(),

            idents: IdentGen::new(),
            errors: Rc::new(RefCell::new(Vec::new())),
            dependent,
            decls: Rc::new(TEnvDecls::new(Rc::clone(&ctx), dependent)),
        };
        env.reinitialize_locals();
        env
    }

    /// Initialize an environment to type check a toplevel function.
    pub fn fun_env(ctx: Rc<TypingCtx<R>>, fd: &oxidized::aast::FunDef<(), ()>) -> Self {
        Self::new(FunName::new(&fd.fun.name.1).into(), ctx)
    }

    /// Initialize an environment to type check a toplevel class.
    pub fn class_env(ctx: Rc<TypingCtx<R>>, cd: &oxidized::aast::Class_<(), ()>) -> Self {
        rupro_todo_mark!(BindThis);
        Self::new(TypeName::new(&cd.name.1).into(), ctx)
    }
}

impl<R: Reason> TEnv<R> {
    // The "dependency root" (`droot` in OCaml). That is, the symbol getting
    // typed so that we might record any dependencies of that symbol on other
    // symbols we encounter as we do so.
    pub fn get_dependent(&self) -> DeclName {
        self.dependent
    }

    /// Destruct the environment and extract the errors and other artifacts.
    pub fn destruct(self) -> Vec<TypingError<R>> {
        let mut empty = Vec::new();
        let mut full = self.errors.borrow_mut();
        std::mem::swap(&mut empty, &mut full);
        empty
    }

    /// Access to the decl provider.
    pub fn decls(&self) -> &TEnvDecls<R> {
        &self.decls
    }

    /// Register a typing error.
    pub fn add_error(&self, error: TypingError<R>) {
        self.errors.borrow_mut().push(error)
    }

    /// Store a function parameter in the global environment.
    ///
    /// Access to parameter configuration is necessary for inout-checks when
    /// typing return statements.
    pub fn set_param(&self, id: LocalId, ty: Ty<R>, pos: R::Pos, param_mode: ParamMode) {
        self.genv
            .params
            .borrow_mut()
            .insert(id, (ty, pos, param_mode));
    }

    /// Retrieve all function parameters.
    pub fn get_params(&self) -> HashMap<LocalId, (Ty<R>, R::Pos, ParamMode)> {
        self.genv.params.borrow().clone()
    }

    /// Set all function parameters in the global environment.
    ///
    /// See `set_param` for more information.
    pub fn set_params(&self, m: HashMap<LocalId, (Ty<R>, R::Pos, ParamMode)>) {
        *self.genv.params.borrow_mut() = m;
    }

    /// Clear all function parameters.
    pub fn clear_params(&self) {
        self.set_params(Default::default());
    }

    /// Reset the locals environments for all continuations.
    pub fn reinitialize_locals(&self) {
        self.lenv.per_cont_env.initial_locals();
    }

    fn set_local_(&self, x: LocalId, ty: Local<R>) {
        self.lenv.per_cont_env.add(TypingContKey::Next, x, ty);
    }

    /// Bind a local in the `Next` typing continuation environment.
    ///
    /// Reuses the expression ID if the local was already bound in the
    /// environment. Generates an expression ID if not.
    pub fn set_local(&self, immutable: bool, x: LocalId, ty: Ty<R>, pos: R::Pos) {
        rupro_todo_mark!(UnionsIntersections);
        rupro_todo_assert!(!immutable, Readonly);
        let expr_id = match self.lenv.per_cont_env.get(TypingContKey::Next, &x) {
            None => self.idents.make(),
            Some(l) => l.expr_id,
        };
        self.set_local_(x, Local { ty, pos, expr_id });
    }

    /// Set the local to the given value type.
    ///
    /// Also assign a new expression ID.
    pub fn set_local_new_value(&self, immutable: bool, x: LocalId, ty: Ty<R>, pos: R::Pos) {
        let expr_id = self.idents.make();
        rupro_todo_assert!(!immutable, Readonly);
        self.set_local_(x, Local { ty, pos, expr_id });
    }

    fn get_local_(&self, error_if_undefined_at_pos: Option<R::Pos>, x: &LocalId) -> (bool, Ty<R>) {
        if !self.lenv.per_cont_env.has_cont(TypingContKey::Next) {
            // If the continuation is absent, we are in dead code so the
            // variable should have type nothing
            rupro_todo!(MissingError, "{:?}", error_if_undefined_at_pos)
        } else {
            let lty = self.lenv.per_cont_env.get(TypingContKey::Next, x);
            rupro_todo_assert!(
                lty.is_some() || error_if_undefined_at_pos.is_none(),
                MissingError
            );
            (true, lty.unwrap().ty)
        }
    }

    /// Get the type of a local in the `Next` continuation.
    ///
    /// If the variable is not defined, add a typing error and return `Tnothing`.
    pub fn get_local_check_defined(&self, p: R::Pos, x: &LocalId) -> Ty<R> {
        rupro_todo_mark!(CheckLocalDefined);
        let (_, lty) = self.get_local_(Some(p), x);
        lty
    }

    /// Get the return type information for the current function.
    pub fn get_return(&self) -> TypingReturnInfo<R> {
        self.genv.return_.borrow().clone()
    }

    /// Store return type information in the global environment.
    pub fn set_return(&self, ret: TypingReturnInfo<R>) {
        *self.genv.return_.borrow_mut() = ret;
    }

    /// Check whether we have a `Next` continuation.
    ///
    /// E.g., a `return` statement will remove the `Next` continuation.
    pub fn has_next(&self) -> bool {
        self.lenv.per_cont_env.has_cont(TypingContKey::Next)
    }

    /// Save the typing environment to be attached to TAST nodes.
    pub fn save(&self, _local_tpenv: ()) -> SavedEnv<R> {
        SavedEnv {
            inf_env: self.inf_env.clone(),
        }
    }

    /// Resolve a subtype query, potentially adding constraints to
    /// the current environment, if no errors are detected.
    ///
    /// Note that any errors are not automatically addded ot the environment.
    pub fn subtyper<'a>(&'a mut self) -> Subtyper<'a, R> {
        Subtyper::new(
            &mut self.inf_env,
            &mut self.tp_env,
            Rc::new(TEnvDeclsOracle::new(self.decls.clone())),
            None,
        )
    }

    /// Allocate a new type variable in the environment.
    pub fn fresh(&mut self, variance: Variance, pos: R::Pos, reason_opt: Option<R>) -> Ty<R> {
        self.inf_env.fresh(variance, pos, reason_opt)
    }

    /// For solve all type variables.
    pub fn solve_all_unsolved_tyvars(&mut self) -> Result<Option<Vec<TypingError<R>>>> {
        for tyvar in self.inf_env.get_all_tyvars() {
            let err_opt = self.subtyper().force_solve(tyvar, &Reason::none())?;
            rupro_todo_assert!(err_opt.is_none(), MissingError);
        }
        Ok(None)
    }

    /// Path shorthen the given type.
    pub fn resolve_ty(&mut self, ty: &Ty<R>) -> Ty<R> {
        self.inf_env.resolve_ty(ty)
    }

    /// Resolve a type and force solve it.
    pub fn resolve_ty_and_solve(&mut self, ty: &Ty<R>) -> Result<Ty<R>> {
        let ty = self.resolve_ty(ty);
        rupro_todo_assert!(self.subtyper().force_solve_ty(&ty)?.is_none(), MissingError);
        Ok(self.resolve_ty(&ty))
    }
}
