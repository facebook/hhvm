// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use label_rust as label;
use local_rust as local;
use options::Options;

use super::{iterator::Iter, GlobalState};

#[derive(Debug, Default)]
pub struct Emitter {
    /// Options are frozen/const after emitter is constructed
    opts: Options,
    /// State is also frozen and set after closure conversion
    state: GlobalState,
    /// systemlib is part of context, changed externally
    systemlib: bool,
    // the rest is being mutated during emittance
    label_gen: label::Gen,
    local_gen: local::Gen,
    iterator: Iter,
}

impl Emitter {
    pub fn new(opts: Options, state: GlobalState) -> Emitter {
        Emitter {
            opts,
            state,
            ..Default::default()
        }
    }

    pub fn options(&self) -> &Options {
        &self.opts
    }

    /// Destruct the emitter but salvage its options (for use in emitting fatal program).
    pub fn into_options(self) -> Options {
        self.opts
    }

    pub fn context(&self) -> &dyn Context {
        self
    }
    pub fn context_mut(&mut self) -> &mut dyn Context {
        self
    }

    pub fn iterator_mut(&mut self) -> &mut Iter {
        &mut self.iterator
    }

    pub fn label_gen_mut(&mut self) -> &mut label::Gen {
        &mut self.label_gen
    }

    pub fn local_gen_mut(&mut self) -> &mut local::Gen {
        &mut self.local_gen
    }
}

/// Interface for changing the behavior, exposed to hh_single_compile
pub trait Context {
    fn set_systemlib(&mut self, flag: bool);
    fn systemlib(&self) -> bool;
}

impl Context for Emitter {
    fn set_systemlib(&mut self, flag: bool) {
        self.systemlib = flag;
    }
    fn systemlib(&self) -> bool {
        self.systemlib
    }
}
