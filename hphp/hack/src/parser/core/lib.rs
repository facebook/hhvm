// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod lexer;
pub use operator;
pub use operator::*;
pub mod parser;

pub use smart_constructors;
pub use smart_constructors::*;

pub mod parser_trait;

pub mod declaration_parser;
pub mod expression_parser;
pub mod pattern_parser;
pub mod statement_parser;
pub mod type_parser;

// The "parser_core_types" crate contains data definitions for a variety of data structures
// and types used by the parser and consumers of the parser. These data types were recently
// split out from this crate. In order to keep the same library facade, the relevant modules
// are re-exported here so that consumers do not need to be made aware of the data-code split.
pub use parser_core_types::compact_token;
pub use parser_core_types::compact_trivia;
pub use parser_core_types::indexed_source_text;
pub use parser_core_types::lexable_token;
pub use parser_core_types::lexable_trivia;
pub use parser_core_types::minimal_trivia;
pub use parser_core_types::parser_env;
pub use parser_core_types::positioned_syntax;
pub use parser_core_types::positioned_token;
pub use parser_core_types::positioned_trivia;
pub use parser_core_types::source_text;
pub use parser_core_types::syntax;
pub use parser_core_types::syntax_by_ref;
pub use parser_core_types::syntax_error;
pub use parser_core_types::syntax_kind;
pub use parser_core_types::syntax_trait;
pub use parser_core_types::token_factory;
pub use parser_core_types::token_kind;
pub use parser_core_types::trivia_factory;
pub use parser_core_types::trivia_kind;
