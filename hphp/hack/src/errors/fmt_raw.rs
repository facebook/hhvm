// Copyright (c) Meta Platforms, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ansi_term::Color;
use ansi_term::Style;
use oxidized::message::Message;
use oxidized::pos_or_decl::PosOrDecl;
use oxidized::user_error::UserError;
use rc_pos::Pos;
use relative_path::RelativePathCtx;

use crate::FmtErrorCode;

pub struct FmtRaw<'a>(
    pub &'a UserError<Pos, PosOrDecl>,
    pub &'a RelativePathCtx,
    pub bool,
);

impl<'a> std::fmt::Display for FmtRaw<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let Self(
            UserError {
                code,
                claim: Message(pos, msg),
                reasons,
                custom_msgs,
                quickfixes: _,
                is_fixmed: _,
                flags: _,
            },
            ctx,
            is_term,
        ) = self;
        // TODO: filename should cwd-relative if terminal, otherwise absolute
        // in hh_single_type_check, Pos.filename is the original path from the
        // cli, not the absolute path, complicating the logic. Lets just use
        // canonical path for now.
        let filename = pos.filename().to_absolute(ctx);
        let (line, start, end) = pos.info_pos();
        write!(
            f,
            "{}:{}:{},{}: {} ({})",
            Styled(Color::Red.bold(), filename.display(), *is_term),
            Styled(Color::Yellow.normal(), line, *is_term),
            Styled(Color::Cyan.normal(), start, *is_term),
            Styled(Color::Cyan.normal(), end, *is_term),
            Styled(Color::Red.bold(), msg, *is_term),
            Styled(Color::Yellow.normal(), FmtErrorCode(*code), *is_term),
        )?;
        for Message(pos, msg) in reasons {
            let filename = pos.filename().to_absolute(ctx);
            let (line, start, end) = pos.info_pos();
            writeln!(f)?;
            write!(
                f,
                "  {}:{}:{},{}: {}",
                Styled(Color::Red.normal(), filename.display(), *is_term),
                Styled(Color::Yellow.normal(), line, *is_term),
                Styled(Color::Cyan.normal(), start, *is_term),
                Styled(Color::Cyan.normal(), end, *is_term),
                Styled(Color::Green.normal(), msg, *is_term),
            )?;
        }
        for msg in custom_msgs {
            writeln!(f)?;
            write!(f, " {}", Styled(Color::Green.normal(), msg, *is_term),)?;
        }
        Ok(())
    }
}

struct Styled<T>(Style, T, bool);

impl<T: std::fmt::Display> std::fmt::Display for Styled<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self(_, x, false) => x.fmt(f),
            Self(style, x, true) => style.paint(x.to_string()).fmt(f),
        }
    }
}
