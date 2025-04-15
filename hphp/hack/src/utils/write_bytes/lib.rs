// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod arguments;
pub mod bytes_formatter;
pub mod display_bytes;

#[cfg(test)]
mod test;

#[cfg(test)]
extern crate self as write_bytes;

pub use arguments::Argument;
pub use arguments::Arguments;
pub use arguments::write_bytes_fmt;
pub use bytes_formatter::BytesFormatter;
pub use bytes_formatter::FmtSpec;
pub use display_bytes::DisplayBytes;
pub use write_bytes_macro::write_bytes;

#[macro_export]
macro_rules! format_bytes {
    ($($toks:tt)*) => (
        {
            let mut tmp = Vec::new();
            write_bytes!(&mut tmp, $($toks)*).unwrap();
            tmp
        }
    )
}

#[macro_export]
macro_rules! writeln_bytes {
    ($stream:expr) => {
        ($stream).write_all(b"\n")
    };
    ($stream:expr, $($toks:tt)*) => {{
        write_bytes!($stream, $($toks)*).and_then(|()| ($stream).write_all(b"\n"))
    }};
}
