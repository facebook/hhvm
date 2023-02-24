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
    struct PackageInfo {
        packages: Vec<PackageMapEntry>,
    }
    struct PackageMapEntry {
        name: String,
        package: Package,
    }
    struct Package {
        uses: Vec<String>,
        includes: Vec<String>,
    }
    extern "Rust" {
        pub fn package_info_cpp_ffi(source_text: &CxxString) -> PackageInfo;
    }
}

pub fn package_info_cpp_ffi(source_text: &CxxString) -> ffi::PackageInfo {
    let info = package::PackageInfo::from_text(&source_text.to_string()).unwrap();
    let packages = info
        .packages()
        .iter()
        .map(|(name, package)| {
            let convert = |v: Option<&Vec<toml::Spanned<String>>>| {
                v.map(|v| v.iter().map(|v| v.get_ref().clone()).collect())
                    .unwrap_or_default()
            };
            let package_ffi = ffi::Package {
                uses: convert(package.uses.as_ref()),
                includes: convert(package.includes.as_ref()),
            };
            ffi::PackageMapEntry {
                name: name.to_string(),
                package: package_ffi,
            }
        })
        .collect();
    ffi::PackageInfo { packages }
}
