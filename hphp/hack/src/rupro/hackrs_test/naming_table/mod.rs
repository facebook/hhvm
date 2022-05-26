// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]

use crate::{FacebookInit, TestContext};
use anyhow::Result;
use hh24_types::ToplevelSymbolHash;
use maplit::btreemap;
use pos::{Prefix, RelativePath};
use rpds::HashTrieSet;
use std::path::PathBuf;

fn make_dep_from_typename(typename: &str) -> deps_rust::Dep {
    deps_rust::Dep::new(ToplevelSymbolHash::from_type(typename).as_u64())
}

fn make_relative_path_from_str(test_path: &str) -> RelativePath {
    RelativePath::new(Prefix::Root, PathBuf::from(test_path))
}

#[fbinit::test]
fn get_filenames_by_hash_test(fb: FacebookInit) -> Result<()> {
    let ctx = TestContext::new(
        fb,
        btreemap! {
            "a.php" => "class A extends B {}",
            "b.php" => "class B {}"
        },
    )?;
    let naming_table = ctx.provider_backend.naming_table;
    let a_type = make_dep_from_typename(r#"\A"#);
    let b_type = make_dep_from_typename(r#"\B"#);
    let a_relative_path = make_relative_path_from_str("a.php");
    let b_relative_path = make_relative_path_from_str("b.php");

    let depset = HashTrieSet::new();
    let depset = depset.insert(a_type);
    let dep_paths = naming_table
        .get_filenames_by_hash(&deps_rust::DepSet::from(depset))
        .unwrap();
    assert!(dep_paths.contains(&a_relative_path));
    assert!(!dep_paths.contains(&b_relative_path));

    let depset = HashTrieSet::new();
    let depset = depset.insert(a_type);
    let depset = depset.insert(b_type);
    let dep_paths = naming_table
        .get_filenames_by_hash(&deps_rust::DepSet::from(depset))
        .unwrap();
    assert!(dep_paths.contains(&a_relative_path));
    assert!(dep_paths.contains(&b_relative_path));

    let depset = HashTrieSet::new();
    let dep_paths = naming_table
        .get_filenames_by_hash(&deps_rust::DepSet::from(depset))
        .unwrap();
    assert!(dep_paths.is_empty());

    let depset = HashTrieSet::new();
    let depset = depset.insert(make_dep_from_typename(r#"\C"#));
    let dep_paths = naming_table
        .get_filenames_by_hash(&deps_rust::DepSet::from(depset))
        .unwrap();
    assert!(dep_paths.is_empty());
    Ok(())
}
