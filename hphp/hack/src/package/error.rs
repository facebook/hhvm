// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use toml::Spanned;

use crate::Config;
use crate::NameSet;

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

pub fn check_config(config: &Config) -> Vec<Error> {
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
