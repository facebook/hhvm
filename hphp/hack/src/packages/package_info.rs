// Copyright (c) Meta, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use anyhow::Context;
use anyhow::Result;
use toml::Spanned;

use crate::config::*;
use crate::error::*;
use crate::types::DeploymentMap;
use crate::types::ImplicitPackageMap;
pub use crate::types::NameSet;
use crate::types::PackageMap;

#[derive(Debug, Default)]
pub struct PackageInfo {
    packages: PackageMap,
    deployments: Option<DeploymentMap>,
    implicit_packages: ImplicitPackageMap,
    line_offsets: Vec<usize>,
    errors: Vec<Error>,
}

impl PackageInfo {
    fn from_text(
        strict: bool,
        enable_implicit_packages: bool,
        packages_toml: &str,
    ) -> Result<PackageInfo> {
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

        if !enable_implicit_packages {
            // The feature is gated off (the default). An `[implicit_packages]`
            // stanza is a hard error, and we do not process families any
            // further -- so a config that declares one behaves as if the
            // section were absent (besides the error). This keeps the feature
            // fully inert until it is explicitly enabled in .hhconfig.
            let disabled: Vec<_> = config.implicit_packages.keys().cloned().collect();
            for name in &disabled {
                errors.push(Error::implicit_packages_disabled(name));
            }
            config.implicit_packages.clear();
        } else {
            // Normalize implicit-package family paths to the same leading-`//`-
            // stripped form used for include_paths. Crucially, this performs NO
            // filesystem access -- not even in strict mode -- because a family's
            // members are resolved lazily at lookup time, so the directories
            // under `path` need not exist (or be checked out) when PACKAGES.toml
            // is read. A family whose `path` is malformed is dropped after
            // reporting, so it does not produce confusing secondary errors from
            // the disjointness / closure checks in `check_config`.
            let mut malformed: Vec<Spanned<String>> = vec![];
            for (name, fam) in config.implicit_packages.iter_mut() {
                let span = fam.path.span();
                let dir = fam.path.get_ref().clone();
                if !dir.starts_with("//") || dir.contains("./") || !dir.ends_with('/') {
                    errors.push(Error::malformed_include_path(dir.clone(), span));
                    malformed.push(name.clone());
                    continue;
                }
                let relative = dir.strip_prefix("//").unwrap_or(&dir).to_string();
                *fam.path.get_mut() = relative;
            }
            for name in malformed {
                config.implicit_packages.shift_remove(&name);
            }
        }

        config.check_config(&mut errors);

        Ok(Self {
            packages: config.packages,
            deployments: config.deployments,
            implicit_packages: config.implicit_packages,
            line_offsets,
            errors,
        })
    }

    pub fn from_text_strict(
        enable_implicit_packages: bool,
        packages_toml: &str,
    ) -> Result<PackageInfo> {
        PackageInfo::from_text(true, enable_implicit_packages, packages_toml)
    }

    pub fn from_text_non_strict(
        enable_implicit_packages: bool,
        packages_toml: &str,
    ) -> Result<PackageInfo> {
        PackageInfo::from_text(false, enable_implicit_packages, packages_toml)
    }

    pub fn packages(&self) -> &PackageMap {
        &self.packages
    }

    pub fn deployments(&self) -> Option<&DeploymentMap> {
        self.deployments.as_ref()
    }

    pub fn implicit_packages(&self) -> &ImplicitPackageMap {
        &self.implicit_packages
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
    use std::sync::LazyLock;

    use regex::Regex;

    use super::*;

    pub static SRCDIR: LazyLock<PathBuf> =
        LazyLock::new(|| Path::new(file!()).parent().unwrap().to_path_buf());

    #[test]
    fn test_parsing_basic_file() {
        let test_path = SRCDIR.as_path().join("tests/package-1.toml");
        let info = PackageInfo::from_text(true, false, test_path.to_str().unwrap()).unwrap();
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
    }

    #[test]
    fn test_config_errors1() {
        let test_path = SRCDIR.as_path().join("tests/package-3.toml");
        let info = PackageInfo::from_text(true, false, test_path.to_str().unwrap()).unwrap();
        assert_eq!(info.errors.len(), 2);
        assert_eq!(info.errors[0].msg(), "Undefined package: baz");
        assert_eq!(info.errors[1].msg(), "Undefined package: baz");
    }

    #[test]
    fn test_config_errors2() {
        let test_path = SRCDIR.as_path().join("tests/package-4.toml");
        let info = PackageInfo::from_text(true, false, test_path.to_str().unwrap()).unwrap();
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
        let info = PackageInfo::from_text(true, false, test_path.to_str().unwrap()).unwrap();
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
        let info = PackageInfo::from_text(true, false, test_path.to_str().unwrap()).unwrap();
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
        let info = PackageInfo::from_text(true, false, test_path.to_str().unwrap()).unwrap();
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
        let info = PackageInfo::from_text(true, false, test_path.to_str().unwrap()).unwrap();

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
        let info = PackageInfo::from_text(false, false, test_path.to_str().unwrap()).unwrap();
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
        let info = PackageInfo::from_text(false, false, test_path.to_str().unwrap()).unwrap();
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
    fn test_include_paths_is_reverse_sorted_in_package() {
        let test_path = SRCDIR.as_path().join("tests/package-8.toml");
        let info = PackageInfo::from_text(false, false, test_path.to_str().unwrap()).unwrap();
        let baz = &info.packages()["baz"];
        let include_paths = &baz.include_paths.as_ref().unwrap();
        assert!(include_paths[0].get_ref().ends_with("longest/"));
        assert!(include_paths[1].get_ref().ends_with("longer/"));
        assert!(include_paths[2].get_ref().ends_with("long/"));
    }

    #[test]
    fn test_no_duplicate_include_paths() {
        let test_path = SRCDIR.as_path().join("tests/package-9.toml");
        let info = PackageInfo::from_text(false, false, test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
        let expected = [
            String::from("This include_path can only be used in one package: path/to/longest/"),
            String::from("This include_path can only be used in one package: path/to/long/"),
        ];
        assert!(errors[0] == expected[0]);
        assert!(errors[1] == expected[1]);
    }

    #[test]
    fn test_implicit_packages_basic() {
        let test_path = SRCDIR.as_path().join("tests/package-implicit.toml");
        let info = PackageInfo::from_text(false, true, test_path.to_str().unwrap()).unwrap();
        assert!(
            info.errors().is_empty(),
            "unexpected errors: {:?}",
            info.errors().iter().map(|e| e.msg()).collect::<Vec<_>>()
        );
        // The family is recorded but NOT expanded into per-directory packages:
        // no member entry appears in the package map at parse time.
        assert!(info.packages().get("prototypes.foo").is_none());
        let fam = &info.implicit_packages()["prototypes"];
        // `path` is normalized to the leading-`//`-stripped form, like include_paths.
        assert_eq!(fam.path.get_ref(), "www/prototypes/");
        assert_eq!(fam.includes.as_ref().unwrap()[0].get_ref(), "intern");
    }

    #[test]
    fn test_implicit_packages_validation_errors() {
        let test_path = SRCDIR.as_path().join("tests/package-implicit-bad.toml");
        let info = PackageInfo::from_text(false, true, test_path.to_str().unwrap()).unwrap();
        let errors = info
            .errors
            .iter()
            .map(|e| e.msg())
            .collect::<std::collections::HashSet<_>>();
        assert_eq!(
            errors,
            [
                String::from(
                    "implicit_packages.www_pkg must not specify include_paths: the include paths are derived from its path",
                ),
                String::from(
                    "implicit_packages path //www/prototypes/ overlaps another package or implicit-family include path; they must be disjoint",
                ),
                String::from(
                    "implicit_packages family www_pkg collides with a package name (a family name may not equal or be a prefix of a package name)",
                ),
            ]
            .iter()
            .cloned()
            .collect::<std::collections::HashSet<_>>()
        );
    }

    #[test]
    fn test_implicit_packages_disabled() {
        // With the feature gated off (the default), an [implicit_packages]
        // stanza is a hard error and the family is not processed.
        let test_path = SRCDIR.as_path().join("tests/package-implicit.toml");
        let info = PackageInfo::from_text(false, false, test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
        assert_eq!(
            errors,
            vec![String::from(
                "[implicit_packages.prototypes] is not permitted: set enable_implicit_packages = true in .hhconfig to use implicit packages",
            )]
        );
        // The family is dropped, so nothing is carried downstream.
        assert!(info.implicit_packages().is_empty());
        // Rejecting the stanza is otherwise inert: the declared package still
        // parses, and the family does not leak in as a regular package.
        assert_eq!(
            info.packages()
                .keys()
                .map(|k| k.get_ref().as_str())
                .collect::<Vec<_>>(),
            vec!["intern"]
        );
    }

    #[test]
    fn test_implicit_family_vs_family_overlap() {
        // Two families whose paths are prefix-related must be rejected, else a
        // file under the overlap would have order-dependent membership.
        let test_path = SRCDIR.as_path().join("tests/package-implicit-overlap.toml");
        let info = PackageInfo::from_text(false, true, test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
        assert_eq!(
            errors,
            vec![String::from(
                "implicit_packages path //www/prototypes/sub/ overlaps another package or implicit-family include path; they must be disjoint",
            )]
        );
    }

    #[test]
    fn test_implicit_deployment_closure() {
        // A deployment may name a family (expands to all members) or an
        // individual member `F.D`; both normalize to the family `F` for the
        // transitive-closure check, so deploying the family together with its
        // hard include `intern` is complete.
        let test_path = SRCDIR.as_path().join("tests/package-implicit-deploy.toml");
        let info = PackageInfo::from_text(false, true, test_path.to_str().unwrap()).unwrap();
        assert!(
            info.errors().is_empty(),
            "unexpected errors: {:?}",
            info.errors().iter().map(|e| e.msg()).collect::<Vec<_>>()
        );
    }

    #[test]
    fn test_implicit_family_name_with_dot_rejected() {
        let test_path = SRCDIR.as_path().join("tests/package-implicit-dotname.toml");
        let info = PackageInfo::from_text(false, true, test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
        assert_eq!(
            errors,
            vec![String::from(
                "implicit_packages family name proto.v1 must not contain '.': the '.' separator is reserved for synthesized member names (family.directory)",
            )]
        );
    }

    #[test]
    fn test_rollout_transitivity() {
        let test_path = SRCDIR.as_path().join("tests/package-rollout.toml");
        let info = PackageInfo::from_text(false, false, test_path.to_str().unwrap()).unwrap();
        let errors = info.errors.iter().map(|e| e.msg()).collect::<Vec<_>>();
        let expected = [String::from(
            "tmp must include all nested included packages. Missing prod, soft",
        )];
        assert!(errors[0] == expected[0]);
    }
}
