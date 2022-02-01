// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::{HashMap, HashSet};
use hcons::Hc;

pub type Symbol = Hc<str>;
pub type SymbolMap<V> = HashMap<Symbol, V>;
pub type SymbolSet = HashSet<Symbol>;
