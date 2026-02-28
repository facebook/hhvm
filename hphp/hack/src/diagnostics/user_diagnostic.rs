// Copyright (c) Meta Platforms, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::message::Message;
use oxidized::user_diagnostic::Severity;
use oxidized::user_diagnostic::UserDiagnostic;
use rc_pos::Pos;
use relative_path::RelativePathCtx;
use serde_derive::Deserialize;
use serde_derive::Serialize;
use serde_json::json;

use crate::ErrorCode;

#[derive(Debug, Deserialize, Clone, Serialize)]
#[serde(rename_all = "lowercase")]
enum LinterSeverity {
    Error,
    Warning,
    Advice,
    Disabled,
}

/// For lint infrastructure integration.
#[derive(Debug, Deserialize, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
struct LintMessage {
    path: String,
    line: Option<usize>,
    char: Option<usize>,
    code: String,
    severity: LinterSeverity,
    name: String,
    description: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    original: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    replacement: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    bypass_changed_line_filtering: Option<bool>,
}

pub fn to_json(e: &UserDiagnostic<Pos, Pos>, ctx: &RelativePathCtx) -> serde_json::Value {
    let mut messages = Vec::new();
    messages.push(msg_json(&e.claim, e.code, ctx));
    messages.extend(e.reasons.iter().map(|m| msg_json(m, e.code, ctx)));
    json!({
        "message": messages,
        "severity": match e.severity {
            Severity::Err => "error",
            Severity::Warning => "warning",
        },
    })
}

fn msg_json(
    Message(pos, descr): &Message<Pos>,
    code: ErrorCode,
    ctx: &RelativePathCtx,
) -> serde_json::Value {
    let (line, scol, ecol) = pos.info_pos();
    json!({
        "descr": descr.to_string(),
        "path": pos.filename().to_absolute(ctx),
        "line": line,
        "start": scol,
        "end": ecol,
        "code": code,
    })
}

fn to_lint_message(e: &UserDiagnostic<Pos, Pos>, ctx: &RelativePathCtx) -> LintMessage {
    let UserDiagnostic {
        severity,
        code,
        claim,
        reasons: _,
        explanation: _,
        quickfixes: _,
        custom_msgs: _,
        is_fixmed: _,
        function_pos: _,
    } = e;
    let Message(pos, claim_descr) = claim;
    let (stline, _edline, stcol, _edcol) = pos.info_pos_extended();
    LintMessage {
        path: pos
            .filename()
            .to_absolute(ctx)
            .into_os_string()
            .into_string()
            .unwrap(),
        line: Some(stline),
        char: Some(stcol),
        code: "HACKWARNING".to_string(),
        severity: match severity {
            Severity::Err => LinterSeverity::Error,
            Severity::Warning => LinterSeverity::Warning,
        },
        name: code.to_string(),
        description: Some(claim_descr.to_string()),
        original: None,
        replacement: None,
        bypass_changed_line_filtering: None,
    }
}

/// For lint infrastructure integration.
pub fn to_lint_json(e: &UserDiagnostic<Pos, Pos>, ctx: &RelativePathCtx) -> serde_json::Value {
    json!(to_lint_message(e, ctx))
}
