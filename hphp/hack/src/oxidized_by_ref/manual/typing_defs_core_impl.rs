// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use oxidized::ToOxidized;

use crate::aast_defs::Tprim;
use crate::ident::Ident;
use crate::pos::Pos;
use crate::typing_defs_core::*;
use crate::typing_reason::Reason;

// Compare two types syntactically, ignoring reason information and other
// small differences that do not affect type inference behaviour. This
// comparison function can be used to construct tree-based sets of types,
// or to compare two types for "exact" equality.
// Note that this function does *not* expand type variables, or type
// aliases.
// But if ty_compare ty1 ty2 = 0, then the types must not be distinguishable
// by any typing rules.
impl PartialEq for Ty<'_> {
    fn eq(&self, other: &Self) -> bool {
        self.1 == other.1
    }
}
impl Eq for Ty<'_> {}
impl PartialOrd for Ty<'_> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
impl Ord for Ty<'_> {
    fn cmp(&self, other: &Self) -> Ordering {
        self.1.cmp(other.1)
    }
}

impl<'a> Ty<'a> {
    pub fn mk(reason: &'a Reason<'a>, ty_: &'a Ty_<'a>) -> Ty<'a> {
        Ty(reason, ty_)
    }
    pub fn unpack(self) -> (&'a Reason<'a>, &'a Ty_<'a>) {
        (self.0, self.1)
    }
    pub fn get_node(self) -> &'a Ty_<'a> {
        let Ty(_r, t) = self;
        t
    }
    pub fn get_reason(self) -> &'a Reason<'a> {
        let Ty(r, _t) = self;
        r
    }
    pub fn get_pos(self) -> Option<&'a Pos<'a>> {
        self.get_reason().pos
    }
    pub fn is_tyvar(self) -> bool {
        match self.get_node() {
            Ty_::Tvar(_) => true,
            _ => false,
        }
    }
    pub fn is_generic(self) -> bool {
        match self.get_node() {
            Ty_::Tgeneric(_) => true,
            _ => false,
        }
    }
    pub fn is_dynamic(self) -> bool {
        match self.get_node() {
            Ty_::Tdynamic => true,
            _ => false,
        }
    }
    pub fn is_nonnull(self) -> bool {
        match self.get_node() {
            Ty_::Tnonnull => true,
            _ => false,
        }
    }
    pub fn is_any(self) -> bool {
        match self.get_node() {
            Ty_::Tany(_) => true,
            _ => false,
        }
    }
    pub fn is_prim(self, kind: Tprim) -> bool {
        match self.get_node() {
            Ty_::Tprim(k) => &kind == &**k,
            _ => false,
        }
    }
    pub fn is_union(self) -> bool {
        match self.get_node() {
            Ty_::Tunion(_) => true,
            _ => false,
        }
    }
    pub fn is_intersection(self) -> bool {
        match self.get_node() {
            Ty_::Tintersection(_) => true,
            _ => false,
        }
    }
}

impl Ty<'_> {
    pub fn get_var(&self) -> Option<Ident> {
        match self.get_node() {
            Ty_::Tvar(v) => Some(*v),
            _ => None,
        }
    }
}

impl<'a> ToOxidized for Ty<'a> {
    type Target = oxidized::typing_defs_core::Ty;

    /// This is a wasteful implementation of to_oxidized, for debugging only. It
    /// does not preserve sharing of filenames in positions, and it allocates an
    /// intermediate Rust Vec to hold an OCaml representation (because we do not
    /// currently have a generated means of directly converting an
    /// oxidized_by_ref value to an oxidized one, so we use ToOcamlRep and
    /// FromOcamlRep instead).
    fn to_oxidized(&self) -> Self::Target {
        let arena = ocamlrep::Arena::new();
        let ocaml_ty = arena.add(self);
        use ocamlrep::FromOcamlRep;
        oxidized::typing_defs_core::Ty::from_ocamlrep(ocaml_ty).unwrap()
    }
}

impl PartialEq for ConstraintType<'_> {
    fn eq(&self, other: &Self) -> bool {
        self.1 == other.1
    }
}
impl Eq for ConstraintType<'_> {}
impl PartialOrd for ConstraintType<'_> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
impl Ord for ConstraintType<'_> {
    fn cmp(&self, other: &Self) -> Ordering {
        self.1.cmp(other.1)
    }
}

impl<'a> InternalType<'a> {
    pub fn get_locl_type_opt(self) -> Option<Ty<'a>> {
        match self {
            InternalType::LoclType(ty) => Some(ty),
            InternalType::ConstraintType(_) => None,
        }
    }

    pub fn get_var(self) -> Option<Ident> {
        match self {
            InternalType::LoclType(ty) => ty.get_var(),
            InternalType::ConstraintType(_) => None,
        }
    }
}

impl<'a> ToOxidized for InternalType<'a> {
    type Target = oxidized::typing_defs_core::InternalType;

    /// This is a wasteful implementation of to_oxidized, for debugging only. It
    /// does not preserve sharing of filenames in positions, and it allocates an
    /// intermediate Rust Vec to hold an OCaml representation (because we do not
    /// currently have a generated means of directly converting an
    /// oxidized_by_ref value to an oxidized one, so we use ToOcamlRep and
    /// FromOcamlRep instead).
    fn to_oxidized(&self) -> Self::Target {
        let arena = ocamlrep::Arena::new();
        let ocaml_ty = arena.add(self);
        use ocamlrep::FromOcamlRep;
        oxidized::typing_defs_core::InternalType::from_ocamlrep(ocaml_ty).unwrap()
    }
}
