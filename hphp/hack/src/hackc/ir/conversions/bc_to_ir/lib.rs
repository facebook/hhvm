// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(box_patterns)]

mod class;
mod context;
mod convert;
mod func;
mod instrs;
mod sequence;
mod types;

pub use convert::bc_to_ir;
