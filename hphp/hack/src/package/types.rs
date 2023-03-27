// Copyright (c) Meta, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::IndexMap;
use hash::IndexSet;
use serde::Deserialize;
use toml::Spanned;

// Preserve the order for ease of testing
// Alternatively, we could use HashMap for performance
pub type PackageMap = IndexMap<Spanned<String>, Package>;
pub type DeploymentMap = IndexMap<Spanned<String>, Deployment>;
pub type NameSet = IndexSet<Spanned<String>>;

#[derive(Debug, Deserialize)]
pub struct Package {
    pub uses: Option<NameSet>,
    pub includes: Option<NameSet>,
    pub soft_includes: Option<NameSet>,
}

#[derive(Debug, Deserialize)]
pub struct Deployment {
    pub packages: Option<NameSet>,
    pub domains: Option<NameSet>,
}
