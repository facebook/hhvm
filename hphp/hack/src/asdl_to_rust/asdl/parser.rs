// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

include!(concat!(env!("ASDL_GEN_OUT_DIR"), "/", "asdl_y.rs"));
pub use asdl_y::*;
