// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]
use std::cell::RefCell;
use std::rc::Rc;

use crate::dependency_registrar::DeclName;
use crate::reason::Reason;
use crate::tast::SavedEnv;
use crate::typing::{Error, Result};
use crate::typing_ctx::TypingCtx;
use crate::typing_decl_provider::Class;
use crate::typing_defs::{ParamMode, Ty};
use crate::typing_error::TypingError;
use crate::typing_local_types::{Local, LocalMap};
use crate::typing_return::TypingReturnInfo;
use crate::utils::core::{IdentGen, LocalId};
use pos::TypeName;

use pos::FunName;

use im::HashMap;

pub struct TEnv<R: Reason> {
    pub ctx: Rc<TypingCtx<R>>,

    genv: Rc<TGEnv<R>>,
    lenv: Rc<TLEnv<R>>,

    idents: IdentGen,
    errors: Rc<RefCell<Vec<TypingError<R>>>>,

    dependent: DeclName,
    decls: TEnvDecls<R>,
}

pub struct TEnvDecls<R: Reason> {
    ctx: Rc<TypingCtx<R>>,
    dependent: DeclName,
}

struct TGEnv<R: Reason> {
    return_: RefCell<TypingReturnInfo<R>>,
    params: RefCell<HashMap<LocalId, (Ty<R>, R::Pos, ParamMode)>>,
}

struct TLEnv<R: Reason> {
    per_cont_env: PerContEnv<R>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
enum TypingContKey {
    Next,
    Continue,
    Break,
    Catch,
    Do,
    Exit,
    Fallthrough,
    Finally,
}

#[derive(Debug, Clone)]
struct PerContEnv<R: Reason>(RefCell<HashMap<TypingContKey, PerContEntry<R>>>);

#[derive(Debug, Clone)]
struct PerContEntry<R: Reason> {
    local_types: LocalMap<R>,
}

impl<R: Reason> TGEnv<R> {
    fn new() -> Self {
        Self {
            return_: RefCell::new(TypingReturnInfo::placeholder()),
            params: RefCell::new(HashMap::new()),
        }
    }
}

impl<R: Reason> TLEnv<R> {
    fn new() -> Self {
        Self {
            per_cont_env: PerContEnv::new(),
        }
    }
}

impl<R: Reason> PerContEnv<R> {
    fn new() -> Self {
        Self(RefCell::new(HashMap::new()))
    }

    fn add(&self, key: TypingContKey, x: LocalId, ty: Local<R>) {
        if let Some(cont) = self.0.borrow_mut().get_mut(&key) {
            cont.local_types.add(x, ty);
        }
    }

    fn get(&self, key: TypingContKey, x: &LocalId) -> Option<Local<R>> {
        self.0
            .borrow()
            .get(&key)
            .and_then(|env| env.local_types.get(x))
            .cloned()
    }

    fn has_cont(&self, key: TypingContKey) -> bool {
        self.0.borrow().get(&key).is_some()
    }
}

impl<R: Reason> TEnv<R> {
    pub fn new(dependent: DeclName, ctx: Rc<TypingCtx<R>>) -> Self {
        let genv = Rc::new(TGEnv::new());
        Self {
            ctx: Rc::clone(&ctx),

            genv,
            lenv: Rc::new(TLEnv::new()),

            idents: IdentGen::new(),
            errors: Rc::new(RefCell::new(Vec::new())),
            dependent,
            decls: TEnvDecls::new(Rc::clone(&ctx), dependent),
        }
    }

    // The "dependency root" (`droot` in OCaml). That is, the symbol getting
    // typed so that we might record any dependencies of that symbol on other
    // symbols we encounter as we do so.
    pub fn get_dependent(&self) -> DeclName {
        self.dependent
    }

    pub fn destruct(self) -> Vec<TypingError<R>> {
        let mut empty = Vec::new();
        let mut full = self.errors.borrow_mut();
        std::mem::swap(&mut empty, &mut full);
        empty
    }

    pub fn decls(&self) -> &TEnvDecls<R> {
        &self.decls
    }

    pub fn add_error(&self, error: TypingError<R>) {
        self.errors.borrow_mut().push(error)
    }

    pub fn fun_env(ctx: Rc<TypingCtx<R>>, fd: &oxidized::aast::FunDef<(), ()>) -> Self {
        Self::new(FunName::new(&fd.fun.name.1).into(), ctx)
    }

    pub fn class_env(ctx: Rc<TypingCtx<R>>, cd: &oxidized::aast::Class_<(), ()>) -> Self {
        // TODO(hrust): set_self, set_parent
        Self::new(TypeName::new(&cd.name.1).into(), ctx)
    }

    pub fn set_param(&self, id: LocalId, ty: Ty<R>, pos: R::Pos, param_mode: ParamMode) {
        self.genv
            .params
            .borrow_mut()
            .insert(id, (ty, pos, param_mode));
    }

    pub fn get_params(&self) -> HashMap<LocalId, (Ty<R>, R::Pos, ParamMode)> {
        self.genv.params.borrow().clone()
    }

    pub fn set_params(&self, m: HashMap<LocalId, (Ty<R>, R::Pos, ParamMode)>) {
        *self.genv.params.borrow_mut() = m;
    }

    fn set_local_(&self, x: LocalId, ty: Local<R>) {
        self.lenv.per_cont_env.add(TypingContKey::Next, x, ty);
    }

    pub fn set_local(&self, immutable: bool, x: LocalId, ty: Ty<R>, pos: R::Pos) {
        // TODO(hrust): union simplification
        let expr_id = match self.lenv.per_cont_env.get(TypingContKey::Next, &x) {
            None => self.idents.make(),
            Some(l) => l.expr_id,
        };
        if immutable {
            unimplemented!()
        }
        self.set_local_(x, Local { ty, pos, expr_id })
    }

    pub fn get_return(&self) -> TypingReturnInfo<R> {
        self.genv.return_.borrow().clone()
    }

    pub fn set_return(&self, ret: TypingReturnInfo<R>) {
        *self.genv.return_.borrow_mut() = ret;
    }

    pub fn has_next(&self) -> bool {
        self.lenv.per_cont_env.has_cont(TypingContKey::Next)
    }

    pub fn save(&self, _local_tpenv: ()) -> SavedEnv {
        SavedEnv
    }
}

impl<R: Reason> TEnvDecls<R> {
    pub fn new(ctx: Rc<TypingCtx<R>>, dependent: DeclName) -> Self {
        Self { ctx, dependent }
    }

    pub fn get_class(&self, name: TypeName) -> Result<Option<Rc<dyn Class<R>>>> {
        self.ctx
            .typing_decl_provider
            .get_class(self.dependent, name)
            .map_err(|e| e.into())
    }

    pub fn get_class_or_error(&self, name: TypeName) -> Result<Rc<dyn Class<R>>> {
        self.get_class(name)
            .and_then(|cls| cls.ok_or_else(|| Error::DeclNotFound(name.into())))
    }
}
