// Copyright (c) Meta, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// use std::path::Component;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Context;
use anyhow::Result;

use crate::config::*;
use crate::error::*;
use crate::types::DeploymentMap;
pub use crate::types::NameSet;
use crate::types::PackageMap;

#[derive(Debug, Default)]
pub struct PackageInfo {
    packages: PackageMap,
    deployments: Option<DeploymentMap>,
    line_offsets: Vec<usize>,
    errors: Vec<Error>,
}

impl PackageInfo {
    pub fn from_text(strict: bool, root: &str, filename: &str) -> Result<PackageInfo> {
        let mut errors = vec![];
        let root = if !root.is_empty() {
            Path::new(root)
                .canonicalize()
                .with_context(|| format!("Failed to canonicalize root path: {}", root))?
        } else {
            PathBuf::new()
        };
        let contents = std::fs::read_to_string(filename)
            .with_context(|| format!("Failed to read config file with path: {}", filename))?;
        let mut config: Config = toml::from_str(&contents)
            .with_context(|| format!("Failed to parse config file with contents: {}", contents))?;
        let line_offsets = contents
            .char_indices()
            .filter(|&(_i, c)| c == '\n')
            .map(|(i, _)| i)
            .collect::<Vec<_>>();

        // absolutize directories relative to config file's parent directory
        for (_, package) in config.packages.iter_mut() {
            if let Some(dirs) = &mut package.include_paths {
                dirs.clone().iter().for_each(|d| {
                    let mut spanned_dir = dirs.take(d).unwrap();
                    let span = spanned_dir.span();
                    let dir = spanned_dir.get_ref();

                    if !dir.starts_with("//") || dir.contains("./") {
                        errors.push(Error::malformed_include_path(dir.clone(), span.clone()))
                    }

                    let rel_path = Path::new(dir.strip_prefix("//").unwrap_or(dir));
                    let abs_path = Path::new(filename)
                        .parent()
                        .expect("Package config path must be valid")
                        .join(rel_path);

                    if strict {
                        let path_exists = abs_path.canonicalize().is_ok();
                        if !path_exists {
                            errors
                                .push(Error::invalid_include_path(abs_path.clone(), span.clone()));
                        } else {
                            let metadata = std::fs::metadata(&abs_path).unwrap();
                            if metadata.is_dir() && !dir.ends_with("/") {
                                errors.push(Error::invalid_include_path(
                                    abs_path.clone(),
                                    span.clone(),
                                ));
                            }
                        }
                    }

                    let new_dir = abs_path
                        .as_path()
                        .strip_prefix(root.clone())
                        .unwrap_or(&abs_path);
                    *spanned_dir.get_mut() = new_dir.to_string_lossy().into_owned();
                    dirs.insert(spanned_dir);
                });
            }
        }

        config.check_config(&mut errors);

        Ok(Self {
            packages: config.packages,
            deployments: config.deployments,
            line_offsets,
            errors,
        })
    }

    pub fn from_text_strict(root: &str, filename: &str) -> Result<PackageInfo> {
        PackageInfo::from_text(true, root, filename)
    }

    pub fn packages(&self) -> &PackageMap {
        &self.packages
    }

    pub fn deployments(&self) -> Option<&DeploymentMap> {
        self.deployments.as_ref()
    }

    pub fn errors(&self) -> &[Error] {
        &self.errors[..]
    }

    pub fn line_number(&self, byte_offset: usize) -> usize {
        match self.line_offsets.binary_search(&byte_offset) {
            Ok(n) | Err(n) => n + 1,
        }
    }

    pub fn beginning_of_line(&self, line_number: usize) -> usize {
        if line_number == 1 {
            1
        } else {
            let line_idx = line_number - 1;
            let prev_line_idx = line_idx - 1;
            let prev_line_end = self.line_offsets[prev_line_idx];
            prev_line_end + 1
        }
    }
}

#[cfg(test)]
mod test {
    use std::path::Path;
    use std::path::PathBuf;

    use lazy_static::lazy_static;
    use regex::Regex;

    use super::*;

    lazy_static! {
        pub static ref SRCDIR: PathBuf = Path::new(file!()).parent().unwrap().to_path_buf();
    }

    #[test]
    fn test_parsing_basic_file() {
        let test_path = SRCDIR.as_path().join("tests/package-1.toml");
        let info = PackageInfo::from_text(true, "", test_path.to_str().unwrap()).unwrap();
        assert!(info.errors.is_empty());

        let foo = &info.packages()["foo"];
        assert_eq!(foo.uses.as_ref().unwrap()[0].get_ref(), "a.*");
        assert!(foo.includes.is_none());
        assert_eq!(
            info.line_number(foo.uses.as_ref().unwrap()[0].span().start),
            4
        );

        let bar = &info.packages()["bar"];
        assert_eq!(bar.uses.as_ref().unwrap()[0].get_ref(), "b.*");
        assert_eq!(bar.includes.as_ref().unwrap()[0].get_ref(), "foo");

        let baz = &info.packages()["baz"];
        assert_eq!(baz.uses.as_ref().unwrap()[0].get_ref(), "x.*");
        assert_eq!(baz.uses.as_ref().unwrap()[1].get_ref(), "y.*");
        assert_eq!(baz.includes.as_ref().unwrap()[0].get_ref(), "foo");
        assert_eq!(baz.includes.as_ref().unwrap()[1].get_ref(), "bar");
        assert_eq!(
            info.line_number(baz.uses.as_ref().unwrap()[0].span().start),
            11
        );
        assert_eq!(
            info.line_number(baz.uses.as_ref().unwrap()[1].span().start),
            11
        );

        let my_prod = &info.deployments().unwrap()["my-prod"];
        assert_eq!(my_prod.packages.as_ref().unwrap()[0].get_ref(), "foo");
        assert_eq!(my_prod.packages.as_ref().unwrap()[1].get_ref(), "bar");
        assert_eq!(
            my_prod.domains.as_ref().unwrap()[0].get_ref(),
            r"www\.my-prod\.com"
        );
        assert_eq!(
            my_prod.domains.as_ref().unwrap()[1].get_ref(),
            r".*\.website\.com$"
        );
    }

    #[test]
    fn test_multiline_uses() {
        let test_path = SRCDIR.as_path().join("tests/package-2.toml");
        let info = PackageInfo::from_text(true, "", test_path.to_str().unwrap()).unwrap();
        assert!(info.errors.is_empty());

        let foo = &info.packages()["foo"];
        let foo_uses = &foo.uses.as_ref().unwrap();
        assert_eq!(foo_uses[0].get_ref(), "a.*");
        assert_eq!(foo_uses[1].get_ref(), "b.*");
        assert_eq!(info.line_number(foo_uses[0].span().start), 7);
        assert_eq!(info.line_number(foo_uses[1].span().start), 9);

        let foo_includes = &foo.includes.as_ref().unwrap();
        assert_eq!(foo_includes[0].get_ref(), "bar");
        assert_eq!(info.line_number(foo_includes[0].span().start), 12);
    }

    #[test]
    fn test_config_errors1() {
        let test_path = SRCDIR.as_path().join("tests/package-3.toml");
        let info = PackageInfo::from_text(true, "", test_path.to_str().unwrap()).unwrap();
        assert_eq!(info.errors.len(), 3);
        assert_eq!(info.errors[0].msg(), "Undefined package: baz");
        assert_eq!(info.errors[1].msg(), "Undefined package: baz");
        assert_eq!(
            info.errors[2].msg(),
            "This module can only be used in one package: b.*"
        );
    }

    #[test]
    fn test_config_errors2() {
        let test_path = SRCDIR.as_path().join("tests/package-4.toml");
        let info = PackageInfo::from_text(true, "", test_path.to_str().unwrap()).unwrap();
        let errors = info
            .errors
            .iter()
            .map(|e| e.msg())
            .collect::<std::collections::HashSet<_>>();
        assert_eq!(
            errors,
            [String::from(
                "my-prod must deploy all nested included packages. Missing e, g, h, i"
            )]
            .iter()
            .cloned()
            .collect::<std::collections::HashSet<_>>()
        );
    }

    #[test]
    fn test_soft() {
        let test_path = SRCDIR.as_path().join("tests/package-5.toml");
        let info = PackageInfo::from_text(true, "", test_path.to_str().unwrap()).unwrap();
        let c = &info.packages()["c"];
        let errors = info
            .errors
            .iter()
            .map(|e| e.msg())
            .collect::<std::collections::HashSet<_>>();
        assert_eq!(
            errors,
            [
                String::from("f must soft-deploy all nested soft-included packages. Missing b"),
                String::from("g must deploy all nested included packages. Missing c")
            ]
            .iter()
            .cloned()
            .collect::<std::collections::HashSet<_>>()
        );

        assert_eq!(c.soft_includes.as_ref().unwrap()[0].get_ref(), "b");

        let d = &info.deployments().unwrap()["d"];
        assert_eq!(d.packages.as_ref().unwrap()[0].get_ref(), "c");
        assert_eq!(d.soft_packages.as_ref().unwrap()[0].get_ref(), "b");
    }

    #[test]
    fn test_include_paths1() {
        let test_path = SRCDIR.as_path().join("tests/package-6.toml");
        let info = PackageInfo::from_text(true, "", test_path.to_str().unwrap()).unwrap();
        let included_dirs = info.packages()["foo"].include_paths.as_ref().unwrap();
        assert_eq!(included_dirs.len(), 2);
        assert!(
            Regex::new(&format!(
                r#".*{}/tests/doesnotexist.php"#,
                SRCDIR.to_string_lossy()
            ))
            .unwrap()
            .is_match(included_dirs[0].get_ref())
        );
        assert!(
            Regex::new(&format!(
                r#".*{}/tests/doesnotexist"#,
                SRCDIR.to_string_lossy()
            ))
            .unwrap()
            .is_match(included_dirs[1].get_ref())
        );
    }

    #[test]
    fn test_include_paths_error() {
        let test_path = SRCDIR.as_path().join("tests/package-6.toml");
        let info = PackageInfo::from_text(true, "", test_path.to_str().unwrap()).unwrap();

        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();

        assert_eq!(errors.len(), 9);

        let expected = [
            String::from(
                "include_path * is malformed: paths must start with // and cannot include ./ or ../, directories must end with /",
            ),
            format!(
                r#"include_path .*{}/tests/doesnotexist.php does not exist"#,
                SRCDIR.to_string_lossy()
            ),
            String::from(
                "include_path bar is malformed: paths must start with // and cannot include ./ or ../, directories must end with /",
            ),
            String::from(
                "include_path bar/ is malformed: paths must start with // and cannot include ./ or ../, directories must end with /",
            ),
        ];

        assert!(expected[0] == errors[7]);
        assert!(Regex::new(&expected[1]).unwrap().is_match(&errors[0]));
        assert!(expected[2] == errors[2]);
        assert!(expected[3] == errors[5]);
    }

    #[test]
    fn test_include_paths_non_strict() {
        let test_path = SRCDIR.as_path().join("tests/package-6.toml");
        let info = PackageInfo::from_text(false, "", test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
        assert!(errors.len() == 3);
        // with non-strict PackageInfo parsing only "malformed path" errors should be generated
        let expected = Regex::new(r#".*malformed.*"#).unwrap();
        let filtered_errors = errors.iter().filter(|x| !expected.is_match(x));
        assert!(filtered_errors.count() == 0);
    }

    #[test]
    fn test_include_paths_error_2() {
        println!(" !!! ");
        let test_path = SRCDIR.as_path().join("tests/package-7.toml");
        let info = PackageInfo::from_text(false, "", test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
        println!(" *** {:?}", errors);
        let expected = [
            String::from(
                "include_path //doesnotexist/./bar/ is malformed: paths must start with // and cannot include ./ or ../, directories must end with /",
            ),
            String::from(
                "include_path //doesnotexist/../bar/ is malformed: paths must start with // and cannot include ./ or ../, directories must end with /",
            ),
        ];
        assert!(errors[0] == expected[0]);
        assert!(errors[1] == expected[1]);
    }
}
