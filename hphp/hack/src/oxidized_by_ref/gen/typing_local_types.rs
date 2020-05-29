// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ca15e09ebb528fadf3cbe1ef02e604eb>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

pub type ExpressionId<'a> = ident::Ident;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Local<'a>(pub Ty<'a>, pub ExpressionId<'a>);
impl<'a> TrivialDrop for Local<'a> {}

pub type TypingLocalTypes<'a> = local_id::map::Map<'a, Local<'a>>;
