// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fmt::Display;
use std::fmt::Formatter;
use std::fmt::Result;

use toml::Spanned;

#[derive(Debug, PartialEq)]
pub enum Error {
    UndefinedInclude {
        name: String,
        span: (usize, usize),
    },
    DuplicateUse {
        name: String,
        span: (usize, usize),
    },
    CyclicIncludes {
        cycle: Vec<Spanned<String>>,
    },
    IncompleteDeployment {
        name: String,
        span: (usize, usize),
        missing_pkgs: Vec<Spanned<String>>,
        soft: bool,
    },
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

    pub fn cyclic_includes(cycle: Vec<Spanned<String>>) -> Self {
        Self::CyclicIncludes { cycle }
    }

    pub fn incomplete_deployment(
        deployment: &Spanned<String>,
        missing_pkgs: Vec<Spanned<String>>,
        soft: bool,
    ) -> Self {
        Self::IncompleteDeployment {
            name: deployment.get_ref().into(),
            span: deployment.span(),
            missing_pkgs,
            soft,
        }
    }

    pub fn span(&self) -> (usize, usize) {
        match self {
            Self::DuplicateUse { span, .. }
            | Self::UndefinedInclude { span, .. }
            | Self::IncompleteDeployment { span, .. } => *span,
            Self::CyclicIncludes { cycle } => {
                let base = cycle.first().unwrap();
                base.span()
            }
        }
    }

    pub fn msg(&self) -> String {
        format!("{}", self)
    }

    pub fn reasons(&self) -> Vec<(usize, usize, String)> {
        if let Self::CyclicIncludes { cycle } = self {
            cycle
                .iter()
                .map(|x| (x.start(), x.end(), x.get_ref().into()))
                .collect::<Vec<_>>()
        } else {
            vec![]
        }
    }
}
impl Display for Error {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        match self {
            Self::CyclicIncludes { cycle } => {
                write!(f, "Circular dependency detected: ")?;
                for elem in cycle.iter() {
                    write!(f, "{} -> ", elem.get_ref())?;
                }
                write!(f, "{}", cycle[0].get_ref())?;
            }
            Self::UndefinedInclude { name, .. } => {
                write!(f, "Undefined package: {}", name)?;
            }
            Self::DuplicateUse { name, .. } => {
                write!(f, "This module can only be used in one package: {}", name)?;
            }
            Self::IncompleteDeployment {
                name,
                missing_pkgs,
                soft,
                ..
            } => {
                let soft_str = if *soft { "soft-" } else { "" };
                write!(
                    f,
                    "{} must {}deploy all nested {}included packages. Missing ",
                    name, soft_str, soft_str
                )?;
                for (i, pkg) in missing_pkgs.iter().enumerate() {
                    if i == missing_pkgs.len() - 1 {
                        write!(f, "{}", pkg.get_ref())?;
                    } else {
                        write!(f, "{}, ", pkg.get_ref())?;
                    }
                }
            }
        };
        Ok(())
    }
}
