// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Result;
use bumpalo::Bump;
use hh24_types::Checksum;
use hh24_types::ToplevelSymbolHash;
use names::FileSummary;
use relative_path::Prefix;
use relative_path::RelativePath;
use tempdir::TempDir;

pub struct TestRepo {
    root: TempDir,
}

impl TestRepo {
    /// Creates a repo with `files`, a map of filename to its contents. Also
    /// creates a .hhconfig.
    ///
    /// Supports nested directories.
    ///
    /// See tests for example usage.
    pub fn new(files: &BTreeMap<&str, &str>) -> Result<Self> {
        let tmpdir = TempDir::new("test_repo")?;
        create_repo(tmpdir.path(), files)?;

        Ok(Self { root: tmpdir })
    }

    pub fn write(&self, rel_path: &dyn AsRef<Path>, text: &str) -> Result<RelativePath> {
        self.write_bytes(rel_path, text.as_bytes())
    }

    pub fn write_bytes(&self, rel_path: &dyn AsRef<Path>, content: &[u8]) -> Result<RelativePath> {
        let mut file = std::fs::File::create(self.root.path().join(rel_path.as_ref()))?;
        file.write_all(content)?;
        Ok(RelativePath::make(
            relative_path::Prefix::Root,
            PathBuf::from(rel_path.as_ref()),
        ))
    }

    pub fn delete(&self, rel_path: &dyn AsRef<Path>) -> Result<RelativePath> {
        std::fs::remove_file(self.root.path().join(rel_path.as_ref()))?;
        Ok(RelativePath::make(
            relative_path::Prefix::Root,
            PathBuf::from(rel_path.as_ref()),
        ))
    }

    pub fn path<'a>(&'a self) -> &'a Path {
        self.root.path()
    }
}

pub fn create_naming_table(path: &Path, files: &BTreeMap<&str, &str>) -> Result<()> {
    names::Names::build_from_iterator(
        path,
        files
            .iter()
            .map(|(path, text)| {
                let relpath = root_path(path);
                let summary = parse_and_summarize(&relpath, text)?;
                Ok((relpath, summary))
            })
            .collect::<Result<Vec<_>>>()?,
    )?;
    Ok(())
}

pub fn parse_and_summarize(relpath: &RelativePath, text: &str) -> Result<FileSummary> {
    let arena = &Bump::new();
    let parsed_file = direct_decl_parser::parse_decls_for_typechecking(
        &Default::default(),
        relpath.clone(),
        text.as_bytes(),
        arena,
    );
    let remove_php_stdlib_if_hhi = true;
    let prefix = relpath.prefix();
    let parsed_file_with_hashes = oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes::new(
        parsed_file,
        remove_php_stdlib_if_hhi,
        prefix,
        arena,
    );
    Ok(names::FileSummary::new(&parsed_file_with_hashes))
}

pub fn calculate_checksum(summaries: &[(RelativePath, FileSummary)]) -> Checksum {
    // Checksum calculations are based on the winner, in case of duplicates.
    // We'll gather all winning decls...
    let mut decls = BTreeMap::new();
    for (relpath, summary) in summaries {
        for (symbol_hash, decl_hash) in summary.decl_hashes() {
            use std::collections::btree_map::Entry;
            match decls.entry(symbol_hash) {
                Entry::Vacant(v) => {
                    v.insert((decl_hash, relpath.clone()));
                }
                Entry::Occupied(mut o) => {
                    let (_, existing_path) = o.get();
                    if relpath < existing_path {
                        o.insert((decl_hash, relpath.clone()));
                    }
                }
            }
        }
    }
    // Now that we know the winners, we can calculate checksum
    let mut expected_checksum = Checksum(0);
    for (symbol_hash, (decl_hash, relpath)) in decls {
        expected_checksum.addremove(symbol_hash, decl_hash, &relpath);
    }
    expected_checksum
}

pub fn type_hash(symbol: &str) -> ToplevelSymbolHash {
    let symbol = if symbol.starts_with('\\') {
        symbol.to_owned()
    } else {
        String::from("\\") + symbol
    };
    ToplevelSymbolHash::from_type(&symbol)
}

pub fn fun_hash(symbol: &str) -> ToplevelSymbolHash {
    let symbol = if symbol.starts_with('\\') {
        symbol.to_owned()
    } else {
        String::from("\\") + symbol
    };
    ToplevelSymbolHash::from_fun(&symbol)
}

pub fn root_path(path: &str) -> RelativePath {
    RelativePath::make(Prefix::Root, PathBuf::from(path))
}

/// Guarantee the file exists, but don't truncate it if it does.
pub fn touch(path: &Path) -> Result<()> {
    std::fs::OpenOptions::new()
        .create(true)
        .write(true)
        .open(path)?;
    Ok(())
}

// Fills the given `root` with `files`, a map of filename to its contents. Also
// creates a .hhconfig in `root`.
//
// Supports nested directories.
fn create_repo(root: &Path, files: &BTreeMap<&str, &str>) -> Result<()> {
    // touch .hhconfig
    touch(&root.join(".hhconfig"))?;
    for (path, text) in files {
        let full_path = root.join(path);
        if let Some(prefix) = full_path.parent() {
            std::fs::create_dir_all(prefix)?;
        }
        let mut file = std::fs::File::create(full_path)?;
        file.write_all(text.as_bytes())?;
    }
    Ok(())
}

#[cfg(test)]
mod test {
    use std::collections::BTreeMap;

    use maplit::btreemap;
    use maplit::convert_args;

    use super::*;

    #[test]
    fn test_repo_expected_files() {
        let files: BTreeMap<&str, &str> = convert_args!(btreemap!(
            "foo.php" => r#"
<?hh
class Foo {
    public function get(): int { return 0; }
}
            "#,
            "nested/bar.php" => r#"
<?hh
class Bar {
    public function get(): int { return 0; }
}
            "#,
        ));
        let repo = TestRepo::new(&files).unwrap();
        assert!(repo.path().join("foo.php").exists());
        assert!(repo.path().join("nested/bar.php").exists());
    }
}
