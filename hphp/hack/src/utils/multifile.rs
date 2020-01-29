// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::{anyhow, Result};
use itertools::Either::{self, *};
use lazy_static::lazy_static;
use regex::bytes::Regex;
use std::{
    ffi::OsStr,
    iter::Iterator,
    path::{Path, PathBuf},
    str,
};

pub fn to_files<'p, 't>(
    path: &'p Path,
    content: &'t [u8],
) -> Result<Vec<(impl AsRef<Path> + 'p, impl AsRef<[u8]> + 't)>> {
    lazy_static! {
        static ref DELIM: Regex = Regex::new(r#"(?m)^////\s*(.*)\n"#).unwrap();
        static ref DIRECTORY: Regex =
            Regex::new(r#"^// @directory \s*(\S+)\s*(?:@file\s*(\S+))?\n"#).unwrap();
    }
    let delims: Vec<&[u8]> = DELIM
        .captures_iter(content)
        .map(|c| c.get(1).unwrap().as_bytes())
        .collect();
    if delims.len() > 0 {
        let mut contents: Vec<Either<&[u8], _>> = DELIM.split(content).skip(1).map(Left).collect();
        if contents.len() + 1 == delims.len() {
            contents.push(Left(b""));
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
            || path.file_name().unwrap_or(OsStr::new("")),
            |m| OsStr::new(str::from_utf8(m.as_bytes()).unwrap()),
        );
        let mut newpath = PathBuf::from(dir);
        newpath.push(file);
        let content = Right(DIRECTORY.replace(content, &b""[..]));
        Ok(vec![(Right(newpath), content)])
    } else {
        Ok(vec![(Left(path), Left(content))])
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
        let c = b"
//// file1.php
a
////  file2.php
b";
        let r = to_files(&p, c).unwrap();
        assert_eq!(r.len(), 2);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("test.php--file1.php"));
        assert_eq!(r[1].0.as_ref(), &PathBuf::from("test.php--file2.php"));

        assert_eq!(r[0].1.as_ref(), b"a\n");
        assert_eq!(r[1].1.as_ref(), b"b");
    }

    #[test]
    fn last_empty() {
        let p = PathBuf::from("test.php");
        let c = b"
//// file1.php
a
////file2.php
";
        let r = to_files(&p, c).unwrap();
        assert_eq!(r.len(), 2);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("test.php--file1.php"));
        assert_eq!(r[1].0.as_ref(), &PathBuf::from("test.php--file2.php"));

        assert_eq!(r[0].1.as_ref(), b"a\n");
        assert_eq!(r[1].1.as_ref(), b"");
    }

    #[test]
    fn mid_empty() {
        let p = PathBuf::from("test.php");
        let c = b"
//// file1.php
////  file2.php
a
";
        let r = to_files(&p, c).unwrap();
        assert_eq!(r.len(), 2);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("test.php--file1.php"));
        assert_eq!(r[1].0.as_ref(), &PathBuf::from("test.php--file2.php"));

        assert_eq!(r[0].1.as_ref(), b"");
        assert_eq!(r[1].1.as_ref(), b"a\n");
    }

    #[test]
    fn dir_only() {
        let p = PathBuf::from("test.php");
        let c = b"// @directory a/b/c
";
        let r = to_files(&p, c).unwrap();
        assert_eq!(r.len(), 1);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("a/b/c/test.php"));

        assert_eq!(r[0].1.as_ref(), b"");
    }

    #[test]
    fn dir() {
        let p = PathBuf::from("test.php");
        let c = b"// @directory a/b/c
a
";
        let r = to_files(&p, c).unwrap();
        assert_eq!(r.len(), 1);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("a/b/c/test.php"));

        assert_eq!(r[0].1.as_ref(), b"a\n");
    }

    #[test]
    fn dir_with_file() {
        let p = PathBuf::from("test.php");
        let c = b"// @directory a/b/c @file f.php
a
";
        let r = to_files(&p, c).unwrap();
        assert_eq!(r.len(), 1);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("a/b/c/f.php"));

        assert_eq!(r[0].1.as_ref(), b"a\n");
    }

    #[test]
    fn normal_file() {
        let p = PathBuf::from("test.php");
        let c = b"a";
        let r = to_files(&p, c).unwrap();
        assert_eq!(r.len(), 1);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("test.php"));
        assert_eq!(r[0].1.as_ref(), b"a");
    }
}
