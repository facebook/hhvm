// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ty::reason::Reason;

use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;

pub trait Infer<R: Reason> {
    type Typed;
    type Params;

    fn infer(&self, env: &mut TEnv<R>, params: Self::Params) -> Result<Self::Typed>;
}

impl<R: Reason, T: Infer<R>> Infer<R> for &T {
    type Typed = T::Typed;
    type Params = T::Params;

    fn infer(&self, env: &mut TEnv<R>, params: Self::Params) -> Result<Self::Typed> {
        (*self).infer(env, params)
    }
}

impl<R: Reason, T> Infer<R> for [T]
where
    T: Infer<R>,
    T::Params: Clone,
{
    type Typed = Vec<T::Typed>;
    type Params = T::Params;

    fn infer(&self, env: &mut TEnv<R>, params: Self::Params) -> Result<Self::Typed> {
        self.iter().map(|x| x.infer(env, params.clone())).collect()
    }
}
