// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::ModuleDef;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateModulePass;

impl Pass for ValidateModulePass {
    fn on_ty_module_def_bottom_up(&mut self, env: &Env, module: &mut ModuleDef) -> ControlFlow<()> {
        if !env.allow_module_declarations() {
            env.emit_error(NamingError::ModuleDeclarationOutsideAllowedFiles(
                module.span.clone(),
            ));
        }
        Continue(())
    }
}

#[cfg(test)]
mod tests {

    use nast::Id;
    use nast::ModuleDef;
    use nast::Pos;
    use nast::UserAttributes;
    use oxidized::typechecker_options::TypecheckerOptions;

    use super::*;
    use crate::env::ProgramSpecificOptions;

    fn mk_module(name: &str) -> ModuleDef {
        ModuleDef {
            annotation: (),
            name: Id(Pos::NONE, name.to_string()),
            user_attributes: UserAttributes::default(),
            file_attributes: vec![],
            span: Pos::NONE,
            mode: oxidized::file_info::Mode::Mstrict,
            doc_comment: None,
            exports: None,
            imports: None,
        }
    }

    #[test]
    fn test_module_def_not_allowed() {
        let env = Env::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions {
                allow_module_declarations: false,
                ..Default::default()
            },
        );
        let mut module = mk_module("foo");
        module.transform(&env, &mut ValidateModulePass);
        assert!(matches!(
            env.into_errors().as_slice(),
            [NamingPhaseError::Naming(
                NamingError::ModuleDeclarationOutsideAllowedFiles(_)
            )]
        ));
    }
}
