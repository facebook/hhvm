// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::reason::Reason;

#[derive(Clone, Debug)]
pub enum Primary<R: Reason> {
    InvalidTypeHint(R::P),
    ExpectingTypeHint(R::P),
    ExpectingReturnTypeHint(R::P),
}
