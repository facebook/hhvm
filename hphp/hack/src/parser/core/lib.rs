// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod lexer;
pub use operator::{self, *};
pub mod parser;

#[macro_use]
mod smart_constructors_macros; // must be before users of providing macros (*_parser)
pub use smart_constructors::{self, *};

pub mod parser_trait;

pub mod declaration_parser;
pub mod expression_parser;
pub mod statement_parser;
pub mod type_parser;

// The "parser_core_types" crate contains data definitions for a variety of data structures
// and types used by the parser and consumers of the parser. These data types were recently
// split out from this crate. In order to keep the same library facade, the relevant modules
// are re-exported here so that consumers do not need to be made aware of the data-code split.
pub use parser_core_types::{
    indexed_source_text, lexable_token, lexable_trivia, minimal_syntax, minimal_token,
    minimal_trivia, parser_env, positioned_syntax, positioned_token, positioned_trivia,
    source_text, syntax, syntax_error, syntax_kind, syntax_trait, token_kind, trivia_kind,
};
