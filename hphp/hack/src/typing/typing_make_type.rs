// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::collections::String as BString;
use bumpalo::collections::Vec as BVec;
use bumpalo::{vec, Bump};

use arena_trait::Arena;
use naming_special_names_rust::{classes, collections};
use oxidized_by_ref::pos::Pos;
use oxidized_by_ref::tany_sentinel::TanySentinel;
use oxidized_by_ref::{
    ident,
    typing_defs_flags::{FunParamFlags, FunTypeFlags},
};

use crate::typing_defs::ExpandEnv;
use crate::typing_defs_core::*;
use crate::typing_logic::{SubtypeProp, SubtypePropEnum};
use crate::typing_reason::*;
use typing_collections_rust::{SMap, Vec};

// Struct off which we call type builder methods
// This gives us the option to keep some state here e.g. for hash consing
pub struct TypeBuilder<'a> {
    // Well-known identifiers
    // We could initialize the positions to their actual definition site in hhi
    // This makes more sense than inheriting the position from the use
    // But for now, let's just put none as the position
    // TODO: we might consider creating a SymbolBuilder type
    id_traversable: PosId<'a>,
    id_keyed_traversable: PosId<'a>,
    id_keyed_container: PosId<'a>,
    id_awaitable: PosId<'a>,
    id_generator: PosId<'a>,
    id_async_generator: PosId<'a>,
    id_async_iterator: PosId<'a>,
    id_async_keyed_iterator: PosId<'a>,
    id_pair: PosId<'a>,
    id_dict: PosId<'a>,
    id_keyset: PosId<'a>,
    id_vec: PosId<'a>,
    id_container: PosId<'a>,
    id_throwable: PosId<'a>,
    id_datetime: PosId<'a>,
    id_datetime_immutable: PosId<'a>,
    id_const_vector: PosId<'a>,
    id_const_collection: PosId<'a>,
    id_collection: PosId<'a>,

    /// The allocator.
    ///
    /// We have made this field private to avoid people calling
    /// alloc on the underlying allocator directly. Instead, use
    /// `TypeBuilder::alloc`, which provides safety mechanisms
    /// against memory leaks.
    bumpalo: &'a Bump,
}

impl<'a> Arena for TypeBuilder<'a> {
    #[inline(always)]
    fn alloc<T>(&self, val: T) -> &mut T {
        return self.bumpalo.alloc(val);
    }
}

// Copying of string is bad
// Perhaps best is to actually *resolve* well-known classes
// as though they appear in the source, so we get the position
// from the definition e.g. in hhi file.
fn mk_special_id(name: &'static str) -> PosId<'static> {
    (Pos::none(), name)
}

impl<'a> TypeBuilder<'a> {
    pub fn new(bumpalo: &'a Bump) -> Self {
        TypeBuilder {
            bumpalo,

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

    pub fn bumpalo(&self) -> &'a Bump {
        self.bumpalo
    }

    pub fn vec_from_iter<T, I: IntoIterator<Item = T>>(&self, iter: I) -> BVec<'a, T> {
        BVec::from_iter_in(iter, &self.bumpalo)
    }

    pub fn slice_from_iter<T, I: IntoIterator<Item = T>>(&self, iter: I) -> &'a [T] {
        self.vec_from_iter(iter).into_bump_slice()
    }

    pub fn str_from_str(&self, s: &str) -> &'a str {
        BString::from_str_in(s, &self.bumpalo).into_bump_str()
    }
}

/// All type builders go here
impl<'a> TypeBuilder<'a> {
    // All type construction should go through here
    fn mk(&'a self, reason: &'a Reason<'a>, ty_: Ty_<'a>) -> &'a Ty<'a> {
        self.alloc(Ty::mk(reason, ty_))
    }

    pub fn any(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tany(TanySentinel))
    }
    pub fn prim(&'a self, reason: &'a Reason<'a>, kind: PrimKind) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tprim(self.alloc(kind)))
    }

    pub fn shape(
        &'a self,
        reason: &'a Reason<'a>,
        kind: ShapeKind,
        map: t_shape_map::TShapeMap<'a, &'a ShapeFieldType<'a>>,
    ) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tshape(self.alloc((kind, map))))
    }

    pub fn class(
        &'a self,
        reason: &'a Reason<'a>,
        name: PosId<'a>,
        tys: &'a [&'a Ty<'a>],
    ) -> &'a Ty<'a> {
        self.mk(
            reason,
            Ty_::Tclass(self.alloc((name, Exact::Nonexact, tys))),
        )
    }
    pub fn traversable(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_traversable,
            vec![in &self.bumpalo; ty].into_bump_slice(),
        )
    }
    pub fn keyed_traversable(
        &'a self,
        reason: &'a Reason<'a>,
        kty: &'a Ty<'a>,
        vty: &'a Ty<'a>,
    ) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_keyed_traversable,
            vec![in &self.bumpalo; kty, vty].into_bump_slice(),
        )
    }
    pub fn keyed_container(
        &'a self,
        reason: &'a Reason<'a>,
        kty: &'a Ty<'a>,
        vty: &'a Ty<'a>,
    ) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_keyed_container,
            vec![in &self.bumpalo; kty, vty].into_bump_slice(),
        )
    }
    pub fn awaitable(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_awaitable,
            vec![in &self.bumpalo; ty].into_bump_slice(),
        )
    }
    pub fn generator(
        &'a self,
        reason: &'a Reason<'a>,
        key: &'a Ty<'a>,
        value: &'a Ty<'a>,
        send: &'a Ty<'a>,
    ) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_generator,
            vec![in &self.bumpalo; key, value, send].into_bump_slice(),
        )
    }
    pub fn async_generator(
        &'a self,
        reason: &'a Reason<'a>,
        key: &'a Ty<'a>,
        value: &'a Ty<'a>,
        send: &'a Ty<'a>,
    ) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_async_generator,
            vec![in &self.bumpalo; key, value, send].into_bump_slice(),
        )
    }
    pub fn async_iterator(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_async_iterator,
            vec![in &self.bumpalo; ty].into_bump_slice(),
        )
    }
    pub fn async_keyed_iterator(
        &'a self,
        reason: &'a Reason<'a>,
        kty: &'a Ty<'a>,
        vty: &'a Ty<'a>,
    ) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_async_keyed_iterator,
            vec![in &self.bumpalo; kty, vty].into_bump_slice(),
        )
    }
    pub fn pair(&'a self, reason: &'a Reason<'a>, ty1: &'a Ty<'a>, ty2: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_pair,
            vec![in &self.bumpalo; ty1, ty2].into_bump_slice(),
        )
    }
    pub fn dict(&'a self, reason: &'a Reason<'a>, kty: &'a Ty<'a>, vty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_dict,
            vec![in &self.bumpalo; kty, vty].into_bump_slice(),
        )
    }
    pub fn keyset(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_keyset,
            vec![in &self.bumpalo; ty].into_bump_slice(),
        )
    }
    pub fn vec(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_vec,
            vec![in &self.bumpalo; ty].into_bump_slice(),
        )
    }
    pub fn container(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_container,
            vec![in &self.bumpalo; ty].into_bump_slice(),
        )
    }
    pub fn const_vector(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_const_vector,
            vec![in &self.bumpalo; ty].into_bump_slice(),
        )
    }
    pub fn const_collection(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_const_collection,
            vec![in &self.bumpalo; ty].into_bump_slice(),
        )
    }
    pub fn collection(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_collection,
            vec![in &self.bumpalo; ty].into_bump_slice(),
        )
    }
    pub fn throwable(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_throwable,
            vec![in &self.bumpalo; ].into_bump_slice(),
        )
    }
    pub fn datetime(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_datetime,
            vec![in &self.bumpalo].into_bump_slice(),
        )
    }
    pub fn datetime_immutable(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.class(
            reason,
            self.id_datetime_immutable,
            vec![in &self.bumpalo].into_bump_slice(),
        )
    }
    pub fn int(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.prim(reason, PrimKind::Tint)
    }
    pub fn bool(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.prim(reason, PrimKind::Tbool)
    }
    pub fn string(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.prim(reason, PrimKind::Tstring)
    }
    pub fn float(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.prim(reason, PrimKind::Tfloat)
    }
    pub fn arraykey(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.prim(reason, PrimKind::Tarraykey)
    }
    pub fn void(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.prim(reason, PrimKind::Tvoid)
    }
    pub fn null(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.prim(reason, PrimKind::Tnull)
    }
    pub fn nonnull(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tnonnull)
    }
    pub fn dynamic(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tdynamic)
    }
    /*
    pub fn object(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tobject)
    }
    */
    pub fn tyvar(&'a self, reason: &'a Reason<'a>, v: ident::Ident) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tvar(v))
    }
    pub fn union(&'a self, reason: &'a Reason<'a>, tys: &'a [&'a Ty<'a>]) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tunion(tys))
    }
    pub fn intersection(&'a self, reason: &'a Reason<'a>, tys: &'a [&'a Ty<'a>]) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tintersection(tys))
    }
    pub fn nothing(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.union(reason, &[])
    }
    pub fn nullable(&'a self, reason: &'a Reason<'a>, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Toption(ty))
    }
    pub fn mixed(&'a self, reason: &'a Reason<'a>) -> &'a Ty<'a> {
        self.nullable(reason, self.nonnull(reason))
    }
    pub fn generic(&'a self, reason: &'a Reason<'a>, name: &'a str) -> &'a Ty<'a> {
        // TODO (T69662957) must fill type args of Tgeneric
        self.mk(reason, Ty_::Tgeneric(self.alloc((name, &[][..]))))
    }
    pub fn fun(&'a self, reason: &'a Reason<'a>, ft: &'a FunType<'a>) -> &'a Ty<'a> {
        self.mk(reason, Ty_::Tfun(ft))
    }
    pub fn loclty(&'a self, ty: &'a Ty<'a>) -> InternalType<'a> {
        InternalType::LoclType(ty)
    }
    pub fn constraintty(&'a self, ty: ConstraintType<'a>) -> InternalType<'a> {
        InternalType::ConstraintType(ty)
    }
    pub fn funparam(&'a self, type_: &'a Ty<'a>) -> &'a FunParam<'a> {
        self.alloc(FunParam {
            type_: self.alloc(PossiblyEnforcedTy {
                type_,
                // TODO: set this correctly
                enforced: Enforcement::Unenforced,
            }),
            // TODO: set the following fields correctly
            flags: FunParamFlags::empty(),
            name: None,
            pos: Pos::none(),
        })
    }
    pub fn funtype(
        &'a self,
        params: &'a [&'a FunParam<'a>],
        implicit_params: &'a FunImplicitParams<'a>,
        ret: &'a Ty<'a>,
    ) -> &'a FunType<'a> {
        self.alloc(FunType {
            ret: self.alloc(PossiblyEnforcedTy {
                type_: ret,
                // TODO: set this correctly
                enforced: Enforcement::Unenforced,
            }),
            params,
            implicit_params,
            // TODO: set the following fields correctly
            arity: FunArity::Fstandard,
            tparams: &[],
            where_constraints: &[],
            flags: FunTypeFlags::empty(),
            ifc_decl: IfcFunDecl::FDPolicied(Some("#PUBLIC")),
        })
    }
}

/// Subtype props
impl<'a> TypeBuilder<'a> {
    pub fn conj(&'a self, v: BVec<'a, SubtypeProp<'a>>) -> SubtypeProp<'a> {
        self.alloc(SubtypePropEnum::Conj(v.into()))
    }
    pub fn disj(&'a self, v: BVec<'a, SubtypeProp<'a>>) -> SubtypeProp<'a> {
        self.alloc(SubtypePropEnum::Disj(v.into()))
    }
    pub fn is_subtype(&'a self, ty1: InternalType<'a>, ty2: InternalType<'a>) -> SubtypeProp<'a> {
        self.alloc(SubtypePropEnum::IsSubtype(ty1, ty2))
    }
    pub fn valid(&'a self) -> SubtypeProp<'a> {
        self.alloc(SubtypePropEnum::Conj(Vec::new_in(self.bumpalo)))
    }
    pub fn invalid(&'a self) -> SubtypeProp<'a> {
        self.alloc(SubtypePropEnum::Disj(Vec::new_in(self.bumpalo)))
    }
}

/// All reason builders go here
impl<'a> TypeBuilder<'a> {
    /// Make an Rnone reason. This does not belong here, but:
    pub fn mk_rnone(&'a self) -> &'a Reason<'a> {
        Reason::none()
    }

    pub fn mk_rinstantiate(
        &'a self,
        r0: &'a Reason<'a>,
        name: &'a str,
        r1: &'a Reason<'a>,
    ) -> &'a Reason<'a> {
        self.alloc(Reason::Rinstantiate(self.alloc((*r0, name, *r1))))
    }

    pub fn mk_rtype_variable_generics(
        &'a self,
        pos: &'a Pos,
        param_name: &'a str,
        fn_name: &'a str,
    ) -> &'a Reason<'a> {
        self.alloc(Reason::RtypeVariableGenerics(
            self.alloc((pos, param_name, fn_name)),
        ))
    }

    pub fn mk_rwitness(&'a self, pos: &'a Pos) -> &'a Reason<'a> {
        self.alloc(Reason::witness(pos))
    }
}

impl<'a> TypeBuilder<'a> {
    pub fn pos_none(&'a self) -> &'a Pos {
        Pos::none()
    }
}

impl<'a> TypeBuilder<'a> {
    pub fn env_with_self(&self) -> &mut ExpandEnv<'a> {
        // TODO(hrust) this_ty
        self.alloc(ExpandEnv {
            type_expansions: vec![in &self.bumpalo],
            substs: SMap::empty(),
        })
    }
}

#[macro_export]
macro_rules! avec {
    (in $bld:expr; $elem:expr; $n:expr) => (bumpalo::vec![in $bld.bumpalo(); $elem; $n]);
    (in $bld:expr) => (bumpalo::vec![in $bld.bumpalo()]);
    (in $bld:expr; $($x:expr),*) => (bumpalo::vec![in $bld.bumpalo(); $($x),*]);
    (in $bld:expr; $($x:expr,)*) => (bumpalo::vec![in $bld.bumpalo(); $($x),*]);
}

#[macro_export]
macro_rules! aset {
  ( ) => ({ Set::empty() });
  ( in $bld:expr; $($x:expr),* ) => ({
      let mut temp_map = Set::empty();
      $(
          temp_map = temp_map.add($bld, $x);
      )*
      temp_map
  });
}
