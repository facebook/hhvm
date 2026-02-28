// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod file_pos;
pub mod file_pos_large;
pub mod file_pos_small;
pub mod pos_span_raw;
pub mod pos_span_tiny;
pub mod with_erased_lines;

mod pos_impl;
pub use pos_impl::Pos;
pub use pos_impl::PosR;
pub use pos_impl::PosString;
pub use pos_impl::map;
