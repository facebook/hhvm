// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]
#![allow(non_snake_case)] // e.g. Use `A` for hack `class A`.

use crate::{FacebookInit, TestContext};
use anyhow::Result;
use hackrs::dependency_registrar::{DeclName, DependencyName};
use hackrs_test_utils::registrar::DependencyGraph;
use maplit::{btreemap, btreeset};
use pos::TypeName;

#[fbinit::test]
fn extends_relation(fb: FacebookInit) -> Result<()> {
    let ctx = TestContext::new(
        fb,
        btreemap! {"extends_dependency_registration.php" =>r#"
class A {}

interface I {}

trait T  {
  require extends A;
  require implements I;
}

class B extends A implements I {
  use T;
}
"#,
        },
    )?;
    let (T, I, A, B) = (
        TypeName::new(r#"\T"#),
        TypeName::new(r#"\I"#),
        TypeName::new(r#"\A"#),
        TypeName::new(r#"\B"#),
    );

    // Fold `B`.
    ctx.folded_decl_provider.get_class(B.into(), B)?;
    // Retrieve the dependency graph.
    let depgraph: &DependencyGraph = &ctx.dependency_graph;
    // Doing the comparisons on binary search trees avoids issues with hash
    // map/set orderings.
    let expected = btreemap! {
        DependencyName::Extends(A) => btreeset!{DeclName::Type(T), DeclName::Type(B)},
        DependencyName::Extends(T) => btreeset!{DeclName::Type(B)},
        DependencyName::Extends(I) => btreeset!{DeclName::Type(B), DeclName::Type(T)},
    };
    let actual = (depgraph.rdeps.iter())
        .map(|e| (*e.key(), e.value().iter().copied().collect()))
        .filter(|(k, _)| matches!(k, DependencyName::Extends(..)))
        .collect();
    // Finally, compare.
    assert_eq!(expected, actual);

    Ok(())
}

#[fbinit::test]
fn constructor_relation(fb: FacebookInit) -> Result<()> {
    let ctx = TestContext::new(
        fb,
        btreemap! {
            "a.php" => "class A {  public function __construct(int $x = 0)[] {} }",
            "b.php" => "class B extends A {}",
        },
    )?;
    let (A, B) = (TypeName::new(r#"\A"#), TypeName::new(r#"\B"#));

    // Fold `B`.
    ctx.folded_decl_provider.get_class(B.into(), B)?;
    // Retrieve the dependency graph.
    let depgraph: &DependencyGraph = &ctx.dependency_graph;
    // Doing the comparisons on binary search trees avoids issues with hash
    // map/set orderings.
    let expected = btreemap! {
        DependencyName::Constructor(A) => btreeset!{DeclName::Type(B)},
    };
    let actual: std::collections::BTreeMap<DependencyName, std::collections::BTreeSet<DeclName>> =
        (depgraph.rdeps.iter())
            .map(|e| (*e.key(), e.value().iter().copied().collect()))
            .filter(|(k, _)| matches!(k, DependencyName::Constructor(..)))
            .collect();
    // Finally, compare.
    assert_eq!(expected, actual);

    Ok(())
}

#[fbinit::test]
fn no_constructor_relation_on_hhi_parent(fb: FacebookInit) -> Result<()> {
    // This test assumes `BReason` mode.
    let ctx = TestContext::new(
        fb,
        btreemap! {
            "no_constructor_relation_on_hhi_parent.php" => "class A extends Exception {}"
        },
    )?;
    let A = TypeName::new(r#"\A"#);

    // Fold `A`.
    ctx.folded_decl_provider.get_class(A.into(), A)?;
    // Retrieve the dependency graph.
    let depgraph: &DependencyGraph = &ctx.dependency_graph;
    // Doing the comparisons on binary search trees avoids issues with hash
    // map/set orderings (not that it matters here).
    let actual: std::collections::BTreeMap<DependencyName, std::collections::BTreeSet<DeclName>> =
        (depgraph.rdeps.iter())
            .map(|e| (*e.key(), e.value().iter().copied().collect()))
            .filter(|(k, _)| matches!(k, DependencyName::Constructor(..)))
            .collect();
    // The constructor relation of child on parent isn't observed when parent is
    // an hhi.
    assert!(actual.is_empty());

    Ok(())
}
