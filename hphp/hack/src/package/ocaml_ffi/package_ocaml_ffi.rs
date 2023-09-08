// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_ocamlpool::ocaml_ffi;
use oxidized::package::Package;
use oxidized::package_info_impl::package_info_to_vec;
use oxidized::package_info_impl::Errors;

ocaml_ffi! {
    fn extract_packages_from_text_ffi(
        filename: String,
        source_text: String,
    ) -> Result<Vec<Package>, Errors> {
        let info = match package::PackageInfo::from_text(&source_text) {
            Ok(info) => info,
            // TODO(T148525961): Send a proper error when packages.toml fails to parse
            Err(_) => return Ok(vec![]),
        };
        package_info_to_vec(&filename, info)
    }
}
