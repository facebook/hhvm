// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cell::RefCell;

use im::HashMap;
use ty::local::ParamMode;
use ty::local::Ty;
use ty::reason::Reason;
use utils::core::LocalId;

use crate::typing::env::typing_return_info::TypingReturnInfo;

/// A subenvironment containing "global" (i.e. on the scope of toplevel
/// definitions) configuration.
#[derive(Debug)]
pub struct TGEnv<R: Reason> {
    pub return_: RefCell<TypingReturnInfo<R>>,
    pub params: RefCell<HashMap<LocalId, (Ty<R>, R::Pos, ParamMode)>>,
}

impl<R: Reason> TGEnv<R> {
    /// Initialize a new global environment.
    ///
    /// A "global" (i.e. relating to the scope of a toplevel definition)
    /// contains some non-changing configuration.
    pub fn new() -> Self {
        Self {
            return_: RefCell::new(TypingReturnInfo::placeholder()),
            params: RefCell::new(HashMap::new()),
        }
    }
}
