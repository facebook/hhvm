// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub(crate) use rc_pos as pos;

mod lint;
pub use lint::*;
use oxidized::tast;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[derive(ocamlrep::FromOcamlRep, ocamlrep::ToOcamlRep)]
#[repr(isize)]
pub enum LintCode {
    LowercaseConstant = 5001,
    UseCollectionLiteral,
    StaticString,
    ShapeIdxRequiredField = 5005,
}

pub type LintError = LintsCore<pos::Pos>;

impl LintError {
    pub fn lowercase_constant(p: pos::Pos, cst: &str) -> Self {
        let lower = cst.to_ascii_lowercase();
        let message = format!("Please use `{}` instead of `{}`", lower, cst);
        Self {
            code: LintCode::LowercaseConstant as isize,
            severity: Severity::LintWarning,
            pos: p,
            message,
            bypass_changed_lines: false,
            autofix: None,
            check_status: None::<tast::CheckStatus>,
        }
    }
}
