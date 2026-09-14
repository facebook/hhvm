/**
 * Copyright (c) Meta, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 */
use cxx::CxxString;

#[cxx::bridge(namespace = "HPHP::package")]
mod ffi {
    #[derive(Default)]
    struct PackageInfo {
        packages: Vec<PackageMapEntry>,
        deployments: Vec<DeploymentMapEntry>,
        implicit_package_families: Vec<ImplicitPackageFamilyMapEntry>,
        errors: Vec<String>,
    }
    struct PackageMapEntry {
        name: String,
        package: Package,
    }
    struct Package {
        includes: Vec<String>,
        soft_includes: Vec<String>,
        include_paths: Vec<String>,
        enable_strict_isolation: bool,
    }
    struct DeploymentMapEntry {
        name: String,
        deployment: Deployment,
    }
    struct Deployment {
        packages: Vec<String>,
        soft_packages: Vec<String>,
    }
    struct ImplicitPackageFamilyMapEntry {
        name: String,
        family: ImplicitPackageFamily,
    }
    struct ImplicitPackageFamily {
        path: String,
        includes: Vec<String>,
        soft_includes: Vec<String>,
    }
    extern "Rust" {
        pub fn package_info(
            packages_toml: &CxxString,
            enable_implicit_packages: bool,
        ) -> PackageInfo;
    }
}

pub fn package_info(packages_toml: &CxxString, enable_implicit_packages: bool) -> ffi::PackageInfo {
    // HHVM should not perform validation of include_paths, so invoking from_text_non_strict.
    let s = packages::PackageInfo::from_text_non_strict(
        enable_implicit_packages,
        &packages_toml.to_string(),
    );
    match s {
        Ok(info) => {
            let convert = |v: Option<&packages::NameSet>| {
                v.map(|v| v.iter().map(|v| v.get_ref().clone()).collect())
                    .unwrap_or_default()
            };
            let packages = info
                .packages()
                .iter()
                .map(|(name, package)| {
                    let package_ffi = ffi::Package {
                        includes: convert(package.includes.as_ref()),
                        soft_includes: convert(package.soft_includes.as_ref()),
                        include_paths: convert(package.include_paths.as_ref()),
                        enable_strict_isolation: package.enable_strict_isolation,
                    };
                    ffi::PackageMapEntry {
                        name: name.get_ref().to_string(),
                        package: package_ffi,
                    }
                })
                .collect();
            let deployments = info
                .deployments()
                .map(|deployments_unwrapped| {
                    deployments_unwrapped
                        .iter()
                        .map(|(name, deployment)| {
                            let deployment_ffi = ffi::Deployment {
                                packages: convert(deployment.packages.as_ref()),
                                soft_packages: convert(deployment.soft_packages.as_ref()),
                            };
                            ffi::DeploymentMapEntry {
                                name: name.get_ref().into(),
                                deployment: deployment_ffi,
                            }
                        })
                        .collect()
                })
                .unwrap_or_default();
            let implicit_package_families = info
                .implicit_packages()
                .iter()
                .map(|(name, family)| ffi::ImplicitPackageFamilyMapEntry {
                    name: name.get_ref().into(),
                    family: ffi::ImplicitPackageFamily {
                        path: family.path.get_ref().into(),
                        includes: convert(family.includes.as_ref()),
                        soft_includes: convert(family.soft_includes.as_ref()),
                    },
                })
                .collect();
            let errors = info.errors().iter().map(|e| e.msg()).collect();
            ffi::PackageInfo {
                packages,
                deployments,
                implicit_package_families,
                errors,
            }
        }
        Err(_e) => ffi::PackageInfo::default(),
    }
}
