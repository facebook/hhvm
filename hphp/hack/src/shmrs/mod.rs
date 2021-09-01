// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(maybe_uninit_ref)]
#![feature(allocator_api)]

/// A Rust library for interacting with shared memory.
pub mod error;
pub mod filealloc;
pub mod sync;
