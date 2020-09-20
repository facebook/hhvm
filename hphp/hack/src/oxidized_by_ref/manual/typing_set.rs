// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_defs::Ty;

pub type TySet<'a> = arena_collections::set::Set<'a, Ty<'a>>;
