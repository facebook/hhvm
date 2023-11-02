// Copyright (c) Meta Platforms, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod fmt_plain;
mod fmt_raw;
mod user_error;

use std::io::Write;

pub use fmt_plain::*;
pub use fmt_raw::*;
pub use oxidized::errors::*;
pub use relative_path::RelativePathCtx;
pub use user_error::*;

/// Render this list of errors using the requested error_format.
/// If `is_term` is true, ansi colors and styles will be used according to the format.
/// Absolute paths are formed (at the discretion of the error format) from error
/// positions using the provided RelativePathCtx.
#[allow(clippy::todo)]
pub fn print_error_list<'e, I>(
    mut w: impl Write,
    is_term: bool,
    roots: &RelativePathCtx,
    error_format: Format,
    errs: I,
) -> std::io::Result<()>
where
    I: IntoIterator<Item = &'e Error>,
{
    match error_format {
        Format::Plain => {
            for err in errs {
                writeln!(w, "{}", FmtPlain(err, roots))?;
            }
        }
        Format::Raw => {
            for err in errs {
                writeln!(w, "{}", FmtRaw(err, roots, is_term))?;
            }
        }
        bad => todo!("{bad}"),
    }
    Ok(())
}
