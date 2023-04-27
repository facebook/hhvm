// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::HashMap;
use nast::Id;
use nast::Pos;
use nast::UserAttributes;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ElabUserAttributesPass;

impl Pass for ElabUserAttributesPass {
    fn on_ty_user_attributes_top_down(
        &mut self,
        env: &Env,
        elem: &mut UserAttributes,
    ) -> ControlFlow<()> {
        let mut seen: HashMap<String, Pos> = HashMap::default();
        let UserAttributes(uas) = elem;
        uas.retain(|ua| {
            let Id(pos, attr_name) = &ua.name;
            if let Some(prev_pos) = seen.get(attr_name) {
                env.emit_error(NamingError::DuplicateUserAttribute {
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
        Continue(())
    }
}

#[cfg(test)]
mod tests {
    use nast::UserAttribute;

    use super::*;

    // Elaboration of CIexpr(..,..,Id(..,..)) when the id refers to a class
    #[test]
    fn test_ciexpr_id_class_ref() {
        let env = Env::default();

        let mut pass = ElabUserAttributesPass;
        let mut elem = UserAttributes(vec![
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

        elem.transform(&env, &mut pass);
        assert_eq!(elem.len(), 2);
        assert_eq!(env.into_errors().len(), 4);
    }
}
