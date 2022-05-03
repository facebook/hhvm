// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{
    oracle::{NoClasses, Oracle},
    visited_goals::VisitedGoals,
};
use crate::inference_env::InferenceEnv;
use crate::typaram_env::TyparamEnv;
use std::{fmt::Debug, rc::Rc};
use ty::local::Ty;
use ty::reason::Reason;

#[derive(Debug, Clone)]
pub struct Env<R: Reason> {
    pub this_ty: Option<Ty<R>>,
    pub visited_goals: VisitedGoals<R>,
    pub inf_env: InferenceEnv<R>,
    pub tp_env: TyparamEnv<R>,
    pub decl_env: Rc<dyn Oracle<R>>,
}

impl<R: Reason> Default for Env<R> {
    fn default() -> Self {
        Env {
            this_ty: None,
            visited_goals: VisitedGoals::default(),
            inf_env: InferenceEnv::default(),
            tp_env: TyparamEnv::default(),
            decl_env: Rc::new(NoClasses),
        }
    }
}
