// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use files_to_ignore::FilesToIgnore;
use relative_path::RelativePath;

const EXTENSIONS: [&str; 8] = [
    "php",         // normal php file
    "phpt",        // our php template or test files
    "hack",        // strict-mode files (Hack)
    "hackpartial", // partial-mode files (Hack)
    "hck",         // open source hack: bikeshed entry
    "hh",          // open source hack: bikeshed entry
    "hhi",         // interface files only visible to the type checker
    "xhp",         // XHP extensions
];

// There's an interesting difference in behavior with findUtils.ml with paths
// like "/my/path/foo.php/.". With findUtils.ml, that path would be ignored.
// With find_utils.rs, we would consider this file, because it's effectively
// "/my/path/foo.php".
fn is_dot_file_or_dir(path: &Path) -> bool {
    match path.file_name() {
        Some(base_name) => match base_name.to_str() {
            Some(base_name_str) => base_name_str.starts_with('.'),
            None => false,
        },
        None => false,
    }
}

pub fn is_hack(path: &Path) -> bool {
    !(is_dot_file_or_dir(path))
        && match path.extension() {
            Some(ext) => EXTENSIONS.iter().any(|x| std::ffi::OsStr::new(x) == ext),
            None => false,
        }
}

/// Returns true if path is a hack file, is not under any of (.hg, .git, .svn),
/// and is not a .phpt file under phpunit/.
pub fn is_non_ignored_hack(files_to_ignore: &FilesToIgnore, path: &Path) -> bool {
    is_hack(path) && !files_to_ignore.should_ignore(path)
}

/// Traverse the directory `root` and yield the non-ignored Hack files under it
/// in parallel using `jwalk`. The root will be stripped from each path and
/// replaced with the given `Prefix`.
pub fn find_hack_files<'a>(
    files_to_ignore: &'a FilesToIgnore,
    root: &'a Path,
    prefix: relative_path::Prefix,
) -> impl Iterator<Item = anyhow::Result<RelativePath>> + 'a {
    jwalk::WalkDir::new(&root)
        .into_iter()
        .filter_map(move |entry_result| {
            let entry = match entry_result {
                Ok(entry) => entry,
                Err(err) => return Some(Err(anyhow::anyhow!(err))),
            };
            if entry.file_type().is_dir() {
                return None;
            }
            let path = entry.path();
            if !is_hack(&path) {
                return None;
            }
            let suffix = path.strip_prefix(&root).unwrap();
            if files_to_ignore.should_ignore(&suffix) {
                return None;
            }
            let path = RelativePath::make(prefix, suffix.to_owned());
            Some(Ok(path))
        })
}

#[cfg(test)]
mod tests {
    use std::path::PathBuf;

    use pretty_assertions::assert_eq;

    use super::*;

    #[test]
    fn dot_files() {
        assert!(is_dot_file_or_dir(Path::new("/my/path/.hello")));
        assert!(is_dot_file_or_dir(Path::new("/my/path/.hello/")));
        assert!(is_dot_file_or_dir(Path::new("/my/path/.hello/.")));

        assert!(!is_dot_file_or_dir(Path::new(" ")));
        assert!(!is_dot_file_or_dir(Path::new(".")));
        assert!(!is_dot_file_or_dir(Path::new("/my/path/.")));
        assert!(!is_dot_file_or_dir(Path::new("/my/path/hello")));
    }

    #[test]
    fn is_hack_file() {
        assert!(is_hack(Path::new("hello.php")));
        assert!(is_hack(Path::new("/my/path/hello.php")));
        assert!(is_hack(Path::new("/my/path/hello.phpt")));
        assert!(is_hack(Path::new("/my/path/hello.hack")));
        assert!(is_hack(Path::new("/my/path/foo.hackpartial")));
        assert!(is_hack(Path::new("/my/path/ack.hck")));
        assert!(is_hack(Path::new("/my/path/bar.hh")));
        assert!(is_hack(Path::new("/my/path/foo.hhi")));
        assert!(is_hack(Path::new("/my/path/ack.xhp")));

        // hidden file
        assert!(!is_hack(Path::new("/my/path/.hello.xhp")));

        // wrong extension
        assert!(!is_hack(Path::new("/my/path/hello.txt")));

        // no extension
        assert!(!is_hack(Path::new("/my/path/foo")));
    }

    #[test]
    fn consider_files_test() {
        let paths = vec![
            PathBuf::from("/a/b/.git/hello.php"),
            PathBuf::from("alpha.hack"),
            PathBuf::from("/my/path/flib/intern/third-party/phpunit/phpunit/hello.phpt"),
            PathBuf::from("/my/path/check.hh"),
            PathBuf::from("some/other/.hg/path/uhoh.php"),
            PathBuf::from("/wrong/extension/oops.txt"),
        ];

        assert_eq!(
            paths
                .iter()
                .map(PathBuf::as_path)
                .filter(|path| is_non_ignored_hack(&Default::default(), path))
                .collect::<Vec<_>>(),
            vec![Path::new("alpha.hack"), Path::new("/my/path/check.hh")]
        );
    }
}
