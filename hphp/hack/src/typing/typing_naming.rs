// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_env_types::Env;
use naming_special_names_rust::typehints;
use oxidized::aast::{Hint, Hint_};
use oxidized::aast_defs::Tprim;
use oxidized::{aast, ast};
use typing_defs_rust::tast;

// In the OCaml code, this is done in naming.ml, and resolves an identifier wrt namespaces
// For now, just prefix with the top-level namespace
// TODO(hrust): align Rust and OCaml
pub fn canonicalize_sid(id: &ast::Sid) -> ast::Sid {
    tast::Id(id.0.clone(), canonicalize_str(&id.1))
}

pub fn canonicalize_class_id(id: &ast::ClassId) -> ast::ClassId {
    use aast::ClassId_::*;
    let id_new = match &id.1 {
        CIparent | CIself | CIstatic => id.1.clone(),
        CIexpr(..) => unimplemented!(),
        CI(sid) => CI(canonicalize_sid(&sid)),
    };
    ast::ClassId(id.0.clone(), id_new)
}

pub fn canonicalize_str(id: &str) -> String {
    "\\".to_string() + &id.to_string()
}

// In the OCaml code, this is done in naming.ml, in the hint_ function
pub fn name_hint<'a>(_env: &mut Env<'a>, h: &'a Hint) -> &'a Hint {
    let Hint(pos, node) = &*h;
    // TODO(hrust): complete the cases
    let newnode = match &**node {
        Hint_::Happly(x, _) => {
            if x.1 == typehints::INT {
                Some(Hint_::Hprim(Tprim::Tint))
            } else if x.1 == typehints::BOOL {
                Some(Hint_::Hprim(Tprim::Tbool))
            } else if x.1 == typehints::FLOAT {
                Some(Hint_::Hprim(Tprim::Tfloat))
            } else if x.1 == typehints::NUM {
                Some(Hint_::Hprim(Tprim::Tnum))
            } else {
                None
            }
        }
        _ => None,
    };

    match newnode {
        None => {
            return h;
        }
        Some(node) => {
            let h = Hint(pos.clone(), Box::new(node));
            Box::leak(Box::new(h))
        }
    }
}
