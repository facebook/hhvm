// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use hash::HashMap;
use oxidized::aast_defs::UserAttributes;
use oxidized::ast_defs::Id;
use oxidized::ast_defs::Pos;
use oxidized::naming_error::NamingError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabUserAttributesPass;

impl Pass for ElabUserAttributesPass {
    fn on_ty_user_attributes_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut UserAttributes<Ex, En>,
        cfg: &Config,
    ) -> ControlFlow<(), ()> {
        let mut seen: HashMap<String, Pos> = HashMap::default();
        let UserAttributes(uas) = elem;
        uas.retain(|ua| {
            let Id(pos, attr_name) = &ua.name;
            if let Some(prev_pos) = seen.get(attr_name) {
                cfg.emit_error(NamingError::DuplicateUserAttribute {
                    pos: pos.clone(),
                    attr_name: attr_name.clone(),
                    prev_pos: prev_pos.clone(),
                });
                false
            } else {
                seen.insert(attr_name.clone(), pos.clone());
                true
            }
        });
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {
    use oxidized::aast_defs::UserAttribute;

    use super::*;
    use crate::Transform;

    // Elaboration of CIexpr(..,..,Id(..,..)) when the id refers to a class
    #[test]
    fn test_ciexpr_id_class_ref() {
        let cfg = Config::default();

        let mut pass = ElabUserAttributesPass;
        let mut elem: UserAttributes<(), ()> = UserAttributes(vec![
            UserAttribute {
                name: Id(Pos::NONE, "One".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::NONE, "Two".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::NONE, "One".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::NONE, "Two".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::NONE, "One".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::NONE, "Two".to_string()),
                params: vec![],
            },
        ]);

        elem.transform(&cfg, &mut pass);
        assert_eq!(elem.len(), 2);
        assert_eq!(cfg.into_errors().len(), 4);
    }
}
