// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::utils::core::Ident;
use serde::{Deserialize, Serialize};

#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Tyvar(Ident);

impl From<Ident> for Tyvar {
    fn from(x: Ident) -> Self {
        Self(x)
    }
}

impl From<Tyvar> for Ident {
    fn from(x: Tyvar) -> Self {
        x.0
    }
}

impl From<Tyvar> for isize {
    fn from(x: Tyvar) -> Self {
        x.0.into()
    }
}
