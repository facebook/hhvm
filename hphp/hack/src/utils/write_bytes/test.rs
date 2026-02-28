// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use crate::format_bytes;
use crate::write_bytes;

type Result = std::io::Result<()>;

#[test]
fn test_basic() -> Result {
    let mut v: Vec<u8> = Vec::new();
    write_bytes!(&mut v, b"abc")?;
    assert_eq!(v, b"abc");

    assert_eq!(format_bytes!(b"abc"), b"abc");
    Ok(())
}

#[test]
fn test_pos() -> Result {
    let mut v = Vec::new();
    write_bytes!(&mut v, b"abc{}def", 5)?;
    assert_eq!(v, b"abc5def");

    assert_eq!(format_bytes!(b"abc{}def", 5), b"abc5def");
    Ok(())
}

#[test]
fn test_named() -> Result {
    let mut v = Vec::new();
    write_bytes!(&mut v, b"abc{foo}def", foo = 5)?;
    assert_eq!(v, b"abc5def");

    assert_eq!(format_bytes!(b"abc{foo}def", foo = 5), b"abc5def");
    Ok(())
}

#[test]
fn test_types() -> Result {
    assert_eq!(format_bytes!(b"{}", 5), b"5");
    assert_eq!(format_bytes!(b"{}", "abc"), b"abc");
    assert_eq!(format_bytes!(b"{}", Cow::from("abc")), b"abc");
    Ok(())
}
