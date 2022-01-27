// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(dead_code)]
use std::cell::RefCell;
use std::rc::Rc;

use crate::reason::Reason;
use crate::tast::SavedEnv;
use crate::typing_ctx::TypingCtx;
use crate::typing_defs::{ParamMode, Ty};
use crate::typing_error::TypingError;
use crate::typing_local_types::{Local, LocalMap};
use crate::typing_return::TypingReturnInfo;
use crate::utils::core::{IdentGen, LocalId};

use im::HashMap;

pub struct TEnv<R: Reason> {
    pub ctx: Rc<TypingCtx<R>>,

    genv: Rc<TGEnv<R>>,
    lenv: Rc<TLEnv<R>>,

    idents: IdentGen,
    errors: Rc<RefCell<Vec<TypingError<R>>>>,
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
    fn new(ctx: &TypingCtx<R>) -> Self {
        Self {
            return_: RefCell::new(TypingReturnInfo::placeholder(ctx.alloc)),
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
    pub fn new(ctx: Rc<TypingCtx<R>>) -> Self {
        let genv = Rc::new(TGEnv::new(&ctx));
        Self {
            ctx,

            genv,
            lenv: Rc::new(TLEnv::new()),

            idents: IdentGen::new(),
            errors: Rc::new(RefCell::new(Vec::new())),
        }
    }

    pub fn destruct(self) -> Vec<TypingError<R>> {
        let mut empty = Vec::new();
        let mut full = self.errors.borrow_mut();
        std::mem::swap(&mut empty, &mut full);
        empty
    }

    pub fn add_error(&self, error: TypingError<R>) {
        self.errors.borrow_mut().push(error)
    }

    pub fn fun_env(ctx: Rc<TypingCtx<R>>, _fd: &oxidized::aast::FunDef<(), ()>) -> Self {
        Self::new(ctx)
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
