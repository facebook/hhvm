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
type Errors = Vec<(Pos, String, Vec<(Pos, String)>)>;

#[derive(ToOcamlRep)]
struct Package {
    name: PosId,
    uses: Vec<PosId>,
    includes: Vec<PosId>,
    soft_includes: Vec<PosId>,
}

ocaml_ffi! {
    fn extract_packages_from_text_ffi(
        filename: String,
        source_text: String,
    ) -> Result<UnsafeOcamlPtr, Errors> {
        let s = package::PackageInfo::from_text(&source_text);
        match s {
            Ok(info) => {
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
                let errors = info
                    .errors()
                    .iter()
                    .map(|e| {
                        let pos = pos_from_span(e.span());
                        let msg = e.msg();
                        let reasons = e
                            .reasons()
                            .into_iter()
                            .map(|(start, end, reason)| (pos_from_span((start, end)), reason))
                            .collect();
                        (pos, msg, reasons)
                    })
                    .collect::<Errors>();
                if !errors.is_empty() {
                    return Err(errors);
                };
                let packages: Vec<Package> = info
                    .packages()
                    .iter()
                    .map(|(name, package)| {
                        let convert = |x: &Spanned<String>| -> PosId {
                            let pos = pos_from_span(x.span());
                            let id = x.to_owned().into_inner();
                            (pos, id)
                        };
                        let convert_many = |xs: &Option<package::NameSet>| -> Vec<PosId> {
                            xs.as_ref()
                                .unwrap_or_default()
                                .iter()
                                .map(convert)
                                .collect()
                        };

                        Package {
                            name: convert(name),
                            uses: convert_many(&package.uses),
                            includes: convert_many(&package.includes),
                            soft_includes: convert_many(&package.soft_includes),
                        }
                    })
                    .collect();
                Ok(unsafe { UnsafeOcamlPtr::new(to_ocaml(packages.as_slice())) })
            },
            Err(_e) => {
                // TODO(T148525961): Send a proper error when packages.toml fails to parse
                Err(vec![])
            }
        }
    }
}
