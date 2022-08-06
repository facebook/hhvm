// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]
#![allow(non_snake_case)] // e.g. Use `A` for hack `class A`.

use std::collections::BTreeSet;

use anyhow::Result;
use depgraph_api::DeclName;
use depgraph_api::DependencyName;
use maplit::btreemap;
use maplit::btreeset;
use pos::TypeName;

use crate::FacebookInit;
use crate::TestContext;

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
    let depgraph = &ctx.dependency_graph;

    // Doing the comparisons on binary search trees avoids issues with hash
    // map/set orderings.
    let expected = btreemap! {
        DependencyName::Extends(A) => btreeset!{DeclName::Type(T).hash1(), DeclName::Type(B).hash1()},
        DependencyName::Extends(T) => btreeset!{DeclName::Type(B).hash1()},
        DependencyName::Extends(I) => btreeset!{DeclName::Type(B).hash1(), DeclName::Type(T).hash1()},
    };
    for name in expected.keys() {
        let act = depgraph.get_dependents(*name).collect::<BTreeSet<_>>();
        let exp = (expected.get(name).unwrap().iter().copied()).collect::<BTreeSet<_>>();
        assert_eq!(act, exp);
    }

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
    let depgraph = &ctx.dependency_graph;

    // Doing the comparisons on binary search trees avoids issues with hash
    // map/set orderings.
    let exp = btreeset! {DeclName::Type(B).hash1()};
    let act = depgraph
        .get_dependents(DependencyName::Constructor(A))
        .collect::<BTreeSet<_>>();
    assert_eq!(exp, act);

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
    let Exception = TypeName::new(r#"\ExceptionA"#);

    // Fold `A`.
    ctx.folded_decl_provider.get_class(A.into(), A)?;
    // Retrieve the dependency graph.
    let depgraph = &ctx.dependency_graph;

    // The constructor relation of child on parent isn't observed when parent is
    // an hhi.
    assert!(
        depgraph
            .get_dependents(DependencyName::Constructor(Exception))
            .peekable()
            .peek()
            .is_none()
    );

    Ok(())
}
