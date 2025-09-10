// Copyright (c) Meta, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

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
    fn from_text(strict: bool, packages_toml: &str) -> Result<PackageInfo> {
        let mut errors = vec![];

        // read the PACKAGES.toml file
        let contents = std::fs::read_to_string(packages_toml)
            .with_context(|| format!("Failed to read config file with path: {}", packages_toml))?;
        let mut config: Config = toml::from_str(&contents)
            .with_context(|| format!("Failed to parse config file with contents: {}", contents))?;
        let line_offsets = contents
            .char_indices()
            .filter(|&(_i, c)| c == '\n')
            .map(|(i, _)| i)
            .collect::<Vec<_>>();

        // perform error check on include_paths
        for (_, package) in config.packages.iter_mut() {
            if let Some(dirs) = &mut package.include_paths {
                let dirs_cloned = dirs.clone();
                dirs_cloned.iter().for_each(|d| {
                    let mut spanned_dir = dirs.take(d).unwrap();
                    let span = spanned_dir.span();
                    let dir = spanned_dir.get_ref();

                    if !dir.starts_with("//") || dir.contains("./") {
                        errors.push(Error::malformed_include_path(dir.clone(), span.clone()))
                    }
                    let include_path = Path::new(dir.strip_prefix("//").unwrap_or(dir));
                    let relative_include_path = include_path.to_path_buf();

                    if strict {
                        let packages_toml_path =
                            Path::new(packages_toml).parent().unwrap_or(Path::new("/"));
                        let include_path_abs = packages_toml_path.join(include_path).canonicalize();
                        match include_path_abs {
                            Ok(p) => {
                                let metadata = std::fs::metadata(&p).unwrap();
                                if metadata.is_dir() && !dir.ends_with("/") {
                                    errors.push(Error::malformed_include_path(
                                        include_path.to_string_lossy().into_owned(),
                                        span.clone(),
                                    ));
                                }
                            }
                            Err(_) => {
                                errors.push(Error::invalid_include_path(
                                    include_path.to_string_lossy().into_owned(),
                                    span.clone(),
                                ));
                            }
                        }
                    }

                    *spanned_dir.get_mut() = relative_include_path.to_string_lossy().into_owned();
                    dirs.insert(spanned_dir);
                });
                dirs.sort_by(|a, b| b.get_ref().cmp(a.get_ref()));
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

    pub fn from_text_strict(packages_toml: &str) -> Result<PackageInfo> {
        PackageInfo::from_text(true, packages_toml)
    }

    pub fn from_text_non_strict(packages_toml: &str) -> Result<PackageInfo> {
        PackageInfo::from_text(false, packages_toml)
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
        let info = PackageInfo::from_text(true, test_path.to_str().unwrap()).unwrap();
        assert!(info.errors.is_empty());

        let foo = &info.packages()["foo"];
        assert!(foo.includes.is_none());

        let bar = &info.packages()["bar"];
        assert_eq!(bar.includes.as_ref().unwrap()[0].get_ref(), "foo");

        let baz = &info.packages()["baz"];
        assert_eq!(baz.includes.as_ref().unwrap()[0].get_ref(), "foo");
        assert_eq!(baz.includes.as_ref().unwrap()[1].get_ref(), "bar");

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
    fn test_config_errors1() {
        let test_path = SRCDIR.as_path().join("tests/package-3.toml");
        let info = PackageInfo::from_text(true, test_path.to_str().unwrap()).unwrap();
        assert_eq!(info.errors.len(), 2);
        assert_eq!(info.errors[0].msg(), "Undefined package: baz");
        assert_eq!(info.errors[1].msg(), "Undefined package: baz");
    }

    #[test]
    fn test_config_errors2() {
        let test_path = SRCDIR.as_path().join("tests/package-4.toml");
        let info = PackageInfo::from_text(true, test_path.to_str().unwrap()).unwrap();
        let errors = info
            .errors
            .iter()
            .map(|e| e.msg())
            .collect::<std::collections::HashSet<_>>();
        assert_eq!(
            errors,
            [
                String::from(
                    "my-prod must deploy all nested included packages. Missing e, g, h, i",
                ),
                String::from("a must include all nested included packages. Missing c, d, e, f, g"),
                String::from("b must include all nested included packages. Missing a, e, f, g"),
                String::from("c must include all nested included packages. Missing b, d, f, g"),
                String::from("d must include all nested included packages. Missing f, g"),
                String::from("f must include all nested included packages. Missing g"),
                String::from("g must include all nested included packages. Missing f"),
                String::from("h must include all nested included packages. Missing i"),
                String::from("i must include all nested included packages. Missing j"),
                String::from("j must include all nested included packages. Missing h"),
            ]
            .iter()
            .cloned()
            .collect::<std::collections::HashSet<_>>()
        );
    }

    #[test]
    fn test_config_internprod() {
        let test_path = SRCDIR.as_path().join("tests/package-internprod.toml");
        let info = PackageInfo::from_text(true, test_path.to_str().unwrap()).unwrap();
        let errors = info
            .errors
            .iter()
            .map(|e| e.msg())
            .collect::<std::collections::HashSet<_>>();
        eprintln!("{:?}", errors);
        assert_eq!(
            errors,
            [
                String::from("intern3 must soft-include all nested soft-included packages. Missing prod_pulled_from_intern"),
                String::from("prod3 must soft-deploy all nested soft-included packages. Missing prod_pulled_from_intern"),
            ]
            .iter()
            .cloned()
            .collect::<std::collections::HashSet<_>>()
        );
    }

    #[test]
    fn test_soft() {
        let test_path = SRCDIR.as_path().join("tests/package-5.toml");
        let info = PackageInfo::from_text(true, test_path.to_str().unwrap()).unwrap();
        let c = &info.packages()["c"];
        let errors = info
            .errors
            .iter()
            .map(|e| e.msg())
            .collect::<std::collections::HashSet<_>>();
        assert_eq!(
            errors,
            [
                String::from("a must include all nested included packages. Missing c"),
                String::from("f must soft-deploy all nested soft-included packages. Missing b"),
                String::from("g must deploy all nested included packages. Missing c"),
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
        let info = PackageInfo::from_text(true, test_path.to_str().unwrap()).unwrap();
        let included_dirs = info.packages()["foo"].include_paths.as_ref().unwrap();
        assert_eq!(included_dirs.len(), 2);
        assert!(
            Regex::new("doesnotexist.php")
                .unwrap()
                .is_match(included_dirs[1].get_ref())
        );
        assert!(
            Regex::new("doesnotexist/")
                .unwrap()
                .is_match(included_dirs[0].get_ref())
        );
    }

    #[test]
    fn test_include_paths_error() {
        let test_path = SRCDIR.as_path().join("tests/package-6.toml");
        let info = PackageInfo::from_text(true, test_path.to_str().unwrap()).unwrap();

        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();

        assert_eq!(errors.len(), 9);

        let expected = [
            String::from(
                "include_path * is malformed: paths must start with // and cannot include ./ or ../, directories must end with /",
            ),
            String::from(r#"include_path doesnotexist.php does not exist"#),
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
        let info = PackageInfo::from_text(false, test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
        assert!(errors.len() == 3);
        // with non-strict PackageInfo parsing only "malformed path" errors should be generated
        let expected = Regex::new(r#".*malformed.*"#).unwrap();
        let filtered_errors = errors.iter().filter(|x| !expected.is_match(x));
        assert!(filtered_errors.count() == 0);
    }

    #[test]
    fn test_include_paths_error_2() {
        let test_path = SRCDIR.as_path().join("tests/package-7.toml");
        let info = PackageInfo::from_text(false, test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
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

    #[test]
    fn test_include_paths_is_reverse_sorted_in_package_v2() {
        let test_path = SRCDIR.as_path().join("tests/package-8.toml");
        let info = PackageInfo::from_text(false, test_path.to_str().unwrap()).unwrap();
        let baz = &info.packages()["baz"];
        let include_paths = &baz.include_paths.as_ref().unwrap();
        assert!(include_paths[0].get_ref().ends_with("longest/"));
        assert!(include_paths[1].get_ref().ends_with("longer/"));
        assert!(include_paths[2].get_ref().ends_with("long/"));
    }

    #[test]
    fn test_no_duplicate_include_paths() {
        let test_path = SRCDIR.as_path().join("tests/package-9.toml");
        let info = PackageInfo::from_text(false, test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
        let expected = [
            String::from("This include_path can only be used in one package: path/to/longest/"),
            String::from("This include_path can only be used in one package: path/to/long/"),
        ];
        assert!(errors[0] == expected[0]);
        assert!(errors[1] == expected[1]);
    }
}
