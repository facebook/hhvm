// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::dependency_registrar::{DeclName, DependencyName, DependencyRegistrar, Result};

/// A no-op implementation of the hackrs `DependencyRegistrar` trait.
/// All registered edges are thrown away.
#[derive(Debug, Clone)]
pub struct NoDepGraph(());

impl NoDepGraph {
    pub fn new() -> Self {
        Self(())
    }
}

impl DependencyRegistrar for NoDepGraph {
    fn add_dependency(&self, _dependent: DeclName, _dependency: DependencyName) -> Result<()> {
        Ok(())
    }
}
