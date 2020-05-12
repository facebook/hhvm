// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::ToOxidized;

use crate::ast_defs::*;
use crate::pos::Pos;

impl<'a> ShapeFieldName<'a> {
    pub fn get_name(&self) -> &'a str {
        use ShapeFieldName::*;
        match self {
            SFlitInt((_, name)) | SFlitStr((_, name)) | SFclassConst(_, (_, name)) => name,
        }
    }

    pub fn get_pos(&self) -> &'a Pos {
        use ShapeFieldName::*;
        match self {
            SFlitInt((p, _)) | SFlitStr((p, _)) | SFclassConst(_, (p, _)) => p,
        }
    }
}

impl<'a> Id<'a> {
    pub fn pos(&self) -> &'a Pos {
        self.0
    }

    pub fn name(&self) -> &'a str {
        self.1
    }
}

impl<'a> ToOxidized for Id<'a> {
    type Target = oxidized::ast_defs::Id;

    /// This is a wasteful implementation of to_oxidized, for debugging only. It
    /// does not preserve sharing of filenames in positions, and it allocates an
    /// intermediate Rust Vec to hold an OCaml representation (because we do not
    /// currently have a generated means of directly converting an
    /// oxidized_by_ref value to an oxidized one, so we use ToOcamlRep and
    /// FromOcamlRep instead).
    fn to_oxidized(&self) -> Self::Target {
        let arena = ocamlrep::Arena::new();
        let ocaml_self = arena.add(self);
        use ocamlrep::FromOcamlRep;
        oxidized::ast_defs::Id::from_ocamlrep(ocaml_self).unwrap()
    }
}

impl<'a> Bop<'a> {
    pub fn is_any_eq(&self) -> bool {
        match self {
            Self::Eq(_) => true,
            _ => false,
        }
    }
}
