// Copyright (c) Meta Platforms, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::message::Message;
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
