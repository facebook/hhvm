// Copyright (c) Meta, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

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
    pub fn from_text(_filename: &str, contents: &str) -> Result<PackageInfo> {
        let config: Config = toml::from_str(contents)
            .with_context(|| format!("Failed to parse config file with contents: {}", contents))?;
        let line_offsets = contents
            .char_indices()
            .filter(|&(_i, c)| c == '\n')
            .map(|(i, _)| i)
            .collect::<Vec<_>>();
        let errors = config.check_config();
        Ok(Self {
            packages: config.packages,
            deployments: config.deployments,
            line_offsets,
            errors,
        })
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

    use super::*;

    lazy_static! {
        pub static ref SRCDIR: PathBuf = Path::new(file!()).parent().unwrap().to_path_buf();
    }

    #[test]
    fn test_parsing_basic_file() {
        let contents = include_str!("tests/package-1.toml");
        let test_path = SRCDIR.as_path().join("tests/package-1.toml");

        let info = PackageInfo::from_text(test_path.to_str().unwrap(), contents).unwrap();
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
        let contents = include_str!("tests/package-2.toml");
        let test_path = SRCDIR.as_path().join("tests/package-2.toml");

        let info = PackageInfo::from_text(test_path.to_str().unwrap(), contents).unwrap();
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
        let contents = include_str!("tests/package-3.toml");
        let test_path = SRCDIR.as_path().join("tests/package-3.toml");

        let info = PackageInfo::from_text(test_path.to_str().unwrap(), contents).unwrap();
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
        let contents = include_str!("tests/package-4.toml");
        let test_path = SRCDIR.as_path().join("tests/package-4.toml");

        let info = PackageInfo::from_text(test_path.to_str().unwrap(), contents).unwrap();
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
        let contents = include_str!("tests/package-5.toml");
        let test_path = SRCDIR.as_path().join("tests/package-5.toml");

        let info = PackageInfo::from_text(test_path.to_str().unwrap(), contents).unwrap();
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
    fn test_allow_directories1() {
        let contents = include_str!("tests/package-6.toml");
        let test_path = SRCDIR.as_path().join("tests/package-6.toml");

        let info = PackageInfo::from_text(test_path.to_str().unwrap(), contents).unwrap();
        let allow_dirs = info.packages()["foo"].allow_directories.as_ref().unwrap();
        assert_eq!(allow_dirs.len(), 4);
        assert_eq!(allow_dirs[0].get_ref(), "foo");
        assert_eq!(allow_dirs[1].get_ref(), "foo/*");
        assert_eq!(allow_dirs[2].get_ref(), "./foo");
        assert_eq!(allow_dirs[3].get_ref(), "./foo/*");
    }
}
