// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;

use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::rc::RcOc;
use ocamlrep::ToOcamlRep;
use ocamlrep_ocamlpool::ocaml_ffi;
use ocamlrep_ocamlpool::to_ocaml;
use rc_pos::Pos;
use relative_path::Prefix;
use relative_path::RelativePath;
use toml::Spanned;

type PosId = (Pos, String);
type Errors = Vec<(Pos, String)>;

#[derive(ToOcamlRep)]
struct Package {
    name: PosId,
    uses: Vec<PosId>,
    includes: Vec<PosId>,
}

ocaml_ffi! {
    fn extract_packages_from_text_ffi(filename: String, source_text: String) -> Result<UnsafeOcamlPtr, Errors> {
        let info = package::PackageInfo::from_text(&source_text).unwrap();
        let pos_from_span = |span: (usize, usize)| {
            let (start_offset, end_offset) = span;
            let start_lnum = info.line_number(start_offset);
            let start_bol = info.beginning_of_line(start_lnum);
            let end_lnum = info.line_number(end_offset);
            let end_bol = info.beginning_of_line(end_lnum);

            Pos::from_lnum_bol_offset(
                RcOc::new(RelativePath::make(
                    Prefix::Dummy,
                    PathBuf::from(filename.clone()),
                )),
                (start_lnum, start_bol, start_offset),
                (end_lnum, end_bol, end_offset),
            )
        };
        let errors = info.errors().iter().map(|e| {
            let pos = pos_from_span(e.span());
            let msg = e.msg();
            (pos, msg)
        }).collect::<Errors>();
        if !errors.is_empty() {
            return Err(errors);
        };
        let packages: Vec<Package> = info
            .packages()
            .iter()
            .map(|(name, package)| {
                let name = name.clone();
                let uses = package.uses.clone().unwrap_or_default();
                let includes = package.includes.clone().unwrap_or_default();

                let convert = |x: Spanned<String>| -> PosId {
                    let pos = pos_from_span(x.span());
                    let id = x.into_inner();
                    (pos, id)
                };
                let convert_many = |xs: package::NameSet| -> Vec<PosId> {
                    xs.into_iter().map(convert).collect()
                };

                Package {
                    name: convert(name),
                    uses: convert_many(uses),
                    includes: convert_many(includes),
                }
            })
            .collect();
        Ok(unsafe { UnsafeOcamlPtr::new(to_ocaml(packages.as_slice())) })
    }
}
