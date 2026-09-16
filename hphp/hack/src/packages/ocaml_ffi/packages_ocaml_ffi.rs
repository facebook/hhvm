// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_ocamlpool::ocaml_ffi;
use oxidized::package_info::PackageInfo;
use oxidized::package_info_impl::Errors;
use oxidized::package_info_impl::package_info_to_oxidized;

ocaml_ffi! {
    fn extract_packages_from_text_strict_ffi(
        filename: String,
        enable_implicit_packages: bool,
    ) -> Result<PackageInfo, Errors> {
        let info = match packages::PackageInfo::from_text_strict(enable_implicit_packages, &filename) {
            Ok(info) => info,
            // TODO(T148525961): Send a proper error when packages.toml fails to parse
            Err(_) => return Ok(PackageInfo::default()),
        };
        package_info_to_oxidized(&filename, info)
    }

    fn extract_packages_from_text_non_strict_ffi(
        filename: String,
        enable_implicit_packages: bool,
    ) -> Result<PackageInfo, Errors> {
        let info = match packages::PackageInfo::from_text_non_strict(enable_implicit_packages, &filename) {
            Ok(info) => info,
            // TODO(T148525961): Send a proper error when packages.toml fails to parse
            Err(_) => return Ok(PackageInfo::default()),
        };
        package_info_to_oxidized(&filename, info)
    }
}
