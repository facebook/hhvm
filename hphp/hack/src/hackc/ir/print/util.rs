// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Print utility functions.

use core::{instr::HasOperands, Func, InstrIdSet, ValueId};
use std::{
    cell::Cell,
    fmt::{Display, Formatter, Result},
};

/// Display the iterator by calling the helper to display each value separated
/// by commas.
pub struct FmtCommaSep<'a, T, I, F>(&'a str, &'a str, Cell<Option<I>>, F)
where
    I: Iterator<Item = T>,
    F: Fn(&mut Formatter<'_>, T) -> Result;

impl<'a, T, I, F> FmtCommaSep<'a, T, I, F>
where
    I: Iterator<Item = T>,
    F: Fn(&mut Formatter<'_>, T) -> Result,
{
    pub fn new(iter: impl IntoIterator<Item = T, IntoIter = I>, f: F) -> Self {
        let iter2 = iter.into_iter();
        Self("", "", Cell::new(Some(iter2)), f)
    }

    pub fn new_with_wrapper(
        prefix: &'a str,
        postfix: &'a str,
        iter: impl IntoIterator<Item = T, IntoIter = I>,
        f: F,
    ) -> Self {
        let iter2 = iter.into_iter();
        Self(prefix, postfix, Cell::new(Some(iter2)), f)
    }
}

impl<T, I, F> Display for FmtCommaSep<'_, T, I, F>
where
    I: Iterator<Item = T>,
    F: Fn(&mut Formatter<'_>, T) -> Result,
{
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtCommaSep(prefix, postfix, iter, callback) = self;
        let mut iter = iter.take().unwrap();
        if let Some(e) = iter.next() {
            f.write_str(prefix)?;
            callback(f, e)?;
            for e in iter {
                f.write_str(", ")?;
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
        let mut s = String::with_capacity(self.0.len() * 3 / 2 + 2);
        s.push('"');
        for &c in self.0 {
            match c {
                b'\\' => s.push('\\'),
                b'\n' => s.push_str("\\n"),
                b'\r' => s.push_str("\\r"),
                b'\t' => s.push_str("\\t"),
                c => {
                    if c.is_ascii_graphic() || c == b' ' {
                        s.push(c as char)
                    } else {
                        s.push_str(&format!("\\{:03o}", c))
                    }
                }
            }
        }
        s.push('"');
        f.write_str(&s)
    }
}

/// Return a set of live InstrIds, used to decide which printed instructions
/// need to print their SSA variable names.
pub(crate) fn compute_live_instrs(func: &Func<'_>, verbose: bool) -> InstrIdSet {
    if verbose {
        return func.body_iids().collect();
    }

    func.body_instrs()
        .flat_map(|instr| instr.operands().iter().copied())
        .filter_map(ValueId::instr)
        .collect()
}
