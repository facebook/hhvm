// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use crate::ast_defs::Id;
use crate::pos::Pos;
use crate::typing_defs::TypedefTypeAssignment;
use crate::typing_defs_core::ConsistentKind;
use crate::typing_defs_core::PosId;
use crate::typing_defs_core::Ty;
use crate::typing_defs_core::Ty_;
use crate::typing_reason::Reason;

impl From<Id> for PosId {
    fn from(Id(pos, id): Id) -> Self {
        (pos, id)
    }
}

impl ConsistentKind {
    /// If the parent's constructor is consistent via `<<__ConsistentConstruct>>`,
    /// then we want to carry this forward even if the child is final. Example:
    ///
    /// ```
    /// <<__ConsistentConstruct>>
    /// class C {
    ///   public static function f(): void {
    ///     new static();
    ///   }
    /// }
    /// final class D<reify T> extends C {}
    /// ```
    ///
    /// Even though D's consistency locally comes from the fact that D was
    /// declared as a final class, calling `new static()` via `D::f` will cause
    /// a runtime exception because D has reified generics.
    ///
    /// c.f. OCaml function `Decl_utils.coalesce_consistent`
    pub fn coalesce(parent: Self, current: Self) -> Self {
        match parent {
            Self::Inconsistent => current,
            Self::ConsistentConstruct => parent,
            // This case is unreachable in a correct program, because parent
            // would have to be a final class
            Self::FinalClass => parent,
        }
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
    pub fn get_reason(&self) -> &Reason {
        &self.0
    }

    pub fn get_pos(&self) -> Option<&Pos> {
        self.get_reason().pos()
    }

    pub fn get_node(&self) -> &Ty_ {
        &self.1
    }
}

impl std::fmt::Debug for Ty_ {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use Ty_::*;
        match self {
            Tthis => write!(f, "Tthis"),
            Tapply(id, tys) => f.debug_tuple("Tapply").field(id).field(tys).finish(),
            Trefinement(ty, rs) => f.debug_tuple("Trefinement").field(ty).field(rs).finish(),
            Taccess(taccess) => f.debug_tuple("Taccess").field(taccess).finish(),
            Tmixed => write!(f, "Tmixed"),
            Twildcard => write!(f, "Twildcard"),
            Tlike(ty) => f.debug_tuple("Tlike").field(ty).finish(),
            Tany(_) => write!(f, "Tany"),
            Tnonnull => write!(f, "Tnonnull"),
            Tdynamic => write!(f, "Tdynamic"),
            Toption(ty) => f.debug_tuple("Toption").field(ty).finish(),
            Tprim(tprim) => write!(f, "Tprim({:?})", tprim),
            Tneg(tprim) => write!(f, "Tneg({:?})", tprim),
            Tlabel(name) => write!(f, "Tlabel({:?})", name),
            Tfun(fun_type) => f.debug_tuple("Tfun").field(fun_type).finish(),
            Ttuple(tys) => f.debug_tuple("Ttuple").field(tys).finish(),
            Tshape(shape) => f.debug_tuple("Tshape").field(shape).finish(),
            Tvar(ident) => f.debug_tuple("Tvar").field(ident).finish(),
            Tgeneric(name) => f.debug_tuple("Tgeneric").field(name).finish(),
            Tunion(tys) => f.debug_tuple("Tunion").field(tys).finish(),
            Tintersection(tys) => f.debug_tuple("Tintersection").field(tys).finish(),
            TvecOrDict(tk, tv) => f.debug_tuple("TvecOrDict").field(tk).field(tv).finish(),
            Tnewtype(name, tys, constraint) => f
                .debug_tuple("Tnewtype")
                .field(name)
                .field(tys)
                .field(constraint)
                .finish(),
            Tdependent(dependent_type, ty) => f
                .debug_tuple("Tdependent")
                .field(dependent_type)
                .field(ty)
                .finish(),
            Tclass(id, exact, tys) => f
                .debug_tuple("Tclass")
                .field(id)
                .field(exact)
                .field(tys)
                .finish(),
            TclassPtr(ty) => f.debug_tuple("TclassPtr").field(ty).finish(),
        }
    }
}

// Add an extra tuple to match oxidized_by_ref. Can remove once we remove o_b_r.
impl std::fmt::Debug for TypedefTypeAssignment {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            TypedefTypeAssignment::SimpleTypeDef(x, y) => {
                f.debug_tuple("SimpleTypeDef").field(&(x, y)).finish()
            }
            TypedefTypeAssignment::CaseType(x, y) => {
                f.debug_tuple("CaseType").field(&(x, y)).finish()
            }
        }
    }
}
