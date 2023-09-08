// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::Range;
use std::path::PathBuf;
use std::sync::Arc;

use rc_pos::Pos;
use relative_path::Prefix;
use relative_path::RelativePath;
use toml::Spanned;

use crate::gen::package::Package;
use crate::gen::package::PosId;
use crate::gen::package_info::PackageInfo;
use crate::manual::s_map::SMap;

pub type Errors = Vec<(Pos, String, Vec<(Pos, String)>)>;

impl Default for PackageInfo {
    fn default() -> Self {
        Self {
            glob_to_package: SMap::default(),
            existing_packages: SMap::default(),
        }
    }
}

pub fn package_info_to_vec(
    filename: &str,
    info: package::PackageInfo,
) -> Result<Vec<Package>, Errors> {
    let pos_from_span = |span: (usize, usize)| {
        let (start_offset, end_offset) = span;
        let start_lnum = info.line_number(start_offset);
        let start_bol = info.beginning_of_line(start_lnum);
        let end_lnum = info.line_number(end_offset);
        let end_bol = info.beginning_of_line(end_lnum);

        Pos::from_lnum_bol_offset(
            Arc::new(RelativePath::make(Prefix::Dummy, PathBuf::from(filename))),
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
                let Range { start, end } = x.span();
                let pos = pos_from_span((start, end));
                let id = x.to_owned().into_inner();
                PosId(pos, id)
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
    Ok(packages)
}

impl TryFrom<package::PackageInfo> for PackageInfo {
    type Error = Errors;
    fn try_from(info: package::PackageInfo) -> Result<Self, Errors> {
        let result = package_info_to_vec("PACKAGES.toml", info);
        match result {
            Ok(packages) => Ok(PackageInfo {
                glob_to_package: packages
                    .iter()
                    .flat_map(|package| package.uses.iter().map(|u| (u.1.clone(), package.clone())))
                    .collect(),
                existing_packages: packages
                    .into_iter()
                    .map(|package| (package.name.1.clone(), package))
                    .collect(),
            }),
            Err(err) => Err(err),
        }
    }
}
