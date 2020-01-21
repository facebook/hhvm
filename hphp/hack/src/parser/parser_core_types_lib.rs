// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! # parser_core_types: Data type definitions for the Hack parser
//!
//! This crate contains data definitions and commonly-used types
//! used within and outside of the Hack parser. This library is separated
//! from the parser proper for two reasons:
//!
//!  1. The Rust compiler is notoriously slow and splitting up "cold" or
//!     infrequently changed data type definitions from the parser code
//!     speeds up the build.
//!  2. Separating the data definitions from the code makes it a little
//!     easier to reason about the structure of the parser.

pub mod indexed_source_text;
pub mod lexable_token;
pub mod lexable_trivia;
pub mod minimal_syntax;
pub mod minimal_token;
pub mod minimal_trivia;
pub mod parser_env;
pub mod positioned_syntax;
pub mod positioned_token;
pub mod positioned_trivia;
pub mod source_text;
pub mod syntax;
pub mod syntax_error;
mod syntax_generated;
pub mod syntax_kind;
pub mod syntax_trait;
pub mod syntax_tree;
pub mod syntax_type;
pub mod token_kind;
pub mod trivia_kind;
