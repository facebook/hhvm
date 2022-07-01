// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod formatters;
pub mod print;
pub mod util;

pub use formatters::{FmtBid, FmtInstr, FmtLid, FmtLoc, FmtRawBid, FmtRawVid, FmtVid};
pub use print::print_unit;
pub use util::{FmtCommaSep, FmtEscapedString, FmtOption, FmtOptionOr};

// This isn't used by the print crate but is useful for code that wants to print
// a Func for debugging purposes.
pub struct DisplayFunc<'a, 'b>(
    pub &'b core::Func<'a>,
    /* verbose */ pub bool,
    pub &'b core::string_intern::StringInterner<'a>,
);

impl std::fmt::Display for DisplayFunc<'_, '_> {
    fn fmt(&self, w: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let DisplayFunc(func, verbose, strings) = *self;
        writeln!(w, "func {} {{", formatters::FmtFuncParams(func, strings))?;
        print::print_func_body(w, func, verbose, strings)?;
        writeln!(w, "}}")?;
        writeln!(w)
    }
}
