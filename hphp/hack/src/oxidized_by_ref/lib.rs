// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[macro_use]
extern crate rust_to_ocaml_attr;

mod manual;

pub use manual::aast;
pub use manual::decl_counters;
pub use manual::decl_env;
pub use manual::direct_decl_parser;
pub use manual::i_map;
pub use manual::i_set;
pub use manual::ident;
pub use manual::lazy;
pub use manual::local_id;
pub use manual::method_flags;
pub use manual::opaque_digest;
pub use manual::pos;
pub use manual::prop_flags;
pub use manual::relative_path;
pub use manual::s_map;
pub use manual::s_set;
pub use manual::shape_map;
pub use manual::symbol_name;
pub use manual::t_shape_map;
pub use manual::tany_sentinel;
pub use manual::typing_defs_flags;

pub mod decl_visitor;

mod gen;

pub use gen::aast_defs;
pub use gen::ast_defs;
pub use gen::custom_error;
pub use gen::custom_error_config;
pub use gen::decl_defs;
pub use gen::decl_reference;
pub use gen::error_message;
pub use gen::file_info;
pub use gen::namespace_env;
pub use gen::naming_types;
pub use gen::nast;
pub use gen::package;
pub use gen::package_info;
pub use gen::patt_binding_ty;
pub use gen::patt_error;
pub use gen::patt_locl_ty;
pub use gen::patt_name;
pub use gen::patt_string;
pub use gen::patt_var;
pub use gen::pos_or_decl;
pub use gen::prim_defs;
pub use gen::saved_state_rollouts;
pub use gen::scoured_comments;
pub use gen::shallow_decl_defs;
pub use gen::typing_defs;
pub use gen::typing_defs_core;
pub use gen::typing_reason;
pub use gen::validation_err;
pub use gen::xhp_attribute;
