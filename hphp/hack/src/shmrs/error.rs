// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// A system error.
#[derive(Debug, Copy, Clone)]
pub struct Errno {
    pub errno: i64,
}

impl Errno {
    #[cfg(not(target_os = "macos"))]
    fn errno() -> libc::c_int {
        // Safety: Reads the errno global.
        //
        // Note that we don't call strerror as it is not thread-safe.
        unsafe { *libc::__errno_location() }
    }

    #[cfg(target_os = "macos")]
    fn errno() -> libc::c_int {
        // Safety: Reads the errno global.
        //
        // Note that we don't call strerror as it is not thread-safe.
        unsafe { *libc::__error() }
    }


    /// Get the global errno info by reading in errno and calling strerror.
    pub fn get_global() -> Self {
        Self {
            errno: Self::errno() as i64,
        }
    }

    /// Check that the given errno number is zero, otherwise, return
    /// an `Errno` as a result.
    pub fn from(errno: libc::c_int) -> Result<(), Self> {
        if errno == 0 {
            Ok(())
        } else {
            Err(Errno {
                errno: errno as i64,
            })
        }
    }

    /// Read and return the global errno value if the given condition holds.
    pub fn if_(cond: bool) -> Result<(), Self> {
        if !cond {
            Ok(())
        } else {
            Err(Self::get_global())
        }
    }
}
