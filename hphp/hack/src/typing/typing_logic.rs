// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bumpalo::collections::Vec;

use typing_defs_rust::{InternalType, Ty};

pub enum SubtypeProp<'a> {
    Coerce(Ty<'a>, Ty<'a>),
    IsSubtype(InternalType<'a>, InternalType<'a>),
    Conj(Vec<'a, SubtypeProp<'a>>),
    Disj(Vec<'a, SubtypeProp<'a>>),
}
