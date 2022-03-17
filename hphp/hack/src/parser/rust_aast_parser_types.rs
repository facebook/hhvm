// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use file_info::Mode;
use lint_rust::LintError;
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use oxidized::{
    aast::Program, errors::Error, file_info, parser_options::ParserOptions, pos::Pos,
    scoured_comments::ScouredComments,
};
use parser_core_types::syntax_error::SyntaxError;

#[derive(Clone, Debug, FromOcamlRep, ToOcamlRep, Default)]
pub struct Env {
    pub codegen: bool,
    pub php5_compat_mode: bool,
    pub elaborate_namespaces: bool,
    pub include_line_comments: bool,
    pub keep_errors: bool,
    pub quick_mode: bool,
    pub show_all_errors: bool,
    pub fail_open: bool,
    pub is_systemlib: bool,
    pub parser_options: ParserOptions,
}

#[derive(Clone, Debug, FromOcamlRep, ToOcamlRep)]
pub struct ParserResult {
    pub file_mode: Mode,
    pub scoured_comments: ScouredComments,
    pub aast: Result<Program<(), ()>, String>,
    pub lowpri_errors: Vec<(Pos, String)>,
    pub syntax_errors: Vec<SyntaxError>,
    pub errors: Vec<Error>,
    pub lint_errors: Vec<LintError>,
    pub parse_peak: i64,
    pub lower_peak: i64,
    pub error_peak: i64,
    pub arena_bytes: i64,
}
