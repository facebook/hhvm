// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use nast::Class_;
use nast::Fun_;
use nast::Method_;
use nast::UserAttribute;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ValidateUserAttributeArityPass;

impl Pass for ValidateUserAttributeArityPass {
    fn on_ty_fun__bottom_up(&mut self, env: &Env, elem: &mut Fun_) -> ControlFlow<(), ()> {
        AritySpec::Range(0, 1).check(sn::user_attributes::POLICIED, &elem.user_attributes, env);
        AritySpec::Exact(0).check(sn::user_attributes::INFERFLOWS, &elem.user_attributes, env);
        AritySpec::Range(1, 2).check(sn::user_attributes::DEPRECATED, &elem.user_attributes, env);
        elem.params.iter().for_each(|fp| {
            AritySpec::Exact(0).check(sn::user_attributes::EXTERNAL, &fp.user_attributes, env);
            AritySpec::Exact(0).check(sn::user_attributes::CAN_CALL, &fp.user_attributes, env);
        });
        ControlFlow::Continue(())
    }

    fn on_ty_class__bottom_up(&mut self, env: &Env, elem: &mut Class_) -> ControlFlow<(), ()> {
        AritySpec::Exact(1).check(sn::user_attributes::DOCS, &elem.user_attributes, env);
        ControlFlow::Continue(())
    }

    fn on_ty_method__bottom_up(&mut self, env: &Env, elem: &mut Method_) -> ControlFlow<(), ()> {
        AritySpec::Range(0, 1).check(sn::user_attributes::POLICIED, &elem.user_attributes, env);
        AritySpec::Exact(0).check(sn::user_attributes::INFERFLOWS, &elem.user_attributes, env);
        AritySpec::Range(1, 2).check(sn::user_attributes::DEPRECATED, &elem.user_attributes, env);
        ControlFlow::Continue(())
    }
}

#[derive(Copy, Clone)]
enum AritySpec {
    Exact(usize),
    Range(usize, usize),
}

impl AritySpec {
    fn check(self, attr_name: &str, user_attrs: &[UserAttribute], env: &Env) {
        if let Some(ua) = user_attrs.iter().find(|ua| ua.name.name() == attr_name) {
            let actual = ua.params.len();
            match self {
                AritySpec::Range(expected, _) if actual < expected => {
                    env.emit_error(NastCheckError::AttributeTooFewArguments {
                        pos: ua.name.pos().clone(),
                        name: ua.name.name().to_string(),
                        expected: expected.try_into().unwrap(),
                    })
                }
                AritySpec::Range(_, expected) if actual > expected => {
                    env.emit_error(NastCheckError::AttributeTooManyArguments {
                        pos: ua.name.pos().clone(),
                        name: ua.name.name().to_string(),
                        expected: expected.try_into().unwrap(),
                    })
                }
                AritySpec::Exact(expected) if actual != expected => {
                    env.emit_error(NastCheckError::AttributeNotExactNumberOfArgs {
                        pos: ua.name.pos().clone(),
                        name: ua.name.name().to_string(),
                        expected: expected.try_into().unwrap(),
                        actual: actual.try_into().unwrap(),
                    })
                }
                _ => (),
            }
        }
    }
}
