// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io::{Result, Write};

// Once we support fmtspec this is where the parameters (like width) will live.
pub struct FmtSpec {}

pub struct BytesFormatter<'a>(pub(crate) &'a mut dyn Write, pub(crate) &'a FmtSpec);

impl Write for BytesFormatter<'_> {
    fn write(&mut self, buf: &[u8]) -> Result<usize> {
        self.0.write(buf)
    }

    fn flush(&mut self) -> Result<()> {
        self.0.flush()
    }

    fn write_vectored(&mut self, bufs: &[std::io::IoSlice<'_>]) -> Result<usize> {
        self.0.write_vectored(bufs)
    }
    fn write_all(&mut self, buf: &[u8]) -> Result<()> {
        self.0.write_all(buf)
    }
    fn write_fmt(&mut self, fmt: std::fmt::Arguments<'_>) -> Result<()> {
        self.0.write_fmt(fmt)
    }
}
