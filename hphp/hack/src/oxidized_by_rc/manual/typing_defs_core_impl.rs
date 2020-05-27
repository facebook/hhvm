// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::rc::Rc;

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
impl PartialEq for Ty {
    fn eq(&self, other: &Self) -> bool {
        self.1 == other.1
    }
}
impl Eq for Ty {}
impl PartialOrd for Ty {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
impl Ord for Ty {
    fn cmp(&self, other: &Self) -> Ordering {
        self.1.cmp(&other.1)
    }
}

impl Ty {
    pub fn mk(reason: Rc<Reason>, ty_: Rc<Ty_>) -> Ty {
        Ty(reason, ty_)
    }
    pub fn unpack(&self) -> (&Rc<Reason>, &Rc<Ty_>) {
        (&self.0, &self.1)
    }
    pub fn get_node(&self) -> &Rc<Ty_> {
        let Ty(_r, t) = self;
        t
    }
    pub fn get_reason(&self) -> &Rc<Reason> {
        let Ty(r, _t) = self;
        r
    }
    pub fn get_pos(&self) -> &Option<Rc<Pos>> {
        &self.get_reason().pos
    }
    pub fn is_tyvar(&self) -> bool {
        match self.get_node().as_ref() {
            Ty_::Tvar(_) => true,
            _ => false,
        }
    }
    pub fn is_generic(&self) -> bool {
        match self.get_node().as_ref() {
            Ty_::Tgeneric(_) => true,
            _ => false,
        }
    }
    pub fn is_dynamic(&self) -> bool {
        match self.get_node().as_ref() {
            Ty_::Tdynamic => true,
            _ => false,
        }
    }
    pub fn is_nonnull(&self) -> bool {
        match self.get_node().as_ref() {
            Ty_::Tnonnull => true,
            _ => false,
        }
    }
    pub fn is_any(&self) -> bool {
        match self.get_node().as_ref() {
            Ty_::Tany(_) => true,
            _ => false,
        }
    }
    pub fn is_prim(&self, kind: Tprim) -> bool {
        match self.get_node().as_ref() {
            Ty_::Tprim(k) => &kind == k,
            _ => false,
        }
    }
    pub fn is_union(&self) -> bool {
        match self.get_node().as_ref() {
            Ty_::Tunion(_) => true,
            _ => false,
        }
    }
    pub fn is_intersection(&self) -> bool {
        match self.get_node().as_ref() {
            Ty_::Tintersection(_) => true,
            _ => false,
        }
    }
}

impl Ty {
    pub fn get_var(&self) -> Option<Ident> {
        match self.get_node().as_ref() {
            Ty_::Tvar(v) => Some(*v),
            _ => None,
        }
    }
}

impl ToOxidized for Ty {
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

impl PartialEq for ConstraintType {
    fn eq(&self, other: &Self) -> bool {
        self.1 == other.1
    }
}
impl Eq for ConstraintType {}
impl PartialOrd for ConstraintType {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
impl Ord for ConstraintType {
    fn cmp(&self, other: &Self) -> Ordering {
        self.1.cmp(&other.1)
    }
}

impl InternalType {
    pub fn get_locl_type_opt(&self) -> Option<Ty> {
        match self {
            InternalType::LoclType(ty) => Some(ty.clone()),
            InternalType::ConstraintType(_) => None,
        }
    }

    pub fn get_var(&self) -> Option<Ident> {
        match self {
            InternalType::LoclType(ty) => ty.get_var(),
            InternalType::ConstraintType(_) => None,
        }
    }
}

impl ToOxidized for InternalType {
    type Target = oxidized::typing_defs_core::InternalType;

    /// This is a wasteful implementation of to_oxidized, for debugging only. It
    /// does not preserve sharing of filenames in positions, and it allocates an
    /// intermediate Rust Vec to hold an OCaml representation (because we do not
    /// currently have a generated means of directly converting an
    /// oxidized_by_ref value to an oxidized one, so we use ToOcamlRep and
    /// FromOcamlRep instead).
    fn to_oxidized(&self) -> Self::Target {
        unimplemented!()
    }
}
