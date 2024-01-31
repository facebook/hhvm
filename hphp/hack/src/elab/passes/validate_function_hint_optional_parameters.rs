// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::HfParamInfo;
use nast::Hint;
use nast::Hint_;

use crate::nast::OptionalKind;
use crate::nast::ParamKind;
use crate::nast::Pos;
use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateFunctionHintOptionalParametersPass;

fn is_optional_param(p: &Option<HfParamInfo>) -> bool {
    match p {
        Some(HfParamInfo {
            optional: Some(OptionalKind::Optional),
            ..
        }) => true,
        _ => false,
    }
}

fn is_inout_param(p: &Option<HfParamInfo>) -> Option<&Pos> {
    match p {
        Some(HfParamInfo {
            kind: ParamKind::Pinout(pos),
            ..
        }) => Some(pos),
        _ => None,
    }
}

impl Pass for ValidateFunctionHintOptionalParametersPass {
    // Reject optional parameter preceding non-optional parameter
    // e.g. (function(int,optional bool,string):void)
    fn on_ty_hint_bottom_up(&mut self, env: &Env, hint: &mut Hint) -> ControlFlow<()> {
        match hint {
            Hint(pos, box Hint_::Hfun(hint_fun)) => {
                let mut previous_optional_param = None;
                for p in &hint_fun.param_info {
                    if is_optional_param(p) {
                        previous_optional_param = Some(p);
                        match is_inout_param(p) {
                            None => (),
                            Some(pos) => {
                                env.emit_error(NamingPhaseError::Parsing(
                                    ParsingError::ParsingError {
                                        pos: pos.clone(),
                                        msg: "Optional parameter cannot be inout".to_string(),
                                        quickfixes: vec![],
                                    },
                                ));
                            }
                        }
                    } else {
                        match previous_optional_param {
                            None => (),
                            Some(_) => {
                                env.emit_error(NamingPhaseError::Parsing(ParsingError::ParsingError {
                                pos: pos.clone(),
                                msg: "Optional parameter cannot precede non-optional parameter"
                                    .to_string(),
                                quickfixes: vec![],
                            }));
                            }
                        }
                    }
                }
            }
            _ => (),
        }
        Continue(())
    }
}
