// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use file_info::Mode;
use naming_special_names_rust as sn;
use oxidized::aast::Expr;
use oxidized::aast::Expr_;
use oxidized::aast::Gconst;
use oxidized::ast::Id;
use oxidized::naming_error::NamingError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Default)]
pub struct ValidateGlobalConstPass;

impl Pass for ValidateGlobalConstPass {
    fn on_ty_gconst_bottom_up<Ex: Default, En>(
        &mut self,
        gconst: &mut Gconst<Ex, En>,
        cfg: &Config,
    ) -> ControlFlow<(), ()> {
        error_if_no_typehint(cfg, gconst);
        error_if_pseudo_constant(cfg, gconst);
        ControlFlow::Continue(())
    }
}

fn error_if_no_typehint<Ex, En>(cfg: &Config, gconst: &Gconst<Ex, En>) {
    if !matches!(gconst.mode, Mode::Mhhi) && matches!(gconst.type_, None) {
        let Expr(_, _, expr_) = &gconst.value;
        let Id(pos, const_name) = &gconst.name;
        let ty_name = match expr_ {
            Expr_::String(_) => "string",
            Expr_::Int(_) => "int",
            Expr_::Float(_) => "float",
            _ => "mixed",
        };
        cfg.emit_error(NamingError::ConstWithoutTypehint {
            pos: pos.clone(),
            const_name: const_name.clone(),
            ty_name: ty_name.to_string(),
        });
    }
}

fn error_if_pseudo_constant<Ex, En>(cfg: &Config, gconst: &Gconst<Ex, En>) {
    if gconst.namespace.name.is_some() {
        let Id(pos, n) = &gconst.name;
        let name = core_utils_rust::add_ns(core_utils_rust::strip_all_ns(n));
        if sn::pseudo_consts::is_pseudo_const(&name) {
            cfg.emit_error(NamingError::NameIsReserved {
                pos: pos.clone(),
                name: name.to_string(),
            });
        }
    }
}

#[cfg(test)]
mod tests {
    use ocamlrep::rc::RcOc;
    use oxidized::aast::Hint;
    use oxidized::aast::Pos;
    use oxidized::namespace_env;
    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::typechecker_options::TypecheckerOptions;

    use super::*;
    use crate::config::ProgramSpecificOptions;
    use crate::elab_utils;
    use crate::Transform;

    fn mk_gconst(
        name: String,
        value: Expr<(), ()>,
        r#type: Option<Hint>,
        namespace: Option<String>,
    ) -> Gconst<(), ()> {
        Gconst {
            annotation: (),
            mode: file_info::Mode::Mstrict,
            name: Id(Pos::NONE, name),
            value,
            type_: r#type,
            namespace: RcOc::new(namespace_env::Env {
                name: namespace,
                ..namespace_env::Env::empty(vec![], false, false)
            }),
            span: Pos::NONE,
            emit_id: None,
        }
    }

    #[test]
    fn test_no_type_hint() {
        let cfg = Config::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions::default(),
        );
        let mut r#const = mk_gconst("FOO".to_string(), elab_utils::expr::null(), None, None);
        r#const.transform(&cfg, &mut ValidateGlobalConstPass);
        assert!(matches!(
            cfg.into_errors().as_slice(),
            [NamingPhaseError::Naming(
                NamingError::ConstWithoutTypehint { .. }
            )]
        ));
    }

    #[test]
    fn test_pseudo_constant() {
        let cfg = Config::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions::default(),
        );
        let mut r#const = mk_gconst(
            "\\A\\B\\C\\__CLASS__".to_string(),
            elab_utils::expr::null(),
            Some(elab_utils::hint::null()),
            Some("Foo".to_string()),
        );
        r#const.transform(&cfg, &mut ValidateGlobalConstPass);
        assert!(matches!(
            cfg.into_errors().as_slice(),
            [NamingPhaseError::Naming(NamingError::NameIsReserved { .. })]
        ));
    }
}
