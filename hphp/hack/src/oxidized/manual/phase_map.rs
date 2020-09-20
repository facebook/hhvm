// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::errors::Phase;

pub type PhaseMap<T> = std::collections::BTreeMap<Phase, T>;
