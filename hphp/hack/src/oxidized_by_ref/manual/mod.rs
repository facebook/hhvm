// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub use crate::gen::aast_defs as aast;

pub mod decl_counters;
pub mod decl_env;
pub mod decl_provider;
pub mod direct_decl_parser;
pub mod i_map;
pub mod i_set;
pub mod ident;
pub mod lazy;
pub mod local_id;
pub mod local_id_map;
pub mod method_flags;
pub mod opaque_digest;
pub mod pos;
pub mod prop_flags;
pub mod relative_path;
pub mod s_map;
pub mod s_set;
pub mod shape_map;
pub mod symbol_name;
pub mod t_shape_map;
pub mod tany_sentinel;
pub mod typing_defs_flags;

mod ast_defs_impl;
mod shallow_decl_defs_impl;
mod typing_defs_core_impl;
mod typing_reason_impl;
