// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub type LocalId = (isize, String);

pub fn get_name((_, name): &LocalId) -> &String {
    name
}
