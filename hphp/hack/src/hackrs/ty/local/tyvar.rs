// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use pos::ToOxidized;
use serde::Deserialize;
use serde::Serialize;
use utils::core::Ident;

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

impl<'a> ToOxidized<'a> for Tyvar {
    type Output = isize;

    fn to_oxidized(&self, _bump: &'a bumpalo::Bump) -> Self::Output {
        self.0.into()
    }
}
