// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;
use std::path::PathBuf;

/// This function is like std::path::Path::strip_prefix,
/// namely it returns a result such that base.join(result) is the same file as 'path',
/// and returns an error if there's no such result.
/// * However, this function operates on files on disk: base.join(result) *identifies the same file* as path.
///   Meanwhile, strip_prefix operates on strings: it's spec is that its base.join(result) is the same *string* as path.
/// * The first way it operates on files is that it works relative to the current working directory (CWD).
///   e.g. if cwd is "/home/ljw/www/flib" and root is "/home/ljw/www"
///   then given "a.php" this function will return "flib/a.php"
///   and given "../a.php" this function will return "a.php"
/// * The second way it operates on files is that it works up to std::fs::canonicalize of base.
///   e.g. if root is "/data/users/ljw/www-hg" and "/home/ljw/www" is a symlink to root, then
///   then given "/home/ljw/www/flib/a.php" this function will return "flib/a.php"
///   It does this by using std::fs::canonicalize.
///   For efficiency, it assumes that base has already been canonicalized (so as to avoid doing a file-system lookup on it).
/// * This function works for files that don't exist.
///   You wouldn't be surprised that std::path::Path::strip_prefix does too, since it operates only on strings.
///   But this function uses std::fs::canonicalize, which normally works only on files that exist.
///   This function nevertheless succeeds, by merely requiring that base exists, and not requiring
///   that the result exist too.
pub fn canonicalized_strip_prefix_relative_to_cwd(
    path: &Path,
    canonicalized_base: &Path,
) -> anyhow::Result<PathBuf> {
    // This function solves for canonicalization of non-existent files
    // by walking up the hierarchy until it finds one that does exist. For example,
    // 1. canonicalize_candidate "/home/users/ljw/www/d/e/a.php", trailing None
    // 2. canonicalize_candidate "/home/users/ljw/www/d/e", trailing Some("a.php")
    // 3. canonicalize_candidate "/home/users/ljw/www/d", trailing Some("e/a.php")
    // It walks up the hierarchy of canonicalize_candidate until it finds one that exists.
    // For instance it might find that step 3 does exist, and resolves to "/data/users/ljw/www-hg/d"
    // which matches root "/data/users/ljw/www-hg". Therefore, the result from this function
    // will be "d/e/a.php" where "d" came from canonicalize and strip_prefix, and "e/a.php" came
    // from walking up the hiearchy until we found something that could canonicalize.
    // Therefore the final answer is "d/e/a.php", i.e. the suffix after stripping plus whatever was trailing.

    let mut trailing = None;
    let mut canonicalize_candidate = PathBuf::from(path);
    // If path is relative of the form "ax.php" then we can't canonicalize it as is, and it has no parent.
    // Let's normalize it to "./ax.php" so we can at least canonicalize the parent.
    if let Some(std::path::Component::Normal(_)) = path.components().next() {
        canonicalize_candidate = PathBuf::from(".").join(path);
    }
    loop {
        match std::fs::canonicalize(&canonicalize_candidate) {
            Ok(canonicalized) => match canonicalized.strip_prefix(canonicalized_base) {
                Ok(suffix) => return Ok(path_join_opt(suffix, trailing)),
                Err(_) => anyhow::bail!(
                    "Path \"{}\" (realpath {}) doesn't start with base {}",
                    path.display(),
                    canonicalized.display(),
                    canonicalized_base.display()
                ),
            },
            Err(_) => match (
                canonicalize_candidate.parent(),
                canonicalize_candidate.file_name(),
            ) {
                (Some(parent), Some(file)) => {
                    trailing = Some(path_join_opt(file.as_ref(), trailing));
                    canonicalize_candidate = PathBuf::from(parent);
                    continue;
                }
                _ => anyhow::bail!("Can't realpath \"{}\"", path.display(),),
            },
        }
    }
}

/// Like path.join, except is a no-op if suffix is None.
fn path_join_opt(path: &Path, suffix: Option<PathBuf>) -> PathBuf {
    match suffix {
        None => path.to_owned(),
        Some(suffix) => path.join(suffix),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_strip_prefix() -> anyhow::Result<()> {
        let tmpdir = tempfile::TempDir::new()?;
        let tmp = PathBuf::from(tmpdir.path());
        let empty = PathBuf::new();

        // within $tmp, we will create the following directory structure
        // $tmp/www-hg            <- this will be the project root
        // $tmp/www-hg/.hhconfig
        // $tmp/www-hg/a.php
        // $tmp/www-hg/d/b.php
        // $tmp/www               <- a symlink to www-hg
        // $tmp/t                 <- a symlink to $tmp
        // $tmp/c.php             <- a file not part of any root
        // $tmp/hhi
        let root = tmp.join("www-hg");
        let link_root = tmp.join("www");
        std::fs::create_dir(&root)?;
        let root = std::fs::canonicalize(&root)?;
        std::os::unix::fs::symlink(&root, &link_root)?;
        std::fs::write(root.join(".hhconfig"), "")?;
        std::fs::write(root.join("a.php"), "<?hh")?;
        std::fs::create_dir(&root.join("d"))?;
        std::fs::write(root.join("d").join("b.php"), "<?hh")?;
        std::os::unix::fs::symlink(&tmp, &tmp.join("t"))?;
        std::fs::create_dir(&root.join("hhi"))?;

        // a helper - given specified "base/suffix", extracts the root-relative path as a string
        let relpath = |base: &Path, suffix: &str| -> anyhow::Result<String> {
            Ok(
                canonicalized_strip_prefix_relative_to_cwd(&base.join(suffix), &root)?
                    .to_str()
                    .unwrap()
                    .to_owned(),
            )
        };

        // files in the repo that exist
        assert_eq!(&relpath(&root, "a.php")?, "a.php");
        assert_eq!(&relpath(&root, "d/b.php")?, "d/b.php");
        assert_eq!(&relpath(&link_root, "a.php")?, "a.php");
        assert_eq!(&relpath(&link_root, "d/b.php")?, "d/b.php");

        // files in the repo that don't exist
        assert_eq!(&relpath(&root, "ax.php")?, "ax.php");
        assert_eq!(&relpath(&root, "d/bx.php")?, "d/bx.php");
        assert_eq!(&relpath(&link_root, "ax.php")?, "ax.php");
        assert_eq!(&relpath(&link_root, "d/bx.php")?, "d/bx.php");

        // files outside the repo
        assert!(relpath(&tmp, "c.php").is_err());
        assert!(relpath(&tmp, "cx.php").is_err());
        assert!(relpath(&tmp, "t/c.php").is_err());
        assert!(relpath(&tmp, "t/cx.php").is_err());

        // relative to CWD
        std::env::set_current_dir(&root)?;
        assert_eq!(&relpath(&empty, "a.php")?, "a.php");
        assert_eq!(&relpath(&empty, "d/b.php")?, "d/b.php");
        assert_eq!(&relpath(&empty, "ax.php")?, "ax.php");
        assert_eq!(&relpath(&empty, "d/bx.php")?, "d/bx.php");
        assert_eq!(&relpath(&empty, "./a.php")?, "a.php");
        assert_eq!(&relpath(&empty, "./d/b.php")?, "d/b.php");
        assert_eq!(&relpath(&empty, "./ax.php")?, "ax.php");
        assert_eq!(&relpath(&empty, "./d/bx.php")?, "d/bx.php");
        std::env::set_current_dir(&root.join("d"))?;
        assert_eq!(&relpath(&empty, "b.php")?, "d/b.php");
        assert_eq!(&relpath(&empty, "bx.php")?, "d/bx.php");
        assert_eq!(&relpath(&empty, "./b.php")?, "d/b.php");
        assert_eq!(&relpath(&empty, "./bx.php")?, "d/bx.php");
        assert_eq!(&relpath(&empty, "../a.php")?, "a.php");
        assert_eq!(&relpath(&empty, "../ax.php")?, "ax.php");
        assert!(&relpath(&empty, "../../x.php").is_err());

        // relative to CWD, a symlink
        std::env::set_current_dir(&link_root)?;
        assert_eq!(&relpath(&empty, "a.php")?, "a.php");
        assert_eq!(&relpath(&empty, "d/b.php")?, "d/b.php");
        assert_eq!(&relpath(&empty, "ax.php")?, "ax.php");
        assert_eq!(&relpath(&empty, "d/bx.php")?, "d/bx.php");
        assert_eq!(&relpath(&empty, "./a.php")?, "a.php");
        assert_eq!(&relpath(&empty, "./d/b.php")?, "d/b.php");
        assert_eq!(&relpath(&empty, "./ax.php")?, "ax.php");
        assert_eq!(&relpath(&empty, "./d/bx.php")?, "d/bx.php");
        std::env::set_current_dir(&link_root.join("d"))?;
        assert_eq!(&relpath(&empty, "b.php")?, "d/b.php");
        assert_eq!(&relpath(&empty, "bx.php")?, "d/bx.php");
        assert_eq!(&relpath(&empty, "./b.php")?, "d/b.php");
        assert_eq!(&relpath(&empty, "./bx.php")?, "d/bx.php");
        assert_eq!(&relpath(&empty, "../a.php")?, "a.php");
        assert_eq!(&relpath(&empty, "../ax.php")?, "ax.php");
        assert!(&relpath(&empty, "../../x.php").is_err());

        Ok(())
    }
}
