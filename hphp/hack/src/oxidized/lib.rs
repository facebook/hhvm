// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[macro_use]
extern crate rust_to_ocaml_attr;

mod manual;

pub use manual::aast_defs_impl;
pub use manual::aast_impl;
pub use manual::ast;
pub use manual::ast_defs_impl;
pub use manual::doc_comment;
pub use manual::file_info_impl;
pub use manual::file_pos;
pub use manual::file_pos_large;
pub use manual::file_pos_small;
pub use manual::global_options_impl;
pub use manual::i_map;
pub use manual::i_set;
pub use manual::ident;
pub use manual::internal_type_set;
pub use manual::lazy;
pub use manual::local_id;
pub use manual::local_id_map::LocalIdMap;
pub use manual::namespace_env_impl;
pub use manual::phase_map;
pub use manual::pos;
pub use manual::pos_span_raw;
pub use manual::pos_span_tiny;
pub use manual::relative_path;
pub use manual::s_map;
pub use manual::s_set;
pub use manual::scoured_comments_impl;
pub use manual::shape_map;
pub use manual::symbol_name;
pub use manual::t_shape_map;
pub use manual::tany_sentinel;
pub use manual::typing_defs_flags;
pub use manual::typing_logic;
pub use manual::typing_set;

mod stubs;

pub use stubs::opaque_digest;

mod impl_gen;
pub use impl_gen::*;

pub mod aast_visitor;

mod gen;

pub use gen::aast;
pub use gen::aast_defs;
pub use gen::ast_defs;
pub use gen::decl_defs;
pub use gen::decl_parser_options;
pub use gen::decl_reference;
pub use gen::error_codes;
pub use gen::errors;
pub use gen::file_info;
pub use gen::full_fidelity_parser_env;
pub use gen::global_options;
pub use gen::message;
pub use gen::namespace_env;
pub use gen::naming_types;
pub use gen::nast;
pub use gen::parser_options;
pub use gen::pos_or_decl;
pub use gen::prim_defs;
pub use gen::quickfix;
pub use gen::scoured_comments;
pub use gen::tast;
pub use gen::type_parameter_env;
pub use gen::typechecker_options;
pub use gen::typing_defs;
pub use gen::typing_defs_core;
pub use gen::typing_inference_env;
pub use gen::typing_kinding_defs;
pub use gen::typing_reason;
pub use gen::typing_tyvar_occurrences;
pub use gen::user_error;
pub use gen::xhp_attribute;
