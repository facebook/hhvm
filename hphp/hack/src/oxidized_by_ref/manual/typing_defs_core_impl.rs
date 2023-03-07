// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use crate::ast_defs::Id;
use crate::ast_defs::ParamKind;
use crate::ast_defs::Tprim;
use crate::ident::Ident;
use crate::pos::Pos;
use crate::typing_defs_core::*;
use crate::typing_reason::Reason;

impl<'a> From<Id<'a>> for PosId<'a> {
    fn from(Id(pos, id): Id<'a>) -> Self {
        (pos, id)
    }
}

// Compare two types syntactically, ignoring reason information and other
// small differences that do not affect type inference behaviour. This
// comparison function can be used to construct tree-based sets of types,
// or to compare two types for "exact" equality.
// Note that this function does *not* expand type variables, or type
// aliases.
// But if compare_locl/decl_ty ty1 ty2 = 0, then the types must not be distinguishable
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
        self.1.cmp(&other.1)
    }
}

impl<'a> Ty<'a> {
    pub fn mk(reason: &'a Reason<'a>, ty_: Ty_<'a>) -> Ty<'a> {
        Ty(reason, ty_)
    }
    pub fn unpack(&self) -> (&'a Reason<'a>, Ty_<'a>) {
        (self.0, self.1)
    }
    pub fn get_node(&self) -> Ty_<'a> {
        self.1
    }
    pub fn get_reason(&self) -> &'a Reason<'a> {
        let Ty(r, _t) = self;
        r
    }
    pub fn get_pos(&self) -> Option<&'a Pos<'a>> {
        self.get_reason().pos()
    }
    pub fn is_tyvar(&self) -> bool {
        match self.get_node() {
            Ty_::Tvar(_) => true,
            _ => false,
        }
    }
    pub fn is_generic(&self) -> bool {
        match self.get_node() {
            Ty_::Tgeneric(_) => true,
            _ => false,
        }
    }
    pub fn is_dynamic(&self) -> bool {
        match self.get_node() {
            Ty_::Tdynamic => true,
            _ => false,
        }
    }
    pub fn is_nonnull(&self) -> bool {
        match self.get_node() {
            Ty_::Tnonnull => true,
            _ => false,
        }
    }
    pub fn is_any(&self) -> bool {
        match self.get_node() {
            Ty_::Tany(_) => true,
            _ => false,
        }
    }
    pub fn is_prim(&self, kind: Tprim) -> bool {
        match self.get_node() {
            Ty_::Tprim(k) => kind == *k,
            _ => false,
        }
    }
    pub fn is_union(&self) -> bool {
        match self.get_node() {
            Ty_::Tunion(_) => true,
            _ => false,
        }
    }
    pub fn is_intersection(&self) -> bool {
        match self.get_node() {
            Ty_::Tintersection(_) => true,
            _ => false,
        }
    }
}

impl Ty<'_> {
    pub fn get_var(&self) -> Option<Ident> {
        match self.get_node() {
            Ty_::Tvar(v) => Some(v),
            _ => None,
        }
    }
}

impl std::fmt::Debug for Ty_<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use Ty_::*;
        match self {
            Tthis => write!(f, "Tthis"),
            Tapply((id, tys)) => f.debug_tuple("Tapply").field(id).field(tys).finish(),
            Trefinement((ty, rs)) => f.debug_tuple("Trefinement").field(ty).field(rs).finish(),
            Taccess(taccess) => f.debug_tuple("Taccess").field(taccess).finish(),
            Tmixed => write!(f, "Tmixed"),
            Tlike(ty) => f.debug_tuple("Tlike").field(ty).finish(),
            Tany(_) => write!(f, "Tany"),
            Tnonnull => write!(f, "Tnonnull"),
            Tdynamic => write!(f, "Tdynamic"),
            TunappliedAlias(name) => write!(f, "TunappliedAlias({:?})", name),
            Toption(ty) => f.debug_tuple("Toption").field(ty).finish(),
            Tprim(tprim) => write!(f, "Tprim({:?})", tprim),
            Tneg(tprim) => write!(f, "Tneg({:?})", tprim),
            Tfun(fun_type) => f.debug_tuple("Tfun").field(fun_type).finish(),
            Ttuple(tys) => f.debug_tuple("Ttuple").field(tys).finish(),
            Tshape((kind, fields)) => f.debug_tuple("Tshape").field(kind).field(fields).finish(),
            Tvar(ident) => f.debug_tuple("Tvar").field(ident).finish(),
            Tgeneric(name) => write!(f, "Tgeneric({:?})", name),
            Tunion(tys) => f.debug_tuple("Tunion").field(tys).finish(),
            Tintersection(tys) => f.debug_tuple("Tintersection").field(tys).finish(),
            TvecOrDict((tk, tv)) => f.debug_tuple("TvecOrDict").field(tk).field(tv).finish(),
            Tnewtype((name, tys, constraint)) => f
                .debug_tuple("Tnewtype")
                .field(name)
                .field(tys)
                .field(constraint)
                .finish(),
            Tdependent((dependent_type, ty)) => f
                .debug_tuple("Tdependent")
                .field(dependent_type)
                .field(ty)
                .finish(),
            Tclass((id, exact, tys)) => f
                .debug_tuple("Tclass")
                .field(id)
                .field(exact)
                .field(tys)
                .finish(),
        }
    }
}

impl<'a> From<ParamKind<'a>> for ParamMode {
    fn from(callconv: ParamKind<'_>) -> Self {
        match callconv {
            ParamKind::Pinout(_) => ParamMode::FPinout,
            ParamKind::Pnormal => ParamMode::FPnormal,
        }
    }
}
