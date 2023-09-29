// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ffi::OsStr;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;

use anyhow::bail;
use anyhow::Result;
use parking_lot::Mutex;

pub type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

/// Took this from https://docs.rs/once_cell/latest/once_cell/index.html#lazily-compiled-regex
/// with this macro we can avoid re-initializing regexes, which are expensive to do
#[macro_export]
macro_rules! regex {
    ($re:literal $(,)?) => {{
        static RE: std::sync::OnceLock<Regex> = std::sync::OnceLock::new();
        RE.get_or_init(|| Regex::new($re).unwrap())
    }};
}

pub(crate) fn collect_files(
    paths: &[PathBuf],
    limit: Option<usize>,
    _num_threads: usize,
) -> Result<Vec<PathBuf>> {
    fn is_php_file_name(file: &OsStr) -> bool {
        use std::os::unix::ffi::OsStrExt;
        let file = file.as_bytes();
        file.ends_with(b".php") || file.ends_with(b".hack")
    }

    let mut files: Vec<(u64, PathBuf)> = paths
        .iter()
        .map(|path| {
            if !path.exists() {
                bail!("File or directory '{}' not found", path.display());
            }

            use jwalk::DirEntry;
            use jwalk::Result;
            use jwalk::WalkDir;
            fn on_read_dir(
                _: Option<usize>,
                path: &Path,
                _: &mut (),
                children: &mut Vec<Result<DirEntry<((), ())>>>,
            ) {
                children.retain(|dir_entry_result| {
                    dir_entry_result.as_ref().map_or(false, |dir_entry| {
                        let file_type = &dir_entry.file_type;
                        if file_type.is_file() {
                            is_php_file_name(dir_entry.file_name())
                        } else {
                            // The HHVM server files contain some weird
                            // multi-file-in-one tests that confuse the
                            // comparator (because you end up with two classes
                            // of the same name, etc) so skip them unless
                            // they're explicitly given. Hopefully this doesn't
                            // exclude something from outside HHVM somehow.
                            !(dir_entry.file_name() == "server" && path.ends_with("test"))
                        }
                    })
                });
            }
            let walker = WalkDir::new(path).process_read_dir(on_read_dir);

            let mut files = Vec::new();
            for dir_entry in walker {
                let dir_entry = dir_entry?;
                if dir_entry.file_type.is_file() {
                    let len = dir_entry.metadata()?.len();
                    files.push((len, dir_entry.path()));
                }
            }
            Ok(files)
        })
        .collect::<Result<Vec<_>>>()?
        .into_iter()
        .flatten()
        .collect();

    // Sort largest first process outliers first, then by path.
    files.sort_unstable_by(|(len1, path1), (len2, path2)| {
        len1.cmp(len2).reverse().then(path1.cmp(path2))
    });
    let mut files: Vec<PathBuf> = files.into_iter().map(|(_, path)| path).collect();
    if let Some(limit) = limit {
        if files.len() > limit {
            files.resize_with(limit, || unreachable!());
        }
    }

    Ok(files)
}
