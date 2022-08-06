// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io::Result;
use std::io::Write;

use crate::BytesFormatter;
use crate::DisplayBytes;
use crate::FmtSpec;

pub fn write_bytes_fmt(w: &mut dyn Write, args: Arguments<'_>) -> Result<()> {
    let (a, b) = args.literals.split_at(args.args.len());

    for (literal, arg) in a.iter().zip(args.args.iter()) {
        if !literal.is_empty() {
            w.write_all(literal)?;
        }
        arg.value.fmt(&mut BytesFormatter(w, &arg.spec))?;
    }

    if !b.is_empty() {
        w.write_all(b.first().unwrap())?;
    }
    Ok(())
}

pub struct Arguments<'a> {
    literals: &'a [&'a [u8]],
    args: &'a [Argument<'a>],
}

impl<'a> Arguments<'a> {
    pub fn new(literals: &'a [&'a [u8]], args: &'a [Argument<'a>]) -> Self {
        Arguments { literals, args }
    }
}

pub struct Argument<'a> {
    value: &'a dyn DisplayBytes,
    spec: FmtSpec,
}

impl<'a> Argument<'a> {
    pub fn new(value: &'a dyn DisplayBytes, spec: FmtSpec) -> Self {
        Argument { value, spec }
    }
}
