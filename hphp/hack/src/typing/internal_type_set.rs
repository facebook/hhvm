// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::InternalType;

use typing_collections_rust::Set;

pub type ITySet<'a> = Set<'a, InternalType<'a>>;
