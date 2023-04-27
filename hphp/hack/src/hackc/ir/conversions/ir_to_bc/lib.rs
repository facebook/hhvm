// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod adata;
mod class;
mod constant;
mod convert;
mod emitter;
mod ex_frame;
mod func;
mod push_count;
mod pusher;
mod strings;
mod types;

pub use convert::ir_to_bc;
