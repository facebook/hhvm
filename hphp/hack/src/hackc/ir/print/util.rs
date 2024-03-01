// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Print utility functions.

use std::cell::Cell;
use std::fmt::Display;
use std::fmt::Formatter;
use std::fmt::Result;

use ir_core::instr::HasOperands;
use ir_core::Func;
use ir_core::InstrIdSet;
use ir_core::ValueId;

/// Display the iterator by calling the helper to display each value separated
/// by commas.
pub struct FmtSep<'a, T, I, F>(&'a str, &'a str, &'a str, Cell<Option<I>>, F)
where
    I: Iterator<Item = T>,
    F: Fn(&mut Formatter<'_>, T) -> Result;

impl<'a, T, I, F> FmtSep<'a, T, I, F>
where
    I: Iterator<Item = T>,
    F: Fn(&mut Formatter<'_>, T) -> Result,
{
    /// Create a new FmtSep. Note that the difference between:
    ///
    ///   format!("({})", FmtSep::new("", ", ", "", ...))
    ///
    /// and
    ///
    ///   format!("{}", FmtSep::new("(", ", ", ")", ...))
    ///
    /// is that in the first example the parentheses are always printed and in
    /// the second they're only printed if the iter is non-empty.
    pub fn new(
        prefix: &'a str,
        infix: &'a str,
        postfix: &'a str,
        iter: impl IntoIterator<Item = T, IntoIter = I>,
        f: F,
    ) -> Self {
        let iter2 = iter.into_iter();
        Self(prefix, infix, postfix, Cell::new(Some(iter2)), f)
    }

    pub fn comma(iter: impl IntoIterator<Item = T, IntoIter = I>, f: F) -> Self {
        let iter2 = iter.into_iter();
        Self("", ", ", "", Cell::new(Some(iter2)), f)
    }
}

impl<T, I, F> Display for FmtSep<'_, T, I, F>
where
    I: Iterator<Item = T>,
    F: Fn(&mut Formatter<'_>, T) -> Result,
{
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtSep(prefix, infix, postfix, iter, callback) = self;
        let mut iter = iter.take().unwrap();
        if let Some(e) = iter.next() {
            f.write_str(prefix)?;
            callback(f, e)?;
            for e in iter {
                f.write_str(infix)?;
                callback(f, e)?;
            }
            f.write_str(postfix)?;
        }
        Ok(())
    }
}

/// If self.0 is Some(_) then call F with the non-None value.
pub struct FmtOption<T, F>(pub Option<T>, pub F)
where
    F: Fn(&mut Formatter<'_>, &T) -> Result;

impl<T, F: Fn(&mut Formatter<'_>, &T) -> Result> Display for FmtOption<T, F> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        if let Some(value) = self.0.as_ref() {
            self.1(f, value)
        } else {
            Ok(())
        }
    }
}

/// If self.0 is Some(_) then call F1 with the non-None value otherwise call F2.
pub struct FmtOptionOr<T, F1, F2>(pub Option<T>, pub F1, pub F2)
where
    F1: Fn(&mut Formatter<'_>, &T) -> Result,
    F2: Fn(&mut Formatter<'_>) -> Result;

impl<T, F1, F2> Display for FmtOptionOr<T, F1, F2>
where
    F1: Fn(&mut Formatter<'_>, &T) -> Result,
    F2: Fn(&mut Formatter<'_>) -> Result,
{
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        if let Some(value) = self.0.as_ref() {
            self.1(f, value)
        } else {
            self.2(f)
        }
    }
}

/// Display a ByteVec as an escaped quoted string.
pub struct FmtEscapedString<'a>(pub &'a [u8]);

impl Display for FmtEscapedString<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let mut s = String::with_capacity(self.0.len() * 2 + 2);
        s.push('"');
        for &c in self.0 {
            match c {
                b'\\' => s.push_str("\\\\"),
                b'\n' => s.push_str("\\n"),
                b'\r' => s.push_str("\\r"),
                b'\t' => s.push_str("\\t"),
                b'\"' => s.push_str("\\\""),
                c if c.is_ascii_graphic() || c == b' ' => s.push(c as char),
                c => s.push_str(&format!("\\x{:02x}", c)),
            }
        }
        s.push('"');
        f.write_str(&s)
    }
}

/// Return a set of live InstrIds, used to decide which printed instructions
/// need to print their SSA variable names.
pub(crate) fn compute_live_instrs(func: &Func, verbose: bool) -> InstrIdSet {
    if verbose {
        return func.body_iids().collect();
    }

    func.body_instrs()
        .flat_map(|instr| instr.operands().iter().copied())
        .filter_map(ValueId::instr)
        .collect()
}
