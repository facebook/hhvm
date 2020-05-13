// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_env_types::Env;
use naming_special_names_rust::typehints;
use oxidized::aast::{Hint, Hint_};
use oxidized::aast_defs::Tprim;
use oxidized::ast;
use oxidized_by_ref::ast::ClassId_;
use oxidized_by_ref::ast_defs::Id;
use typing_defs_rust::tast;

// In the OCaml code, this is done in naming.ml, and resolves an identifier wrt namespaces
// For now, just prefix with the top-level namespace
// TODO(hrust): align Rust and OCaml
pub fn canonicalize_sid(id: &ast::Sid) -> ast::Sid {
    tast::Id(id.0.clone(), canonicalize_str(&id.1))
}

pub fn canonicalize_class_id<'a>(env: &Env<'a>, id: &mut ClassId_<'a>) {
    use oxidized_by_ref::aast::ClassId_::*;
    match id {
        CIparent | CIself | CIstatic => (),
        CIexpr(..) => unimplemented!(),
        CI(Id(p, sid)) => {
            let new_sid = canonicalize_str(&sid);
            let new_sid = env.bld().str_from_str(&new_sid);
            *id = CI(Id(p, new_sid))
        }
    };
}

pub fn canonicalize_str(id: &str) -> String {
    "\\".to_string() + &id.to_string()
}

// In the OCaml code, this is done in naming.ml, in the hint_ function
pub fn name_hint<'a>(h: &Hint) -> Hint {
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
            return h.clone();
        }
        Some(node) => Hint(pos.clone(), Box::new(node)),
    }
}
