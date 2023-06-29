// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::gen::package_info::PackageInfo;
use crate::manual::s_map::SMap;

impl Default for PackageInfo {
    fn default() -> Self {
        Self {
            glob_to_package: SMap::default(),
            existing_packages: SMap::default(),
        }
    }
}
