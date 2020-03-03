// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{aast_defs::*, ident, typing_defs::*, typing_reason::*};

// Struct off which we call type builder methods
// This gives us the option to keep some state here e.g. for hash consing
pub struct TypeBuilder {}

impl TypeBuilder {
    pub fn new() -> Self {
        TypeBuilder {}
    }
    // All type construction should go through here
    pub fn mk(&self, reason: Reason, ty_: Ty_) -> Ty {
        Ty(reason, Box::new(ty_))
    }
    pub fn prim(&self, reason: Reason, kind: Tprim) -> Ty {
        self.mk(reason, Ty_::Tprim(kind))
    }
    pub fn int(&self, reason: Reason) -> Ty {
        self.prim(reason, Tprim::Tint)
    }
    pub fn bool(&self, reason: Reason) -> Ty {
        self.prim(reason, Tprim::Tbool)
    }
    pub fn string(&self, reason: Reason) -> Ty {
        self.prim(reason, Tprim::Tstring)
    }
    pub fn float(&self, reason: Reason) -> Ty {
        self.prim(reason, Tprim::Tfloat)
    }
    pub fn arraykey(&self, reason: Reason) -> Ty {
        self.prim(reason, Tprim::Tarraykey)
    }
    pub fn void(&self, reason: Reason) -> Ty {
        self.prim(reason, Tprim::Tvoid)
    }
    pub fn null(&self, reason: Reason) -> Ty {
        self.prim(reason, Tprim::Tnull)
    }
    pub fn nonnull(&self, reason: Reason) -> Ty {
        self.mk(reason, Ty_::Tnonnull)
    }
    pub fn dynamic(&self, reason: Reason) -> Ty {
        self.mk(reason, Ty_::Tdynamic)
    }
    /*
    pub fn object(&self, reason: Reason) -> Ty {
        self.mk(reason, Ty_::Tobject)
    }
    */
    pub fn tyvar(&self, reason: Reason, v: ident::Ident) -> Ty {
        self.mk(reason, Ty_::Tvar(v))
    }
    pub fn union(&self, reason: Reason, tys: Vec<Ty>) -> Ty {
        self.mk(reason, Ty_::Tunion(tys))
    }
    pub fn intersection(&self, reason: Reason, tys: Vec<Ty>) -> Ty {
        self.mk(reason, Ty_::Tintersection(tys))
    }
    pub fn nothing(&self, reason: Reason) -> Ty {
        self.union(reason, Vec::new())
    }
    pub fn nullable(&self, reason: Reason, ty: Ty) -> Ty {
        self.mk(reason, Ty_::Toption(ty))
    }
    pub fn mixed(&self, reason: Reason) -> Ty {
        // TODO: cloning is annoying. We should see if we can change rep of reasons
        let reason2 = reason.clone();
        self.nullable(reason, self.nonnull(reason2))
    }
    pub fn generic(&self, reason: Reason, name: String) -> Ty {
        self.mk(reason, Ty_::Tgeneric(name))
    }
}
