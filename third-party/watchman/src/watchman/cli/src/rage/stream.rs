/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::io;
use std::io::IsTerminal;
use std::io::Stdout;
use std::io::Write;

#[cfg(feature = "fb")]
pub use super::facebook::FbReporter;

#[cfg(not(feature = "fb"))]
mod fallback {
    use std::io;
    use std::io::Write;

    use anyhow::anyhow;
    use anyhow::Result;

    pub struct FbReporter {}

    impl FbReporter {
        pub fn new(hostname: Option<String>) -> Result<Self> {
            Err(anyhow!("unimplemented"))
        }

        pub fn wait(mut self) {
            unimplemented!()
        }
    }

    impl Write for FbReporter {
        fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
            unimplemented!()
        }

        fn flush(&mut self) -> io::Result<()> {
            unimplemented!()
        }
    }
}

#[cfg(not(feature = "fb"))]
pub use self::fallback::*;

pub enum Stream {
    Child(FbReporter),
    Stdout(Stdout),
}

impl Stream {
    /// Creates a printer that writes to stdout
    fn new_stdout() -> Self {
        Self::Stdout(std::io::stdout())
    }

    pub fn new(hostname: Option<String>) -> Self {
        if !std::io::stdout().is_terminal() {
            Self::new_stdout()
        } else if let Ok(reporter) = FbReporter::new(hostname) {
            Self::Child(reporter)
        } else {
            Self::new_stdout()
        }
    }

    #[allow(dead_code)]
    pub fn is_redirected(&self) -> bool {
        if let Self::Child(_) = self {
            true
        } else {
            !std::io::stdout().is_terminal()
        }
    }

    fn out(&mut self) -> &mut dyn Write {
        match self {
            Self::Child(c) => c,
            Self::Stdout(out) => out,
        }
    }

    pub fn wait(self) {
        if let Self::Child(child) = self {
            // wait until reporter finishes, and leave terminal nice and clean.
            child.wait();
        }
    }
}

impl Write for Stream {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        self.out().write(buf)
    }

    fn flush(&mut self) -> io::Result<()> {
        self.out().flush()
    }
}
