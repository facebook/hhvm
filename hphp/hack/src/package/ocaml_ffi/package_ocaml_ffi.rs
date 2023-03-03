// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::ToOcamlRep;
use ocamlrep_ocamlpool::ocaml_ffi;
use ocamlrep_ocamlpool::to_ocaml;

#[derive(ToOcamlRep)]
struct Package {
    name: String,
    uses: Vec<String>,
    includes: Vec<String>,
}

ocaml_ffi! {
    fn extract_packages_from_text_ffi(source_text: String) -> UnsafeOcamlPtr {
        let info = package::PackageInfo::from_text(&source_text).unwrap();
        let packages : Vec<Package> = info
        .packages()
        .iter()
        .map(|(name, package)| {
            let convert = |v: Option<&Vec<toml::Spanned<String>>>| {
                v.map(|v| v.iter().map(|v| v.get_ref().clone()).collect())
                    .unwrap_or_default()
            };
            Package {
                name: name.into(),
                uses: convert(package.uses.as_ref()),
                includes: convert(package.includes.as_ref()),
            }

        })
        .collect();
        unsafe { UnsafeOcamlPtr::new(to_ocaml(packages.as_slice())) }
    }
}
