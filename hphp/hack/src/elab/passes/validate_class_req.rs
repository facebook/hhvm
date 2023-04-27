// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::ClassReq;
use nast::Class_;
use nast::ClassishKind;
use nast::Hint;
use nast::RequireKind;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateClassReqPass;

impl Pass for ValidateClassReqPass {
    fn on_ty_class__top_down(&mut self, env: &Env, cls: &mut Class_) -> ControlFlow<()> {
        let is_trait = cls.kind == ClassishKind::Ctrait;
        let is_interface = cls.kind == ClassishKind::Cinterface;
        let find_req = |kind| cls.reqs.iter().find(|&&ClassReq(_, k)| k == kind);
        // `require implements` and `require class` are only allowed in traits.
        if !is_trait {
            if let Some(ClassReq(Hint(pos, _), _)) = find_req(RequireKind::RequireImplements) {
                env.emit_error(NamingError::InvalidRequireImplements(pos.clone()));
            }
            if let Some(ClassReq(Hint(pos, _), _)) = find_req(RequireKind::RequireClass) {
                env.emit_error(NamingError::InvalidRequireClass(pos.clone()));
            }
        }
        // `require extends` is only allowed in traits and interfaces, so if
        // this classish is neither that's an error.
        if !(is_trait || is_interface) {
            if let Some(ClassReq(Hint(pos, _), _)) = find_req(RequireKind::RequireExtends) {
                env.emit_error(NamingError::InvalidRequireExtends(pos.clone()));
            }
        }
        Continue(())
    }
}
