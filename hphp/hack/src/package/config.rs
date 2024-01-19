// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::VecDeque;

use hash::HashSet;
use serde::Deserialize;
use toml::Spanned;

use crate::error::Error;
use crate::types::DeploymentMap;
use crate::types::NameSet;
pub use crate::types::PackageMap;

#[derive(Debug, Deserialize)]
pub struct Config {
    pub packages: PackageMap,
    pub deployments: Option<DeploymentMap>,
}
impl Config {
    pub fn check_config(&self) -> Vec<Error> {
        let check_packages_are_defined =
            |errors: &mut Vec<Error>, pkgs: &Option<NameSet>, soft_pkgs: &Option<NameSet>| {
                if let Some(packages) = pkgs {
                    packages.iter().for_each(|package| {
                        if !self.packages.contains_key(package) {
                            errors.push(Error::undefined_package(package))
                        }
                    })
                }
                if let Some(packages) = soft_pkgs {
                    packages.iter().for_each(|package| {
                        if !self.packages.contains_key(package) {
                            errors.push(Error::undefined_package(package))
                        }
                    })
                }
            };
        let mut used_globs = NameSet::default();
        let mut check_each_glob_is_used_once =
            |errors: &mut Vec<Error>, globs: &Option<NameSet>| {
                if let Some(l) = globs {
                    l.iter().for_each(|glob| {
                        if used_globs.contains(glob) {
                            errors.push(Error::duplicate_use(glob))
                        }
                        used_globs.insert(glob.clone());
                    })
                }
            };
        let check_deployed_packages_are_transitively_closed =
            |errors: &mut Vec<Error>,
             deployment: &Spanned<String>,
             pkgs: &Option<NameSet>,
             soft_pkgs: &Option<NameSet>| {
                let deployed = pkgs.as_ref().unwrap_or_default();
                let soft_deployed = soft_pkgs.as_ref().unwrap_or_default();
                let (missing_pkgs, missing_soft_pkgs) =
                    find_missing_packages_from_deployment(&self.packages, deployed, soft_deployed);
                if !missing_pkgs.is_empty() {
                    errors.push(Error::incomplete_deployment(
                        deployment,
                        missing_pkgs,
                        false,
                    ));
                }
                if !missing_soft_pkgs.is_empty() {
                    errors.push(Error::incomplete_deployment(
                        deployment,
                        missing_soft_pkgs,
                        true,
                    ));
                }
            };
        let mut errors = vec![];
        for (_, package) in self.packages.iter() {
            check_packages_are_defined(&mut errors, &package.includes, &package.soft_includes);
            check_each_glob_is_used_once(&mut errors, &package.uses);
        }
        if let Some(deployments) = &self.deployments {
            for (positioned_name, deployment) in deployments.iter() {
                check_packages_are_defined(
                    &mut errors,
                    &deployment.packages,
                    &deployment.soft_packages,
                );
                check_deployed_packages_are_transitively_closed(
                    &mut errors,
                    positioned_name,
                    &deployment.packages,
                    &deployment.soft_packages,
                );
            }
        };
        errors
    }
}

// The function takes a starting set of package names and a PackageMap, and
// returns two HashSets: one with packages included by the starting set (transitive closure),
// and the other with packages soft-included by the starting set (transitive closure) but not included.
fn analyze_includes<'a>(
    starting_set: &'a NameSet,
    package_map: &'a PackageMap,
) -> (HashSet<&'a Spanned<String>>, HashSet<&'a Spanned<String>>) {
    // Sets to store packages that are included and soft-included, respectively
    let mut included = HashSet::default();
    let mut soft_included = HashSet::default();

    // Queue of package names and whether the package is soft included
    let mut queue = VecDeque::new();

    // Add the starting set of package names to the queue, with 'is_soft' flag set to false
    for package_name in starting_set.iter() {
        queue.push_back((package_name, false));
    }

    while let Some((current_package_name, is_soft)) = queue.pop_front() {
        if let Some(package) = package_map.get(current_package_name) {
            // If the package is not soft-included, add it to the 'included' set
            if !is_soft {
                included.insert(current_package_name);
            } else if !included.contains(current_package_name) {
                // If the package is soft-included and not in the 'included' set, add it to the 'soft_included' set
                soft_included.insert(current_package_name);
            }

            if let Some(ref includes) = package.includes {
                for include in includes.iter() {
                    if !included.contains(include) {
                        queue.push_back((include, false));
                    }
                }
            }

            if let Some(ref soft_includes) = package.soft_includes {
                for soft_include in soft_includes.iter() {
                    if !included.contains(soft_include) && !soft_included.contains(soft_include) {
                        queue.push_back((soft_include, true));
                    }
                }
            }
        }
    }
    (included, soft_included)
}

fn find_missing_packages_from_deployment(
    package_map: &PackageMap,
    deployed: &NameSet,
    soft_deployed: &NameSet,
) -> (Vec<Spanned<String>>, Vec<Spanned<String>>) {
    // Taking the transitive closure of all nested included packages so that the user
    // could complete the deployment set upon the first error message they receive, as
    // opposed to iterating upon it over multiple checks and going through a full init
    // every time they update the config.
    // TODO: simplify after incremental mode (T148526825)

    let (included, soft_included) = analyze_includes(deployed, package_map);
    let soft_or_regular_deployed = deployed.union(soft_deployed).cloned().collect();

    fn get_missing<'a>(
        included: &HashSet<&'a Spanned<String>>,
        deployed: &NameSet,
        package_map: &PackageMap,
    ) -> Vec<Spanned<String>> {
        let mut missing_pkgs = included
            .iter()
            .filter_map(|pkg| {
                let pkg_name = pkg.get_ref().as_str();
                if !deployed.contains(pkg_name) {
                    let (positioned_pkg_name, _) = package_map.get_key_value(pkg_name).unwrap();
                    Some(positioned_pkg_name.clone())
                } else {
                    None
                }
            })
            .collect::<Vec<_>>();
        missing_pkgs.sort();
        missing_pkgs
    }
    (
        get_missing(&included, deployed, package_map),
        get_missing(&soft_included, &soft_or_regular_deployed, package_map),
    )
}
