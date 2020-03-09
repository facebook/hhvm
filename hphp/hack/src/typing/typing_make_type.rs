// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::collections::Vec;
use bumpalo::{vec, Bump};

use naming_special_names_rust::{classes, collections};
use oxidized::pos::Pos;
use oxidized::{aast_defs::*, ast_defs::*, ident};

use crate::typing_defs_core::{Ty, *};
use crate::typing_reason::*;

// Struct off which we call type builder methods
// This gives us the option to keep some state here e.g. for hash consing
pub struct TypeBuilder<'a> {
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

    alloc: &'a Bump,
}

// Copying of string is bad
// Perhaps best is to actually *resolve* well-known classes
// as though they appear in the source, so we get the position
// from the definition e.g. in hhi file.
fn mk_special_id(name: &str) -> Id {
    Id(Pos::make_none(), name.to_string())
}

impl<'a> TypeBuilder<'a> {
    pub fn new(alloc: &'a Bump) -> Self {
        TypeBuilder {
            alloc,

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
}

/// All type builders go here
impl<'a> TypeBuilder<'a> {
    // All type construction should go through here
    fn mk(&'a self, reason: PReason<'a>, ty_: Ty_<'a>) -> Ty<'a> {
        Ty(reason, self.alloc.alloc(ty_))
    }

    pub fn prim(&'a self, reason: PReason<'a>, kind: PrimKind<'a>) -> Ty<'a> {
        self.mk(reason, Ty_::Tprim(kind))
    }
    pub fn class(&'a self, reason: PReason<'a>, name: &'a Sid, tys: Vec<'a, Ty<'a>>) -> Ty<'a> {
        self.mk(reason, Ty_::Tclass(name, Exact::Nonexact, tys))
    }
    pub fn traversable(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_traversable, vec![in &self.alloc; ty])
    }
    pub fn keyed_traversable(&'a self, reason: PReason<'a>, kty: Ty<'a>, vty: Ty<'a>) -> Ty<'a> {
        self.class(
            reason,
            &self.id_keyed_traversable,
            vec![in &self.alloc; kty, vty],
        )
    }
    pub fn keyed_container(&'a self, reason: PReason<'a>, kty: Ty<'a>, vty: Ty<'a>) -> Ty<'a> {
        self.class(
            reason,
            &self.id_keyed_container,
            vec![in &self.alloc; kty, vty],
        )
    }
    pub fn awaitable(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_awaitable, vec![in &self.alloc; ty])
    }
    pub fn generator(
        &'a self,
        reason: PReason<'a>,
        key: Ty<'a>,
        value: Ty<'a>,
        send: Ty<'a>,
    ) -> Ty<'a> {
        self.class(
            reason,
            &self.id_generator,
            vec![in &self.alloc; key, value, send],
        )
    }
    pub fn async_generator(
        &'a self,
        reason: PReason<'a>,
        key: Ty<'a>,
        value: Ty<'a>,
        send: Ty<'a>,
    ) -> Ty<'a> {
        self.class(
            reason,
            &self.id_async_generator,
            vec![in &self.alloc; key, value, send],
        )
    }
    pub fn async_iterator(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_async_iterator, vec![in &self.alloc; ty])
    }
    pub fn async_keyed_iterator(&'a self, reason: PReason<'a>, kty: Ty<'a>, vty: Ty<'a>) -> Ty<'a> {
        self.class(
            reason,
            &self.id_async_keyed_iterator,
            vec![in &self.alloc; kty, vty],
        )
    }
    pub fn pair(&'a self, reason: PReason<'a>, ty1: Ty<'a>, ty2: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_pair, vec![in &self.alloc; ty1, ty2])
    }
    pub fn dict(&'a self, reason: PReason<'a>, kty: Ty<'a>, vty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_dict, vec![in &self.alloc; kty, vty])
    }
    pub fn keyset(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_keyset, vec![in &self.alloc; ty])
    }
    pub fn vec(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_vec, vec![in &self.alloc; ty])
    }
    pub fn container(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_container, vec![in &self.alloc; ty])
    }
    pub fn const_vector(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_const_vector, vec![in &self.alloc; ty])
    }
    pub fn const_collection(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_const_collection, vec![in &self.alloc; ty])
    }
    pub fn collection(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.class(reason, &self.id_collection, vec![in &self.alloc; ty])
    }
    pub fn throwable(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.class(reason, &self.id_throwable, vec![in &self.alloc; ])
    }
    pub fn datetime(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.class(reason, &self.id_datetime, vec![in &self.alloc])
    }
    pub fn datetime_immutable(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.class(reason, &self.id_datetime_immutable, vec![in &self.alloc])
    }
    pub fn int(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.prim(reason, PrimKind::Tint)
    }
    pub fn bool(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.prim(reason, PrimKind::Tbool)
    }
    pub fn string(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.prim(reason, PrimKind::Tstring)
    }
    pub fn float(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.prim(reason, PrimKind::Tfloat)
    }
    pub fn arraykey(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.prim(reason, PrimKind::Tarraykey)
    }
    pub fn void(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.prim(reason, PrimKind::Tvoid)
    }
    pub fn null(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.prim(reason, PrimKind::Tnull)
    }
    pub fn nonnull(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.mk(reason, Ty_::Tnonnull)
    }
    pub fn dynamic(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.mk(reason, Ty_::Tdynamic)
    }
    /*
    pub fn object(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.mk(reason, Ty_::Tobject)
    }
    */
    pub fn tyvar(&'a self, reason: PReason<'a>, v: ident::Ident) -> Ty<'a> {
        self.mk(reason, Ty_::Tvar(v))
    }
    pub fn union(&'a self, reason: PReason<'a>, tys: Vec<'a, Ty<'a>>) -> Ty<'a> {
        self.mk(reason, Ty_::Tunion(tys))
    }
    pub fn intersection(&'a self, reason: PReason<'a>, tys: Vec<'a, Ty<'a>>) -> Ty<'a> {
        self.mk(reason, Ty_::Tintersection(tys))
    }
    pub fn nothing(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.union(reason, Vec::new_in(self.alloc))
    }
    pub fn nullable(&'a self, reason: PReason<'a>, ty: Ty<'a>) -> Ty<'a> {
        self.mk(reason, Ty_::Toption(ty))
    }
    pub fn mixed(&'a self, reason: PReason<'a>) -> Ty<'a> {
        self.nullable(reason, self.nonnull(reason))
    }
    pub fn generic(&'a self, reason: PReason<'a>, name: &'a str) -> Ty<'a> {
        self.mk(reason, Ty_::Tgeneric(name))
    }
}

/// All reason builders go here
impl<'a> TypeBuilder<'a> {
    fn mk_reason(&'a self, pos: Option<&'a Pos>, reason: Reason<'a>) -> PReason<'a> {
        self.alloc.alloc(PReason_ { pos, reason })
    }

    /// Make an Rnone reason. This does not belong here, but:
    pub fn mk_rnone(&'a self) -> PReason<'a> {
        self.mk_reason(None, Reason::Rnone)
    }
}
