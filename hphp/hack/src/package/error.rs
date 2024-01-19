// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fmt::Display;
use std::fmt::Formatter;
use std::fmt::Result;
use std::ops::Range;
use std::path::PathBuf;

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
    IncompleteDeployment {
        name: String,
        span: (usize, usize),
        missing_pkgs: Vec<Spanned<String>>,
        soft: bool,
    },
    InvalidAllowDirectory {
        abs_path: PathBuf,
        span: (usize, usize),
    },
}

impl Error {
    pub fn undefined_package(x: &Spanned<String>) -> Self {
        let Range { start, end } = x.span();
        Self::UndefinedInclude {
            name: x.get_ref().into(),
            span: (start, end),
        }
    }

    pub fn duplicate_use(x: &Spanned<String>) -> Self {
        let Range { start, end } = x.span();
        Self::DuplicateUse {
            name: x.get_ref().into(),
            span: (start, end),
        }
    }

    pub fn invalid_allow_directory(abs_path: PathBuf, span: Range<usize>) -> Self {
        let Range { start, end } = span;
        Self::InvalidAllowDirectory {
            abs_path,
            span: (start, end),
        }
    }

    pub fn incomplete_deployment(
        deployment: &Spanned<String>,
        missing_pkgs: Vec<Spanned<String>>,
        soft: bool,
    ) -> Self {
        let Range { start, end } = deployment.span();
        Self::IncompleteDeployment {
            name: deployment.get_ref().into(),
            span: (start, end),
            missing_pkgs,
            soft,
        }
    }

    pub fn span(&self) -> (usize, usize) {
        match self {
            Self::DuplicateUse { span, .. }
            | Self::UndefinedInclude { span, .. }
            | Self::IncompleteDeployment { span, .. }
            | Self::InvalidAllowDirectory { span, .. } => *span,
        }
    }

    pub fn msg(&self) -> String {
        format!("{}", self)
    }

    pub fn reasons(&self) -> Vec<(usize, usize, String)> {
        // Might need reasons later for more complicated error messages
        vec![]
    }
}
impl Display for Error {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        match self {
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
            Self::InvalidAllowDirectory { abs_path, .. } => {
                write!(f, "allow_directory {} does not exist", abs_path.display())?;
            }
        };
        Ok(())
    }
}
