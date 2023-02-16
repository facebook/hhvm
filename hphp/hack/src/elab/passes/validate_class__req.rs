// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::aast::ClassHint;
use oxidized::aast::Hint;
use oxidized::aast::RequireKind;
use oxidized::aast_defs::ClassReq;
use oxidized::aast_defs::Class_;
use oxidized::ast_defs::ClassishKind;
use oxidized::naming_error::NamingError;
use oxidized::naming_phase_error::NamingPhaseError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
#[allow(non_camel_case_types)]
pub struct ValidateClass_ReqPass;

// Splits `reqs` into "extends", "implements" and "class" requirements c.f.
// 'hphp/hack/src/annotated_ast/aast.ml'
// TODO(sf, 2023-02-15): Perhaps this should go into a hackrs/annotated_ast
// crate or elsewhere?
fn split_reqs(reqs: &[ClassReq]) -> (Vec<&ClassHint>, Vec<&ClassHint>, Vec<&ClassHint>) {
    let mut extends = vec![];
    let mut implements = vec![];
    let mut class_ = vec![];
    for ClassReq(h, require_kind) in reqs {
        match require_kind {
            RequireKind::RequireExtends => extends.push(h),
            RequireKind::RequireImplements => implements.push(h),
            RequireKind::RequireClass => class_.push(h),
        }
    }
    (extends, implements, class_)
}

impl Pass for ValidateClass_ReqPass {
    fn on_ty_class__top_down<Ex: Default, En>(
        &mut self,
        elem: &mut Class_<Ex, En>,
        _cfg: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()> {
        let Class_ { reqs, kind, .. } = elem;
        let (req_extends, req_implements, req_class) = split_reqs(reqs);
        let is_trait = *kind == ClassishKind::Ctrait;
        let is_interface = *kind == ClassishKind::Cinterface;
        match &req_implements[..] {
            // If `req_implements` is non-empty & this class like thing is not a
            // trait then that's an error.
            [Hint(pos, _), ..] if !is_trait => {
                errs.push(NamingPhaseError::Naming(
                    NamingError::InvalidRequireImplements(pos.clone()),
                ));
            }
            _ => (),
        }
        match &req_class[..] {
            // If `req_class` is non-empty & this class like thing is not a
            // trait then that's an error.
            [Hint(pos, _), ..] if !is_trait => {
                errs.push(NamingPhaseError::Naming(NamingError::InvalidRequireClass(
                    pos.clone(),
                )));
            }
            _ => (),
        }
        match &req_extends[..] {
            // If `req_extends` is non-empty & this class like thing is not a
            // trait or an interface then that's an error.
            [Hint(pos, _), ..] if !(is_trait || is_interface) => {
                errs.push(NamingPhaseError::Naming(
                    NamingError::InvalidRequireExtends(pos.clone()),
                ));
            }
            _ => (),
        }
        ControlFlow::Continue(())
    }
}
