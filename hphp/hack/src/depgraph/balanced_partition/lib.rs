// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! This crate implements the balanced graph partitioning algorithm described at
//! https://www.kdd.org/kdd2016/papers/files/rpp0883-dhulipalaAemb.pdf
//!
//! At a high level, it permutes an array of Doc to be more compressible by
//! swapping pairs of docs across the left and right halves of the array to reduce
//! a cost function, essentially clustering similar docs in each half. Then it
//! recurses on both halves to do the same thing there.

mod balance;
mod config;
mod timers;

pub use balance::Doc;
pub use balance::ExternalId;
pub use balance::optimize_doc_order;
pub use config::BalanceConfig;
