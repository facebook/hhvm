// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(box_patterns)]
#[macro_use]
extern crate lazy_static;

mod desugar_expression_tree;
mod lowerer;
mod modifier;
mod scour_comment;

pub use lowerer::lower;
pub use lowerer::Env;
pub use scour_comment::ScourComment;
