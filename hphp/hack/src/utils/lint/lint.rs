// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::OcamlRep;
use oxidized::pos::Pos;

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
#[repr(usize)]
pub enum LintCode {
    LowercaseConstant = 5001,
    UseCollectionLiteral,
    StaticString,
    ShapeIdxRequiredField = 5005,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum Severity {
    Error,
    Warning,
    Advice,
}

// port from lint_core.t
#[derive(Clone, Debug, OcamlRep)]
pub struct LintError {
    code: usize,
    severity: Severity,
    pos: Pos,
    message: String,

    /// Normally, lint warnings and lint advice only get shown by arcanist if the
    /// lines they are raised on overlap with lines changed in a diff. This
    /// flag bypasses that behavior
    bypass_changed_lines: bool,
    autofix: (String, String),
}

impl LintError {
    pub fn lowercase_constant(p: Pos, cst: &str) -> Self {
        let lower = cst.to_ascii_lowercase();
        let message = format!(
            "Please use '{}' instead of '{}'",
            lower.to_string(),
            cst.to_string()
        );
        Self {
            code: LintCode::LowercaseConstant as usize,
            severity: Severity::Warning,
            pos: p,
            message,
            bypass_changed_lines: false,
            autofix: ("".into(), "".into()),
        }
    }
}
