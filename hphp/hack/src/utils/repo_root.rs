// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Keep in sync with //hphp/hack/src/utils/wwwroot.ml

use std::path::Path;
use std::path::PathBuf;

use thiserror::Error;

/// Maximum number of parent steps after checking the starting directory.
pub const TRAVERSAL_LIMIT: usize = 50;

/// Check the current dir and up to `TRAVERSAL_LIMIT` ancestors, returning the
/// first one containing an hh config file.
pub fn guess_root_from_current_dir() -> Result<PathBuf, GuessRootError> {
    let root = std::env::current_dir().map_err(GuessRootError::CurrentDir)?;
    guess_root_from(&root)
}

/// Check the starting dir and up to `TRAVERSAL_LIMIT` ancestors, returning the
/// first one containing an hh config file.
pub fn guess_root_from(start: &std::path::Path) -> Result<PathBuf, GuessRootError> {
    let mut possible_root = start;
    for _ in 0..=TRAVERSAL_LIMIT {
        if is_root(possible_root) {
            return Ok(possible_root.to_owned());
        }
        possible_root = possible_root
            .parent()
            .ok_or(GuessRootError::ParentNotFound)?;
    }
    Err(GuessRootError::TraversalLimitReached(TRAVERSAL_LIMIT))
}

pub fn is_root(possible_root: &Path) -> bool {
    let config_path = possible_root.join(hh_config::FILE_PATH_RELATIVE_TO_ROOT);
    config_path.exists()
}

#[derive(Debug, Error)]
pub enum GuessRootError {
    #[error("unable to get the current directory: {0}")]
    CurrentDir(#[source] std::io::Error),
    #[error("unable to find the root before traversal limit of {0}")]
    TraversalLimitReached(usize),
    #[error("no parent or parent couldn't be found")]
    ParentNotFound,
}

#[cfg(test)]
mod tests {
    use std::fs::create_dir_all;
    use std::path::Path;

    use pretty_assertions::assert_eq;
    use tempfile::TempDir;

    use super::*;

    // A simple, panicky touch
    fn touch(path: &Path) {
        // Guarantee the file exists, but don't truncate it if it does.
        std::fs::OpenOptions::new()
            .create(true)
            .truncate(false)
            .write(true)
            .open(path)
            .unwrap();
    }

    #[test]
    fn guess_root_finds_root() {
        let repo_parent = TempDir::with_prefix("repo_root_tests.").unwrap();
        let contains_hhconfig = repo_parent.path().join("www");
        let current_dir = contains_hhconfig.join("a/b/c");

        create_dir_all(&current_dir).unwrap();
        touch(&contains_hhconfig.join(".hhconfig"));

        let root = guess_root_from(&current_dir).unwrap();
        assert_eq!(root, contains_hhconfig);
    }

    #[test]
    fn guess_root_cant_find_root() {
        let repo_parent = TempDir::with_prefix("repo_root_tests.").unwrap();
        let current_dir = repo_parent.path().join("a/b/c");
        create_dir_all(&current_dir).unwrap();

        let root = guess_root_from(&current_dir);
        assert!(matches!(root, Err(GuessRootError::ParentNotFound)));
    }

    #[test]
    fn guess_root_finds_root_at_traversal_limit() {
        let repo_root = TempDir::with_prefix("repo_root_tests.").unwrap();
        touch(&repo_root.path().join(".hhconfig"));
        let mut current_dir = repo_root.path().to_owned();
        for _ in 0..50 {
            current_dir.push("dir");
        }
        create_dir_all(&current_dir).unwrap();

        assert_eq!(guess_root_from(&current_dir).unwrap(), repo_root.path());
        current_dir.pop();
        assert_eq!(guess_root_from(&current_dir).unwrap(), repo_root.path());
    }

    #[test]
    fn guess_root_traversal_limit() {
        let repo_parent = TempDir::with_prefix("repo_root_tests.").unwrap();
        touch(&repo_parent.path().join(".hhconfig"));
        let mut current_dir = repo_parent.path().to_owned();
        // The TRAVERSAL_LIMIT is 50. We could do TRAVERSAL_LIMIT + 1 here, but
        // maybe it's better to avoid a test accidentally creating a million
        // folders if we update TRAVERSAL_LIMIT in the future.
        for ii in 0..51 {
            current_dir.push(format!("dir{}", ii));
        }
        create_dir_all(&current_dir).unwrap();

        let root = guess_root_from(&current_dir);
        assert!(matches!(
            root,
            Err(GuessRootError::TraversalLimitReached(TRAVERSAL_LIMIT))
        ));
    }
}
