// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::time::Duration;

use file_info::Mode;
use lint_rust::LintError;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use oxidized::aast::Program;
use oxidized::errors::Error;
use oxidized::file_info;
use oxidized::parser_options::ParserOptions;
use oxidized::pos::Pos;
use oxidized::scoured_comments::ScouredComments;
use parser_core_types::syntax_error::SyntaxError;

#[derive(Clone, Debug, FromOcamlRep, ToOcamlRep, Default)]
pub struct Env {
    pub codegen: bool,
    pub php5_compat_mode: bool,
    pub elaborate_namespaces: bool,
    pub include_line_comments: bool,
    pub quick_mode: bool,
    pub show_all_errors: bool,
    pub is_systemlib: bool,
    pub for_debugger_eval: bool,
    pub parser_options: ParserOptions,
    pub scour_comments: bool,
}

#[derive(Debug, ToOcamlRep)]
pub struct ParserResult {
    pub file_mode: Mode,
    pub scoured_comments: ScouredComments,
    pub aast: Program<(), ()>,
    pub lowerer_parsing_errors: Vec<(Pos, String)>,
    pub syntax_errors: Vec<SyntaxError>,
    pub errors: Vec<Error>,
    pub lint_errors: Vec<LintError>,
    #[ocamlrep(skip)]
    pub profile: ParserProfile,
}

#[derive(Debug, Default)]
pub struct ParserProfile {
    pub parse_peak: u64,
    pub lower_peak: u64,
    pub error_peak: u64,

    pub arena_bytes: u64,

    pub parsing_t: Duration,
    pub lowering_t: Duration,
    pub elaboration_t: Duration,
    pub error_t: Duration,
    pub total_t: Duration,
}

impl ParserProfile {
    pub fn fold(self, b: Self) -> Self {
        Self {
            parse_peak: std::cmp::max(self.parse_peak, b.parse_peak),
            lower_peak: std::cmp::max(self.lower_peak, b.lower_peak),
            error_peak: std::cmp::max(self.error_peak, b.error_peak),

            arena_bytes: self.arena_bytes + b.arena_bytes,

            parsing_t: self.parsing_t + b.parsing_t,
            lowering_t: self.lowering_t + b.lowering_t,
            elaboration_t: self.elaboration_t + b.elaboration_t,
            error_t: self.error_t + b.error_t,
            total_t: self.total_t + b.total_t,
        }
    }
}
