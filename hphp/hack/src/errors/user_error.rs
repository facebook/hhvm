// Copyright (c) Meta Platforms, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::message::Message;
use oxidized::pos_or_decl::PosOrDecl;
use oxidized::user_error::UserError;
use rc_pos::Pos;
use relative_path::RelativePathCtx;
use serde_json::json;

use crate::ErrorCode;

pub fn to_json(e: &UserError<Pos, Pos>, ctx: &RelativePathCtx) -> serde_json::Value {
    let mut messages = Vec::new();
    messages.push(msg_json(&e.claim, e.code, ctx));
    messages.extend(e.reasons.iter().map(|m| msg_json(m, e.code, ctx)));
    json!({
        "message": messages,
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

/// A `std::fmt::Display` implementation that displays the error in the
/// Errors.Plain format produced by OCaml Errors.to_string.
pub struct FmtPlain<'a>(pub &'a UserError<Pos, PosOrDecl>, pub &'a RelativePathCtx);

impl<'a> std::fmt::Display for FmtPlain<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let Self(
            UserError {
                code,
                claim: Message(pos, msg),
                reasons,
                quickfixes: _,
                is_fixmed: _,
            },
            ctx,
        ) = self;
        let code = FmtErrorCode(*code);
        write!(f, "{}\n{} ({})", pos.absolute(ctx), msg, code)?;
        for Message(pos, msg) in reasons {
            write!(f, "\n  {}\n  {}", pos.absolute(ctx), msg)?;
        }
        Ok(())
    }
}

struct FmtErrorCode(ErrorCode);

impl std::fmt::Display for FmtErrorCode {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let Self(code) = self;
        write!(f, "{}[{:04}]", error_kind(*code), code)
    }
}

fn error_kind(error_code: ErrorCode) -> &'static str {
    match error_code / 1000 {
        1 => "Parsing",
        2 => "Naming",
        3 => "NastCheck",
        4 => "Typing",
        5 => "Lint",
        8 => "Init",
        _ => "Other",
    }
}
