// Copyright (c) Meta, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Context;
use anyhow::Result;
use hash::IndexMap;
use hash::IndexSet;
use serde::Deserialize;
use toml::Spanned;

// Preserve the order for ease of testing
// Alternatively, we could use HashMap for performance
type PackageMap = IndexMap<Spanned<String>, Package>;
type DeploymentMap = IndexMap<String, Deployment>;
pub type NameSet = IndexSet<Spanned<String>>;

#[derive(Debug, Deserialize)]
struct Config {
    packages: PackageMap,
    deployments: Option<DeploymentMap>,
}

#[derive(Debug, Deserialize)]
pub struct Package {
    pub uses: Option<NameSet>,
    pub includes: Option<NameSet>,
}

#[derive(Debug, Deserialize)]
pub struct Deployment {
    pub packages: Option<NameSet>,
    pub domains: Option<NameSet>,
}

#[derive(Debug)]
pub struct PackageInfo {
    packages: PackageMap,
    deployments: Option<DeploymentMap>,
    line_offsets: Vec<usize>,
    errors: Vec<Error>,
}

#[derive(Debug, PartialEq)]
pub enum Error {
    UndefinedInclude { name: String, span: (usize, usize) },
    DuplicateUse { name: String, span: (usize, usize) },
}

impl Error {
    pub fn undefined_package(x: &Spanned<String>) -> Self {
        Self::UndefinedInclude {
            name: x.get_ref().into(),
            span: x.span(),
        }
    }

    pub fn duplicate_use(x: &Spanned<String>) -> Self {
        Self::DuplicateUse {
            name: x.get_ref().into(),
            span: x.span(),
        }
    }

    pub fn span(&self) -> (usize, usize) {
        match self {
            Self::DuplicateUse { span, .. } | Self::UndefinedInclude { span, .. } => *span,
        }
    }

    pub fn msg(&self) -> String {
        match self {
            Self::UndefinedInclude { name, .. } => {
                format!("Undefined package: {}", name)
            }
            Self::DuplicateUse { name, .. } => {
                format!("This module can only be used in one package: {}", name)
            }
        }
    }
}

impl PackageInfo {
    pub fn from_text(contents: &str) -> Result<PackageInfo> {
        let config: Config = toml::from_str(contents)
            .with_context(|| format!("Failed to parse config file with contents: {}", contents))?;
        let line_offsets = contents
            .char_indices()
            .filter(|&(_i, c)| c == '\n')
            .map(|(i, _)| i)
            .collect::<Vec<_>>();
        let errors = check_config(&config);
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

fn check_config(config: &Config) -> Vec<Error> {
    let check_packages_are_defined = |errors: &mut Vec<Error>, pkgs: &Option<NameSet>| {
        if let Some(packages) = pkgs {
            packages.iter().for_each(|package| {
                if !config.packages.contains_key(package) {
                    errors.push(Error::undefined_package(package))
                }
            })
        }
    };
    let mut used_globs = NameSet::default();
    let mut check_each_glob_is_used_once = |errors: &mut Vec<Error>, globs: &Option<NameSet>| {
        if let Some(l) = globs {
            l.iter().for_each(|glob| {
                if used_globs.contains(glob) {
                    errors.push(Error::duplicate_use(glob))
                }
                used_globs.insert(glob.clone());
            })
        }
    };
    let mut errors = vec![];
    for (_, package) in config.packages.iter() {
        check_packages_are_defined(&mut errors, &package.includes);
        check_each_glob_is_used_once(&mut errors, &package.uses);
    }
    if let Some(deployments) = &config.deployments {
        for (_, deployment) in deployments.iter() {
            check_packages_are_defined(&mut errors, &deployment.packages);
        }
    };
    errors
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_parsing_basic_file() {
        let contents = include_str!("tests/package-1.toml");
        let info = PackageInfo::from_text(contents).unwrap();

        let foo = &info.packages()["foo"];
        assert_eq!(foo.uses.as_ref().unwrap()[0].get_ref(), "a.*");
        assert!(foo.includes.is_none());
        assert_eq!(info.line_number(foo.uses.as_ref().unwrap()[0].span().0), 4);

        let bar = &info.packages()["bar"];
        assert_eq!(bar.uses.as_ref().unwrap()[0].get_ref(), "b.*");
        assert_eq!(bar.includes.as_ref().unwrap()[0].get_ref(), "foo");

        let baz = &info.packages()["baz"];
        assert_eq!(baz.uses.as_ref().unwrap()[0].get_ref(), "x.*");
        assert_eq!(baz.uses.as_ref().unwrap()[1].get_ref(), "y.*");
        assert_eq!(baz.includes.as_ref().unwrap()[0].get_ref(), "foo");
        assert_eq!(baz.includes.as_ref().unwrap()[1].get_ref(), "bar");
        assert_eq!(info.line_number(baz.uses.as_ref().unwrap()[0].span().0), 11);
        assert_eq!(info.line_number(baz.uses.as_ref().unwrap()[1].span().0), 11);

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
        let info = PackageInfo::from_text(contents).unwrap();

        let foo = &info.packages()["foo"];
        let foo_uses = &foo.uses.as_ref().unwrap();
        assert_eq!(foo_uses[0].get_ref(), "a.*");
        assert_eq!(foo_uses[1].get_ref(), "b.*");
        assert_eq!(info.line_number(foo_uses[0].span().0), 7);
        assert_eq!(info.line_number(foo_uses[1].span().0), 9);

        let foo_includes = &foo.includes.as_ref().unwrap();
        assert_eq!(foo_includes[0].get_ref(), "bar");
        assert_eq!(info.line_number(foo_includes[0].span().0), 12);
    }

    #[test]
    fn test_config_errors() {
        let contents = include_str!("tests/package-3.toml");
        let info = PackageInfo::from_text(contents).unwrap();
        assert_eq!(info.errors.len(), 2);
        assert_eq!(info.errors[0].msg(), "Undefined package: baz");
        assert_eq!(
            info.errors[1].msg(),
            "This module can only be used in one package: b.*"
        );
    }
}
