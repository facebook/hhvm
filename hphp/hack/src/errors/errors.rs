// Copyright (c) Meta Platforms, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(is_terminal)]

mod fmt_plain;
mod fmt_raw;
mod user_error;

use std::io::IsTerminal;

pub use fmt_plain::*;
pub use fmt_raw::*;
pub use oxidized::errors::*;
pub use relative_path::RelativePathCtx;
pub use user_error::*;

pub fn print_error_list<'e, I>(roots: &RelativePathCtx, error_format: Format, errs: I)
where
    I: IntoIterator<Item = &'e Error>,
{
    match error_format {
        Format::Plain => {
            for err in errs {
                println!("{}", FmtPlain(err, roots));
            }
        }
        Format::Raw => {
            let is_term = std::io::stdout().is_terminal();
            for err in errs {
                println!("{}", FmtRaw(err, roots, is_term));
            }
        }
        bad => todo!("{bad}"),
    }
}
