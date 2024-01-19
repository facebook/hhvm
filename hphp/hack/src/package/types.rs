// Copyright (c) Meta, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::Deref;
use std::ops::DerefMut;

use hash::IndexMap;
use hash::IndexSet;
use once_cell::sync::Lazy;
use serde::Deserialize;
use toml::Spanned;
// Preserve the order for ease of testing
// Alternatively, we could use HashMap for performance
pub type PackageMap = IndexMap<Spanned<String>, Package>;
pub type DeploymentMap = IndexMap<Spanned<String>, Deployment>;

#[derive(Debug, Default, Deserialize, Clone, Eq, PartialEq)]
pub struct NameSet(IndexSet<Spanned<String>>);

#[derive(Debug, Deserialize, Clone, PartialEq, Eq)]
pub struct Package {
    pub uses: Option<NameSet>,
    pub includes: Option<NameSet>,
    pub soft_includes: Option<NameSet>,
    pub allow_directories: Option<NameSet>,
}

#[derive(Debug, Deserialize)]
pub struct Deployment {
    pub packages: Option<NameSet>,
    pub soft_packages: Option<NameSet>,
    pub domains: Option<NameSet>,
}

impl<'a> Default for &'a NameSet {
    fn default() -> &'a NameSet {
        static SET: Lazy<NameSet> = Lazy::new(|| NameSet(IndexSet::default()));
        &SET
    }
}
impl Deref for NameSet {
    type Target = IndexSet<Spanned<String>>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}
impl DerefMut for NameSet {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}
impl FromIterator<Spanned<String>> for NameSet {
    fn from_iter<I>(iter: I) -> Self
    where
        I: IntoIterator<Item = Spanned<String>>,
    {
        let mut set = IndexSet::default();
        for name in iter {
            set.insert(name);
        }
        NameSet(set)
    }
}
impl Iterator for NameSet {
    type Item = Spanned<String>;

    fn next(&mut self) -> Option<Self::Item> {
        self.0.iter().next().cloned()
    }
}
impl NameSet {
    pub fn take(&mut self, value: &Spanned<String>) -> Option<Spanned<String>> {
        self.0.take(value)
    }
}
