// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeSet;

use crate::warnings_saved_state::ErrorHash;

pub type ErrorHashSet = BTreeSet<ErrorHash>;
