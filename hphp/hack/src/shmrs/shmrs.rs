// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(maybe_uninit_array_assume_init)]
#![feature(allocator_api)]

/// A Rust library for interacting with shared memory.
pub mod chashmap;
pub mod error;
pub mod filealloc;
pub mod hash_builder;
pub mod hashmap;
pub mod segment;
pub mod shardalloc;
pub mod sync;
