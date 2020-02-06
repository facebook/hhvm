// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::{anyhow, Result};
use itertools::Either::*;
use lazy_static::lazy_static;
use regex::bytes::Regex;
use std::{
    ffi::OsStr,
    iter::Iterator,
    path::{Path, PathBuf},
    str,
};

// Content type in return type can be improved to
//     impl AsRef<[u8]> + Into<Vec<u8>> + 't
// instead of Vec<u8>. After Either implements,
//     impl<L: Into<T>, R: Into<T>> Into<T> for Either<L, R> { .. }
pub fn to_files<'p, 't>(
    path: &'p Path,
    content: impl AsRef<[u8]> + Into<Vec<u8>> + 't,
) -> Result<Vec<(impl AsRef<Path> + 'p, Vec<u8>)>> {
    lazy_static! {
        static ref DELIM: Regex = Regex::new(r#"(?m)^////\s*(.*)\n"#).unwrap();
        static ref DIRECTORY: Regex =
            Regex::new(r#"^// @directory \s*(\S+)\s*(?:@file\s*(\S+))?\n"#).unwrap();
    }
    let content = content.as_ref();
    let delims: Vec<&[u8]> = DELIM
        .captures_iter(content)
        .map(|c| c.get(1).unwrap().as_bytes())
        .collect();
    if content.starts_with(b"////") && delims.len() > 0 {
        let mut contents: Vec<Vec<u8>> = DELIM.split(content).skip(1).map(|c| c.into()).collect();
        if contents.len() + 1 == delims.len() {
            contents.push(vec![]);
        } else if contents.len() != delims.len() {
            return Err(
                anyhow! {"delims len {} and content len {} mismatch.", delims.len(), contents.len()},
            );
        }
        Ok(delims
            .iter()
            .map(|f| str::from_utf8(f))
            .collect::<std::result::Result<Vec<_>, _>>()?
            .into_iter()
            .map(|f| Right(PathBuf::from(format!("{}--{}", path.display(), f))))
            .zip(contents.into_iter())
            .collect())
    } else if let Some(captures) = DIRECTORY.captures(content) {
        let dir = str::from_utf8(&captures[1])?;
        let file = captures.get(2).map_or_else(
            || strip_root(path).as_os_str(),
            |m| OsStr::new(str::from_utf8(m.as_bytes()).unwrap()),
        );
        let mut newpath = PathBuf::from(dir);
        newpath.push(file);
        let content = DIRECTORY.replace(content, &b""[..]);
        Ok(vec![(Right(newpath), content.into())])
    } else {
        Ok(vec![(Left(path), content.into())])
    }
}

fn strip_root(p: &Path) -> &Path {
    if p.starts_with("/") {
        p.strip_prefix("/").unwrap()
    } else {
        p
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;
    use std::path::PathBuf;

    #[test]
    fn two_files() {
        let p = PathBuf::from("test.php");
        let c = b"//// file1.php
a
////  file2.php
b";
        let r = to_files(&p, &c[..]).unwrap();
        assert_eq!(r.len(), 2);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("test.php--file1.php"));
        assert_eq!(r[1].0.as_ref(), &PathBuf::from("test.php--file2.php"));

        assert_eq!(&r[0].1, b"a\n");
        assert_eq!(&r[1].1, b"b");
    }

    #[test]
    fn last_empty() {
        let p = PathBuf::from("test.php");
        let c = b"//// file1.php
a
////file2.php
";
        let r = to_files(&p, &c[..]).unwrap();
        assert_eq!(r.len(), 2);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("test.php--file1.php"));
        assert_eq!(r[1].0.as_ref(), &PathBuf::from("test.php--file2.php"));

        assert_eq!(&r[0].1, b"a\n");
        assert_eq!(&r[1].1, b"");
    }

    #[test]
    fn mid_empty() {
        let p = PathBuf::from("test.php");
        let c = b"//// file1.php
////  file2.php
a
";
        let r = to_files(&p, &c[..]).unwrap();
        assert_eq!(r.len(), 2);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("test.php--file1.php"));
        assert_eq!(r[1].0.as_ref(), &PathBuf::from("test.php--file2.php"));

        assert_eq!(&r[0].1, b"");
        assert_eq!(&r[1].1, b"a\n");
    }

    #[test]
    fn dir_only() {
        let p = PathBuf::from("test.php");
        let c = b"// @directory a/b/c
";
        let r = to_files(&p, &c[..]).unwrap();
        assert_eq!(r.len(), 1);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("a/b/c/test.php"));

        assert_eq!(&r[0].1, b"");
    }

    #[test]
    fn dir() {
        let p = PathBuf::from("test.php");
        let c = b"// @directory a/b/c
a
";
        let r = to_files(&p, &c[..]).unwrap();
        assert_eq!(r.len(), 1);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("a/b/c/test.php"));

        assert_eq!(&r[0].1, b"a\n");
    }

    #[test]
    fn dir_with_file() {
        let p = PathBuf::from("test.php");
        let c = b"// @directory a/b/c @file f.php
a
";
        let r = to_files(&p, &c[..]).unwrap();
        assert_eq!(r.len(), 1);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("a/b/c/f.php"));

        assert_eq!(&r[0].1, b"a\n");
    }

    #[test]
    fn normal_file() {
        let p = PathBuf::from("test.php");
        let c = b"a";
        let r = to_files(&p, &c[..]).unwrap();
        assert_eq!(r.len(), 1);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("test.php"));
        assert_eq!(&r[0].1, b"a");
    }
}
