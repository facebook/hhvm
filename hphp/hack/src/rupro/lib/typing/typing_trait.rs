// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::reason::Reason;
use crate::typing::typing_env::TEnv;
use crate::typing::typing_error::Result;

pub trait TC<R: Reason> {
    type Typed;
    type Params;

    fn infer(&self, env: &TEnv<R>, params: Self::Params) -> Result<Self::Typed>;
}
