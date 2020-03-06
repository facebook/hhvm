// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use typing_defs_rust as typing_defs;

pub struct TypingEnvReturnInfo<'a> {
    pub type_: typing_defs::PossiblyEnforcedTy<'a>,
    pub disposable: bool,
    pub mutable: bool,
    pub explicit: bool,
    pub void_to_rx: bool,
}
