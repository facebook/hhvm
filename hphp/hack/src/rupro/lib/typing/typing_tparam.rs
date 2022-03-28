// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::reason::Reason;
use crate::tast;
use crate::typing::typing_env::TEnv;
use crate::typing::typing_error::Result;
use crate::typing::typing_trait::TC;

impl<R: Reason> TC<R> for oxidized::aast::Tparam<(), ()> {
    type Params = ();
    type Typed = tast::Tparam<R>;

    fn infer(&self, env: &TEnv<R>, params: ()) -> Result<Self::Typed> {
        assert!(self.user_attributes.is_empty(), "unimplemented");
        let parameters = self.parameters.infer(env, params)?;
        let tp = tast::Tparam {
            variance: self.variance.clone(),
            name: self.name.clone(),
            parameters,
            constraints: self.constraints.clone(),
            reified: self.reified.clone(),
            user_attributes: vec![],
        };
        Ok(tp)
    }
}
