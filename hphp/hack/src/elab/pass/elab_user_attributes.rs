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
use oxidized::naming_phase_error::NamingPhaseError;
use transform::Pass;

use crate::context::Context;

pub struct ElabUserAttributesPass;

impl Pass for ElabUserAttributesPass {
    type Ctx = Context;
    type Err = NamingPhaseError;

    fn on_ty_user_attributes<Ex: Default, En>(
        &self,
        elem: &mut UserAttributes<Ex, En>,
        _ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        let mut seen: HashMap<String, Pos> = HashMap::default();
        let UserAttributes(uas) = elem;
        uas.retain(|ua| {
            let Id(pos, attr_name) = &ua.name;
            if let Some(prev_pos) = seen.get(attr_name) {
                errs.push(NamingPhaseError::Naming(
                    NamingError::DuplicateUserAttribute {
                        pos: pos.clone(),
                        attr_name: attr_name.clone(),
                        prev_pos: prev_pos.clone(),
                    },
                ));
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
    use transform::Transform;

    use super::*;

    pub struct Identity;
    impl Pass for Identity {
        type Err = NamingPhaseError;
        type Ctx = Context;
    }

    // Elaboration of CIexpr(..,..,Id(..,..)) when the id refers to a class
    #[test]
    fn test_ciexpr_id_class_ref() {
        let mut ctx = Context::default();
        let mut errs = Vec::default();
        let top_down = ElabUserAttributesPass;
        let bottom_up = Identity;
        let mut elem: UserAttributes<(), ()> = UserAttributes(vec![
            UserAttribute {
                name: Id(Pos::make_none(), "One".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::make_none(), "Two".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::make_none(), "One".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::make_none(), "Two".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::make_none(), "One".to_string()),
                params: vec![],
            },
            UserAttribute {
                name: Id(Pos::make_none(), "Two".to_string()),
                params: vec![],
            },
        ]);

        elem.transform(&mut ctx, &mut errs, &top_down, &bottom_up);
        assert_eq!(elem.len(), 2);
        assert_eq!(errs.len(), 4);
    }
}
