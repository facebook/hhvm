// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fmt::Display;
use std::fmt::Formatter;
use std::fmt::Result;
use std::hash::Hash;

use hash::HashMap;
use hash::HashSet;
use toml::Spanned;

use crate::Config;
use crate::NameSet;
use crate::Package;
use crate::PackageMap;

#[derive(Debug, PartialEq)]
pub enum Error {
    UndefinedInclude { name: String, span: (usize, usize) },
    DuplicateUse { name: String, span: (usize, usize) },
    CyclicIncludes { cycle: Vec<Spanned<String>> },
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

    pub fn span(&self) -> (usize, usize) {
        match self {
            Self::DuplicateUse { span, .. } | Self::UndefinedInclude { span, .. } => *span,
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
        };
        Ok(())
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
    let check_cyclic_package_deps = |errors: &mut Vec<Error>, pkgs: &PackageMap| {
        let cycles = find_cycles(pkgs);
        cycles
            .into_iter()
            .for_each(|cycle| errors.push(Error::cyclic_includes(cycle)));
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
    check_cyclic_package_deps(&mut errors, &config.packages);
    errors
}

fn find_cycles(packages: &PackageMap) -> HashSet<Vec<Spanned<String>>> {
    #[derive(Default)]
    struct State<Pkg> {
        path: Vec<Pkg>,
        package_location_in_path: HashMap<Pkg, usize>,
        stack: Vec<(Pkg, Option<Pkg>)>,
        visited: HashSet<Pkg>,
    }
    impl<Pkg: Eq + PartialEq + Hash> State<Pkg> {
        fn reset(&mut self, start_package: Option<Pkg>) {
            match start_package {
                None => {
                    self.path = vec![];
                    self.package_location_in_path = HashMap::default();
                }
                Some(start_package) => {
                    while let Some(last_package) = self.path.pop() {
                        if last_package != start_package {
                            self.package_location_in_path.remove(&last_package);
                        } else {
                            break;
                        }
                    }
                    self.path.push(start_package);
                }
            }
        }
    }
    let mut state = State::default();
    let mut cycles = HashSet::default();

    for package in packages.keys() {
        let start_package = package.get_ref().as_str();
        state.stack.push((start_package, None));
        while let Some((current_package, parent_package)) = state.stack.pop() {
            state.reset(parent_package);
            let idx = state.path.len();
            state.path.push(current_package);
            state.package_location_in_path.insert(current_package, idx);

            if let Some(Package {
                includes: Some(next_packages),
                ..
            }) = packages.get(current_package)
            {
                for next_package in next_packages {
                    let next_package = next_package.get_ref().as_str();
                    if let Some(idx) = state.package_location_in_path.get(next_package) {
                        let cycle = normalize_cycle(&state.path[*idx..], packages);
                        cycles.insert(cycle);
                    } else if !state.visited.contains(next_package) {
                        state.stack.push((next_package, Some(current_package)));
                    }
                }
            }
        }
        state.visited.insert(start_package);
    }

    cycles
}

fn normalize_cycle<'a>(cycle: &[&'a str], packages: &'a PackageMap) -> Vec<Spanned<String>> {
    let min_idx: Option<usize> = cycle
        .iter()
        .enumerate()
        .min_by_key(|(_, &val)| val)
        .map(|(idx, _)| idx);
    match min_idx {
        None => Vec::new(),
        Some(idx) => cycle[idx..]
            .iter()
            .chain(cycle[..idx].iter())
            .into_iter()
            .map(|&pkg_name| {
                let (positioned_pkg_name, _) = packages.get_key_value(pkg_name).unwrap();
                positioned_pkg_name.clone()
            })
            .collect(),
    }
}
