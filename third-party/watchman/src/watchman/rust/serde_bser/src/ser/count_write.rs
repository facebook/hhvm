/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::io;
use std::io::Write;

/// A writer that counts how many bytes were written.
pub struct CountWrite {
    count: usize,
}

impl CountWrite {
    pub fn new() -> Self {
        CountWrite { count: 0 }
    }

    #[inline]
    pub fn count(&self) -> usize {
        self.count
    }
}

impl Write for CountWrite {
    #[inline]
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        self.count += buf.len();
        Ok(buf.len())
    }

    #[inline]
    fn flush(&mut self) -> io::Result<()> {
        Ok(())
    }
}
