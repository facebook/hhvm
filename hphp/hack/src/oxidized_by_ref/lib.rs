// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[macro_use]
extern crate rust_to_ocaml_attr;

mod manual;

pub use manual::aast;
pub use manual::ast;
pub use manual::blame_set;
pub use manual::decl_counters;
pub use manual::decl_env;
pub use manual::decl_provider;
pub use manual::direct_decl_parser;
pub use manual::i_map;
pub use manual::i_set;
pub use manual::ident;
pub use manual::internal_type_set;
pub use manual::lazy;
pub use manual::local_id;
pub use manual::local_id_map::LocalIdMap;
pub use manual::method_flags;
pub use manual::opaque_digest;
pub use manual::phase_map;
pub use manual::pos;
pub use manual::prop_flags;
pub use manual::relative_path;
pub use manual::s_map;
pub use manual::s_set;
pub use manual::shallow_decl_defs_impl;
pub use manual::shape_map;
pub use manual::symbol_name;
pub use manual::t_shape_map;
pub use manual::tany_sentinel;
pub use manual::typing_continuations;
pub use manual::typing_defs_flags;
pub use manual::typing_env;
pub use manual::typing_logic;
pub use manual::typing_set;

pub mod decl_visitor;

mod gen;

pub use gen::aast_defs;
pub use gen::ast_defs;
pub use gen::decl_defs;
pub use gen::decl_reference;
pub use gen::error_codes;
pub use gen::errors;
pub use gen::file_info;
pub use gen::global_options;
pub use gen::message;
pub use gen::namespace_env;
pub use gen::naming_types;
pub use gen::nast;
pub use gen::pos_or_decl;
pub use gen::prim_defs;
pub use gen::quickfix;
pub use gen::scoured_comments;
pub use gen::shallow_decl_defs;
pub use gen::tast;
pub use gen::type_parameter_env;
pub use gen::typechecker_options;
pub use gen::typing_cont_key;
pub use gen::typing_defs;
pub use gen::typing_defs_core;
pub use gen::typing_env_return_info;
pub use gen::typing_env_types;
pub use gen::typing_fake_members;
pub use gen::typing_inference_env;
pub use gen::typing_kinding_defs;
pub use gen::typing_local_types;
pub use gen::typing_modules;
pub use gen::typing_per_cont_env;
pub use gen::typing_reason;
pub use gen::typing_tyvar_occurrences;
pub use gen::user_error;
pub use gen::xhp_attribute;
