// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;
use std::io::Result;
use std::io::Write;

use relative_path::RelativePath;

/// Indent is an abstraction of indentation. Configurable indentation
/// and perf tweaking will be easier.
#[derive(Clone)]
struct Indent(usize);

impl Indent {
    fn new() -> Self {
        Self(0)
    }

    fn inc(&mut self) {
        self.0 += 1;
    }

    fn dec(&mut self) {
        self.0 -= 1;
    }
}

impl fmt::Display for Indent {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for _ in 0..self.0 {
            f.write_str("  ")?;
        }
        Ok(())
    }
}

#[derive(Clone)]
pub struct Context<'a> {
    pub(crate) path: Option<&'a RelativePath>,
    indent: Indent,
}

impl<'a> Context<'a> {
    pub fn new(path: Option<&'a RelativePath>) -> Self {
        Self {
            path,
            indent: Indent::new(),
        }
    }

    /// Insert a newline with indentation
    pub(crate) fn newline(&self, w: &mut dyn Write) -> Result<()> {
        write!(w, "\n{}", self.indent)
    }

    /// Start a new indented block
    pub(crate) fn block<F>(&self, w: &mut dyn Write, f: F) -> Result<()>
    where
        F: FnOnce(&Self, &mut dyn Write) -> Result<()>,
    {
        let mut ctx = self.clone();
        ctx.indent.inc();
        f(&ctx, w)
    }

    pub(crate) fn unblock<F>(&self, w: &mut dyn Write, f: F) -> Result<()>
    where
        F: FnOnce(&Self, &mut dyn Write) -> Result<()>,
    {
        let mut ctx = self.clone();
        ctx.indent.dec();
        f(&ctx, w)
    }

    /// Printing instruction list requires manually control indentation,
    /// where indent_inc/indent_dec are called
    pub(crate) fn indent_inc(&mut self) {
        self.indent.inc();
    }

    pub(crate) fn indent_dec(&mut self) {
        self.indent.dec();
    }
}

pub(crate) struct FmtIndent<'a>(pub(crate) &'a Context<'a>);

impl fmt::Display for FmtIndent<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        self.0.indent.fmt(f)?;
        Ok(())
    }
}

write_bytes::display_bytes_using_display!(FmtIndent<'_>);
