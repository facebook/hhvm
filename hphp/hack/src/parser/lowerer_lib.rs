// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#[macro_use]
extern crate lazy_static;

mod lowerer;
mod lowerer_modifier;
mod scour_comment;

pub use lowerer::{lower, Env};
pub use scour_comment::ScourComment;
