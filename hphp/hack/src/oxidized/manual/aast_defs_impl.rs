// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use eq_modulo_pos::EqModuloPos;
use serde::Deserialize;

use crate::aast_defs::*;
use crate::ast_defs::*;
use crate::pos::Pos;

impl<En, Ex> Argument<En, Ex> {
    pub fn to_expr(self) -> Expr<En, Ex> {
        match self {
            Argument::Anormal(e) => e,
            Argument::Ainout(_, e) => e,
        }
    }

    pub fn to_expr_ref(&self) -> &Expr<En, Ex> {
        match self {
            Argument::Anormal(e) => e,
            Argument::Ainout(_, e) => e,
        }
    }

    pub fn to_expr_mut(&mut self) -> &mut Expr<En, Ex> {
        match self {
            Argument::Anormal(e) => e,
            Argument::Ainout(_, e) => e,
        }
    }

    pub fn is_inout(&self) -> bool {
        matches!(self, Argument::Ainout(_, _))
    }

    pub fn is_lvar(&self) -> bool {
        matches!(self.to_expr_ref(), Expr(_, _, Expr_::Lvar(_)))
    }
}

impl Lid {
    pub fn new(p: Pos, s: String) -> Self {
        Self(p, (0, s))
    }

    pub fn from_counter(p: Pos, counter: isize, s: &str) -> Self {
        Self(p, (counter, String::from(s)))
    }

    pub fn name(&self) -> &String {
        crate::local_id::get_name(&self.1)
    }

    pub fn pos(&self) -> &Pos {
        &self.0
    }

    pub fn as_local_id(&self) -> &LocalId {
        &self.1
    }
}

impl Hint {
    pub fn new(p: Pos, h: Hint_) -> Self {
        Self(p, Box::new(h))
    }

    pub fn as_happly(&self) -> Option<(&Sid, &Vec<Hint>)> {
        self.1.as_happly()
    }

    pub fn is_hlike(&self) -> bool {
        self.1.is_hlike()
    }
}

impl AsRef<Hint> for Hint {
    fn as_ref(&self) -> &Self {
        self
    }
}

impl AsRef<str> for Id {
    fn as_ref(&self) -> &str {
        &self.1
    }
}

impl AsRef<str> for Visibility {
    fn as_ref(&self) -> &str {
        use Visibility::*;
        match self {
            Private => "private",
            Public => "public",
            Protected => "protected",
            Internal => "internal",
        }
    }
}

impl std::fmt::Display for Visibility {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.as_ref())
    }
}

impl<'a> ocamlrep::FromOcamlRepIn<'a> for PackageMembership {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        use ocamlrep::FromOcamlRep;
        Self::from_ocamlrep(value)
    }
}
arena_deserializer::impl_deserialize_in_arena!(PackageMembership);

impl EqModuloPos for PackageMembership {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        self == rhs
    }
    fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
        self == rhs
    }
}
impl PackageMembership {
    pub fn to_arena_allocated<'a>(&self, arena: &'a bumpalo::Bump) -> &'a PackageMembership {
        use PackageMembership::*;
        match self {
            PackageConfigAssignment(x) => arena.alloc(PackageConfigAssignment(x.clone())),
            PackageOverride(p, x) => arena.alloc(PackageOverride(p.clone(), x.clone())),
        }
    }
}
