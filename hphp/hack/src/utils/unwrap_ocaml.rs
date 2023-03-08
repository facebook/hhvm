// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern "C" {
    fn caml_failwith(msg: *const libc::c_char);
}

/// This trait is to provide .unwrap_ocaml() on a Result<T,E>.
/// Its behavior upon Err is to caml_failwith, i.e. exactly the same
/// behavior as ocaml's "failwith" primitive. The exception string
/// includes a debug-print of E which, for anyhow-backed errors,
/// includes a stack trace.
/// It must only be called from within an ocaml FFI, and indeed can only
/// be linked when inside an ocaml binary.
pub trait UnwrapOcaml<T> {
    fn unwrap_ocaml(self) -> T;
}

impl<T, E: std::fmt::Debug + std::fmt::Display> UnwrapOcaml<T> for Result<T, E> {
    fn unwrap_ocaml(self) -> T {
        match self {
            Ok(t) => t,
            Err(e) => {
                eprintln!("Gathering error callstacks... [{e}]");
                // It can honestly take 45s to fetch the backtrace ":?" in an ocaml binary. Crazy.
                let msg = format!("{e:?}");
                let msg = std::ffi::CString::new(msg).expect("null byte in error");
                // Safety: through the unenforced convention that we're only ever invoked by ocaml ffi
                unsafe { caml_failwith(msg.into_raw()) };
                unreachable!()
            }
        }
    }
}
