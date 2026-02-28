// Copyright (c) Meta Platforms, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod fmt_plain;
mod fmt_raw;
mod user_diagnostic;

use std::io::Write;

pub use fmt_plain::*;
pub use fmt_raw::*;
pub use oxidized::diagnostics::*;
pub use relative_path::RelativePathCtx;
pub use user_diagnostic::*;

/// Render this list of diagnostics using the requested error_format.
/// If `is_term` is true, ansi colors and styles will be used according to the format.
/// Absolute paths are formed (at the discretion of the error format) from diagnostic
/// positions using the provided RelativePathCtx.
#[allow(clippy::todo)]
pub fn print_diagnostic_list<'e, I>(
    mut w: impl Write,
    is_term: bool,
    roots: &RelativePathCtx,
    error_format: Format,
    diagnostics: I,
) -> std::io::Result<()>
where
    I: IntoIterator<Item = &'e Diagnostic>,
{
    match error_format {
        Format::Plain => {
            for diag in diagnostics {
                writeln!(w, "{}", FmtPlain(diag, roots))?;
            }
        }
        Format::Raw => {
            for diag in diagnostics {
                writeln!(w, "{}", FmtRaw(diag, roots, is_term))?;
            }
        }
        bad => todo!("{bad}"),
    }
    Ok(())
}
