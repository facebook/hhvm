// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bitflags::bitflags;
use hash::HashSet;
use oxidized::aast_defs::Tparam;

#[derive(Clone)]
pub struct Context {
    tparams: HashSet<String>,
    mode: file_info::Mode,
    in_class: bool,
    flags: Flags,
}

bitflags! {
    #[derive(Default)]
    pub struct Flags: u8 {
        const SOFT_AS_LIKE= 1 << 0;
    }
}

impl Default for Context {
    fn default() -> Self {
        Context {
            tparams: HashSet::default(),
            mode: file_info::Mode::Mstrict,
            in_class: false,
            flags: Flags::empty(),
        }
    }
}

impl Context {
    pub fn new(flags: Flags) -> Self {
        Context {
            flags,
            ..Default::default()
        }
    }

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

    pub fn set_mode(&mut self, mode: file_info::Mode) {
        self.mode = mode
    }
    pub fn mode(&self) -> &file_info::Mode {
        &self.mode
    }

    pub fn set_in_class(&mut self, in_class: bool) {
        self.in_class = in_class;
    }

    pub fn in_class(&self) -> bool {
        self.in_class
    }

    pub fn soft_as_like(&self) -> bool {
        self.flags.contains(Flags::SOFT_AS_LIKE)
    }
}
