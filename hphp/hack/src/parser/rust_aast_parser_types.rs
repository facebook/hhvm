// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::time::Duration;

use file_info::Mode;
use hash::HashSet;
use lint_rust::LintError;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use oxidized::aast::Program;
use oxidized::diagnostics::Diagnostic;
use oxidized::experimental_features::FeatureName;
use oxidized::file_info;
use oxidized::namespace_env::Mode as NamespaceMode;
use oxidized::parser_options::ParserOptions;
use oxidized::pos::Pos;
use oxidized::scoured_comments::ScouredComments;
use parser_core_types::syntax_error::SyntaxError;

#[derive(Clone, Debug, FromOcamlRep, ToOcamlRep)]
pub struct Env {
    pub mode: NamespaceMode,
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

impl Default for Env {
    fn default() -> Env {
        Env {
            mode: NamespaceMode::ForTypecheck,
            php5_compat_mode: false,
            elaborate_namespaces: false,
            include_line_comments: false,
            quick_mode: false,
            show_all_errors: false,
            is_systemlib: false,
            for_debugger_eval: false,
            parser_options: ParserOptions::default(),
            scour_comments: false,
        }
    }
}

#[derive(Debug, ToOcamlRep)]
pub struct ParserResult {
    pub file_mode: Mode,
    pub scoured_comments: ScouredComments,
    pub aast: Program<(), ()>,
    pub lowerer_parsing_errors: Vec<(Pos, String)>,
    pub syntax_errors: Vec<SyntaxError>,
    pub errors: Vec<Diagnostic>,
    pub lint_errors: Vec<LintError>,
    #[ocamlrep(skip)]
    pub profile: ParserProfile,
    #[ocamlrep(skip)]
    pub active_experimental_features: HashSet<FeatureName>,
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
