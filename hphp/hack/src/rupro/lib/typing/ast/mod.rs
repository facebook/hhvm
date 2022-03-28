// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub mod typing_block;
pub mod typing_expr;
pub mod typing_func_body;
pub mod typing_localize;
pub mod typing_param;
pub mod typing_stmt;
pub mod typing_tparam;
pub mod typing_trait;

pub use typing_trait::TC;
