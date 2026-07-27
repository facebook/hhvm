/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::io;
use std::io::Write;
use std::process::Child;
use std::process::Command;
use std::process::Stdio;

use anyhow::Result;

pub struct PastryReporter {
    child: Child,
}

impl PastryReporter {
    /// Creates a printer that writes to reporter
    pub fn new(hostname: Option<String>) -> Result<Self> {
        let hostname = hostname.as_deref().unwrap_or("<unknown>");
        let child = Command::new("pastry")
            .args(["-t", &format!("watchman rage from {}", hostname)])
            .stdin(Stdio::piped())
            .stdout(Stdio::inherit())
            .spawn()?;

        Ok(Self { child })
    }

    fn out(&mut self) -> &mut dyn Write {
        self.child
            .stdin
            .as_mut()
            .expect("unable to acquire handle to reporter process")
    }

    pub fn wait(mut self) {
        // wait until reporter finishes, and leave terminal nice and clean.
        self.child.wait().ok();
    }
}

impl Write for PastryReporter {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        self.out().write(buf)
    }

    fn flush(&mut self) -> io::Result<()> {
        self.out().flush()
    }
}

pub use PastryReporter as FbReporter;
