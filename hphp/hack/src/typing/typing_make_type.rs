// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::{classes, collections};
use oxidized::{aast_defs::*, ast_defs::*, ident, typing_defs::*, typing_reason::*};

// Struct off which we call type builder methods
// This gives us the option to keep some state here e.g. for hash consing
pub struct TypeBuilder {
    // Well-known identifiers
    // We could initialize the positions to their actual definition site in hhi
    // This makes more sense than inheriting the position from the use
    // But for now, let's just put none as the position
    // TODO: we might consider creating a SymbolBuilder type
    id_traversable: Id,
    id_keyed_traversable: Id,
    id_keyed_container: Id,
    id_awaitable: Id,
    id_generator: Id,
    id_async_generator: Id,
    id_async_iterator: Id,
    id_async_keyed_iterator: Id,
    id_pair: Id,
    id_dict: Id,
    id_keyset: Id,
    id_vec: Id,
    id_container: Id,
    id_throwable: Id,
    id_datetime: Id,
    id_datetime_immutable: Id,
    id_const_vector: Id,
    id_const_collection: Id,
    id_collection: Id,
}

// Copying of string is bad
// Perhaps best is to actually *resolve* well-known classes
// as though they appear in the source, so we get the position
// from the definition e.g. in hhi file.
fn mk_special_id(name: &str) -> Id {
    Id(Pos::make_none(), name.to_string())
}

impl TypeBuilder {
    pub fn new() -> Self {
        TypeBuilder {
            id_traversable: mk_special_id(collections::TRAVERSABLE),
            id_keyed_traversable: mk_special_id(collections::KEYED_TRAVERSABLE),
            id_keyed_container: mk_special_id(collections::KEYED_CONTAINER),
            id_awaitable: mk_special_id(classes::AWAITABLE),
            id_generator: mk_special_id(classes::GENERATOR),
            id_async_generator: mk_special_id(classes::ASYNC_GENERATOR),
            id_async_iterator: mk_special_id(classes::ASYNC_ITERATOR),
            id_async_keyed_iterator: mk_special_id(classes::ASYNC_KEYED_ITERATOR),
            id_pair: mk_special_id(collections::PAIR),
            id_dict: mk_special_id(collections::DICT),
            id_keyset: mk_special_id(collections::KEYSET),
            id_vec: mk_special_id(collections::VEC),
            id_container: mk_special_id(collections::CONTAINER),
            id_throwable: mk_special_id(classes::THROWABLE),
            id_datetime: mk_special_id(classes::DATE_TIME),
            id_datetime_immutable: mk_special_id(classes::DATE_TIME_IMMUTABLE),
            id_const_vector: mk_special_id(collections::CONST_VECTOR),
            id_const_collection: mk_special_id(collections::CONST_COLLECTION),
            id_collection: mk_special_id(collections::COLLECTION),
        }
    }
    // All type construction should go through here
    pub fn mk(&self, reason: Reason, ty_: Ty_) -> Ty {
        Ty(reason, Box::new(ty_))
    }
    pub fn prim(&self, reason: Reason, kind: Tprim) -> Ty {
        self.mk(reason, Ty_::Tprim(kind))
    }
    pub fn class(&self, reason: Reason, name: Sid, tys: Vec<Ty>) -> Ty {
        self.mk(reason, Ty_::Tclass(name, Exact::Nonexact, tys))
    }
    pub fn traversable(&self, reason: Reason, ty: Ty) -> Ty {
        self.class(reason.clone(), self.id_traversable.clone(), vec![ty])
    }
    pub fn keyed_traversable(&self, reason: Reason, kty: Ty, vty: Ty) -> Ty {
        self.class(
            reason.clone(),
            self.id_keyed_traversable.clone(),
            vec![kty, vty],
        )
    }
    pub fn keyed_container(&self, reason: Reason, kty: Ty, vty: Ty) -> Ty {
        self.class(
            reason.clone(),
            self.id_keyed_container.clone(),
            vec![kty, vty],
        )
    }
    pub fn awaitable(&self, reason: Reason, ty: Ty) -> Ty {
        self.class(reason.clone(), self.id_awaitable.clone(), vec![ty])
    }
    pub fn generator(&self, reason: Reason, key: Ty, value: Ty, send: Ty) -> Ty {
        self.class(
            reason.clone(),
            self.id_generator.clone(),
            vec![key, value, send],
        )
    }
    pub fn async_generator(&self, reason: Reason, key: Ty, value: Ty, send: Ty) -> Ty {
        self.class(
            reason.clone(),
            self.id_async_generator.clone(),
            vec![key, value, send],
        )
    }
    pub fn async_iterator(&self, reason: Reason, ty: Ty) -> Ty {
        self.class(reason.clone(), self.id_async_iterator.clone(), vec![ty])
    }
    pub fn async_keyed_iterator(&self, reason: Reason, kty: Ty, vty: Ty) -> Ty {
        self.class(
            reason.clone(),
            self.id_async_keyed_iterator.clone(),
            vec![kty, vty],
        )
    }
    pub fn pair(&self, reason: Reason, ty1: Ty, ty2: Ty) -> Ty {
        self.class(reason.clone(), self.id_pair.clone(), vec![ty1, ty2])
    }
    pub fn dict(&self, reason: Reason, kty: Ty, vty: Ty) -> Ty {
        self.class(reason.clone(), self.id_dict.clone(), vec![kty, vty])
    }
    pub fn keyset(&self, reason: Reason, ty: Ty) -> Ty {
        self.class(reason.clone(), self.id_keyset.clone(), vec![ty])
    }
    pub fn vec(&self, reason: Reason, ty: Ty) -> Ty {
        self.class(reason.clone(), self.id_vec.clone(), vec![ty])
    }
    pub fn container(&self, reason: Reason, ty: Ty) -> Ty {
        self.class(reason.clone(), self.id_container.clone(), vec![ty])
    }
    pub fn const_vector(&self, reason: Reason, ty: Ty) -> Ty {
        self.class(reason.clone(), self.id_const_vector.clone(), vec![ty])
    }
    pub fn const_collection(&self, reason: Reason, ty: Ty) -> Ty {
        self.class(reason.clone(), self.id_const_collection.clone(), vec![ty])
    }
    pub fn collection(&self, reason: Reason, ty: Ty) -> Ty {
        self.class(reason.clone(), self.id_collection.clone(), vec![ty])
    }
    pub fn throwable(&self, reason: Reason) -> Ty {
        self.class(reason.clone(), self.id_throwable.clone(), vec![])
    }
    pub fn datetime(&self, reason: Reason) -> Ty {
        self.class(reason.clone(), self.id_datetime.clone(), vec![])
    }
    pub fn datetime_immutable(&self, reason: Reason) -> Ty {
        self.class(reason.clone(), self.id_datetime_immutable.clone(), vec![])
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
