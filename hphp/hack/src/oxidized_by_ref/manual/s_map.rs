// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub type SMap<'a, T> = arena_collections::map::Map<'a, &'a str, T>;
