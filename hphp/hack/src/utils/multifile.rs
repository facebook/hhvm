// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Multifiles fake having multiple files in one file. This is intended for use
//! only in unit test files.
//!
//! Example features that require multiple files in tests include the `newtype`
//! feature.

use std::borrow::Cow;
use std::ffi::OsStr;
use std::path::Path;
use std::path::PathBuf;
use std::str;

use anyhow::anyhow;
use lazy_static::lazy_static;
use regex::bytes::Regex;

lazy_static! {
    // Matches a line denoting a single file within the multifile, and captures
    // the filename.
    // For example, the following line (ending with a newline)
    //
    // `//// foo.php`
    //
    // will result in a match, with "foo.php" captured.
    // Note: `(?x)` ignores whitespace in the regex string.
    static ref MULTIFILE_DELIM: Regex = Regex::new(r#"(?x)
        (?m) # allow matching on multiple lines
        ^////\s*
        (.+\S) # capture filename up to last non-whitespace character
        \s*\n"#).unwrap();
    // Matches a line of the format
    //
    // `// @directory some/path/here`
    //
    // optionally containing a @file tag followed by some file name; e.g.,
    //
    // `// @directory some/path/here @file foo.php`
    static ref DIRECTORY: Regex =
        Regex::new(r#"^// @directory \s*(\S+)\s*(?:@file\s*(\S+))?\n"#).unwrap();
}

/// Processes multifile `content` into a list of files. This function expects `content` in one of the following formats:
/// 1. A series of files.
/// 2. A single file with a specific directory.
///
/// # "series of files" format:
/// `content` represents multiple files. Each file starts with a line in the format `//// filename.php`, followed by content.
///
/// //// foo.php
///
/// function foo(): void {}
/// //// bar.php
///
/// class Bar {}
///
/// # "specific dir" format:
/// `content` represents a single file in a specific directory, denoted by
///
/// `// @directory some/dir/here`
///
/// You can optionally include a `@file` tag followed by a file name that would
/// be appended to the given directory.
///
/// See the tests for examples.
pub fn to_files<'p>(
    path: &'p Path,
    content: impl AsRef<[u8]> + Into<Vec<u8>>,
) -> anyhow::Result<Vec<(Cow<'p, Path>, Vec<u8>)>> {
    let content = content.as_ref();
    // A multifile always starts with 4 forward slashes.
    if MULTIFILE_DELIM.is_match(content) {
        let filename_and_content = split_multifile(&content)?;
        Ok(filename_and_content
            .into_iter()
            .map(|(filename, content)| {
                let filename = str::from_utf8(filename)?;
                let filename = PathBuf::from(format!("{}--{}", path.display(), filename));
                Ok((filename.into(), content))
            })
            .collect::<anyhow::Result<Vec<_>>>()?)
    } else if let Some(captures) = DIRECTORY.captures(content) {
        let dir = str::from_utf8(&captures[1])?;
        let file = captures.get(2).map_or_else(
            || strip_root(path).as_os_str(),
            |m| OsStr::new(str::from_utf8(m.as_bytes()).unwrap()),
        );
        let mut newpath = PathBuf::from(dir);
        newpath.push(file);
        let mut content: Vec<u8> = DIRECTORY.replace(content, &b""[..]).into();
        if let Some(b'\n') = content.last() {
            content.pop();
        }
        Ok(vec![(newpath.into(), content)])
    } else {
        Ok(vec![(path.into(), content.into())])
    }
}

/// Given a multifile where files are prefixed either with "base-" or
/// "changed-", return the list of files prefixed with "base-", and the list of
/// files prefixed with "changed-". These prefixes are removed from the
/// resulting lists.
///
/// See the tests for an example multifile.
pub fn base_and_changed_files(
    content: impl AsRef<[u8]> + Into<Vec<u8>>,
) -> anyhow::Result<(Vec<(PathBuf, Vec<u8>)>, Vec<(PathBuf, Vec<u8>)>)> {
    let file_to_content = split_multifile(&content)?;
    let mut base_files = vec![];
    let mut changed_files = vec![];
    for (file, content) in file_to_content {
        let file = str::from_utf8(file)?;
        let file = Path::new(file);
        if let Some(base) = strip_file_name_prefix(file, "base-") {
            base_files.push((base, content));
        } else if let Some(changed) = strip_file_name_prefix(file, "changed-") {
            changed_files.push((changed, content));
        }
    }
    Ok((base_files, changed_files))
}

fn split_multifile<'t, T>(content: &'t T) -> anyhow::Result<Vec<(&'t [u8], Vec<u8>)>>
where
    T: AsRef<[u8]> + Into<Vec<u8>>,
{
    let filenames: Vec<_> = MULTIFILE_DELIM
        .captures_iter(content.as_ref())
        .map(|c| {
            c.get(1)
                .expect("delim regex should capture filename")
                .as_bytes()
        })
        .collect();
    let mut contents: Vec<Vec<u8>> = MULTIFILE_DELIM
        .split(content.as_ref())
        .skip(1) // skips the 0th entry (empty string)
        .map(|c| c.into())
        .collect();
    if contents.len() + 1 == filenames.len() {
        contents.push(vec![]);
    } else if contents.len() != filenames.len() {
        return Err(anyhow!(
            "filenames len {} and content len {} mismatch.",
            filenames.len(),
            contents.len()
        ));
    }
    Ok(filenames.into_iter().zip(contents.into_iter()).collect())
}

// If `file`'s file name component starts with `prefix`, return `file` with that prefix removed.
fn strip_file_name_prefix(file: impl AsRef<Path>, prefix: &str) -> Option<PathBuf> {
    let file_name = file.as_ref().file_name()?.to_str()?;
    file_name
        .strip_prefix(prefix)
        .map(|stripped_file_name| file.as_ref().with_file_name(stripped_file_name))
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
    use std::path::PathBuf;

    use pretty_assertions::assert_eq;

    use super::*;

    #[test]
    fn two_files() {
        let p = PathBuf::from("test.php");
        let c = b"//// file1.php
a
////  file2.php
b
";
        let r = to_files(&p, &c[..]).unwrap();
        assert_eq!(r.len(), 2);
        assert_eq!(r[0].0.as_ref(), &PathBuf::from("test.php--file1.php"));
        assert_eq!(r[1].0.as_ref(), &PathBuf::from("test.php--file2.php"));

        assert_eq!(&r[0].1, b"a\n");
        assert_eq!(&r[1].1, b"b\n");
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

        assert_eq!(&r[0].1, b"a");
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

        assert_eq!(&r[0].1, b"a");
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

    #[test]
    fn test_base_and_changed_files() {
        let c = b"//// base-foo.php
a
//// a/base-bar.php
b
//// changed-foo.php
c
";
        let (base_files, changed_files) = base_and_changed_files(*c).unwrap();
        assert_eq!(
            base_files,
            vec![
                (PathBuf::from("foo.php"), b"a\n".to_vec()),
                (PathBuf::from("a/bar.php"), b"b\n".to_vec()),
            ],
        );
        assert_eq!(
            changed_files,
            vec![(PathBuf::from("foo.php"), b"c\n".to_vec())]
        );
    }
}
