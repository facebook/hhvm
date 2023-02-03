// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hash::HashSet;
use oxidized::aast_defs::Tparam;

#[derive(Clone)]
pub struct Context {
    tparams: HashSet<String>,
}

impl Context {
    pub fn extend_tparams<Ex, En>(&mut self, tps: &[Tparam<Ex, En>]) {
        tps.iter().for_each(|tparam| {
            self.tparams.insert(tparam.name.1.clone());
        })
    }
    pub fn reset_tparams(&mut self) {
        self.tparams.clear()
    }
    pub fn set_tparams<Ex, En>(&mut self, tps: &[Tparam<Ex, En>]) {
        self.reset_tparams();
        self.extend_tparams(tps);
    }
    pub fn tparams(&self) -> &HashSet<String> {
        &self.tparams
    }
}
