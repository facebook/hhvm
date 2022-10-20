// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub trait FilePos {
    fn offset(&self) -> usize;
    fn line_column_beg(&self) -> (usize, usize, usize);
}
