// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]

use std::fs;

use anyhow::Result;
use maplit::btreemap;
use pos::Prefix;
use pos::RelativePath;
use pos::TypeName;
use ty::decl::shallow;
use ty::decl_error::DeclError;

use crate::FacebookInit;
use crate::TestContext;

#[fbinit::test]
fn when_cyclic_class_error(fb: FacebookInit) -> Result<()> {
    let ctx = TestContext::new(
        fb,
        btreemap! {
            "a.php" => "class A extends B {}",
            "b.php" => "class B extends A {}"
        },
    )?;
    let (a, b) = (TypeName::new(r#"\A"#), TypeName::new(r#"\B"#));
    // To declare B, we'll first declare A. During the declaring of A, the
    // dependency on B will be noted as a cycle in A's errors.
    ctx.folded_decl_provider.get_class(b.into(), b)?;
    // Since we already declared A incidentally above, this next line will
    // simply pull it from cache.
    let decl = ctx.folded_decl_provider.get_class(a.into(), a)?.unwrap();
    // Now check that A has recorded the cyclic class error as we predict.
    match decl.decl_errors.first().unwrap() {
        DeclError::CyclicClassDef(_, ts) => {
            itertools::assert_equal(ts.iter().copied(), [b, a].into_iter())
        }
        _ => panic!(),
    };

    Ok(())
}

#[fbinit::test]
fn results_stable(fb: FacebookInit) -> Result<()> {
    // Our use of `index_map::IndexMap` in strategic places implies folded class
    // maps are stable.
    for _ in 1..5 {
        let ctx = TestContext::new(
            fb,
            btreemap! {
                "a.php" => "class A {}",
                "b.php" => "class B extends A {}",
                "c.php" => "class C extends B {}",
                "d.php" => "class D extends C {}",
            },
        )?;
        let (a, b, c, d) = (
            TypeName::new(r#"\A"#),
            TypeName::new(r#"\B"#),
            TypeName::new(r#"\C"#),
            TypeName::new(r#"\D"#),
        );
        let decl = ctx.folded_decl_provider.get_class(d.into(), d)?.unwrap();
        itertools::assert_equal(decl.ancestors.keys().copied(), [a, b, c].into_iter())
    }

    Ok(())
}

#[fbinit::test]
fn when_file_missing_error(fb: FacebookInit) -> Result<()> {
    let ctx = TestContext::new(
        fb,
        btreemap! {
            "a.php" => "class A {}",
            "b.php" => "class B extends A {}",
            "c.php" => "class C extends B {}",
            "d.php" => "class D extends C {}",
        },
    )?;

    let (a, b, c, d) = (
        TypeName::new(r#"\A"#),
        TypeName::new(r#"\B"#),
        TypeName::new(r#"\C"#),
        TypeName::new(r#"\D"#),
    );

    // check we can decl parse 'd.php'
    for decl in ctx
        .decl_parser
        .parse(RelativePath::new(Prefix::Root, "d.php"))?
    {
        match decl {
            shallow::Decl::Class(cls, _) => {
                assert_eq!(cls, d);
            }
            _ => panic!("unexpected decl in 'd.php'"),
        }
    }

    // remove 'a.php'
    fs::remove_file(ctx.root.path().join("a.php").as_path())?;

    // try getting a folded decl for 'D'
    use ::folded_decl_provider::Error;
    match ctx.folded_decl_provider.get_class(d.into(), d) {
        Err(
            ref err @ Error::Parent {
                ref class,
                ref parents,
                ..
            },
        ) => {
            // check the error is about 'D'
            assert_eq!(*class, d);
            // check we enumerated all 'D's parents
            assert!([&a, &b, &c].iter().all(|p| parents.contains(p)));
            // check the error text
            assert_eq!(
                format!("{}", err),
                "Failed to declare \\D because of error in ancestor \\A (via \\C, \\B, \\A): Failed to parse decls in root|a.php: No such file or directory (os error 2)"
            )
        }
        _ => panic!("failure folding 'D' expected"),
    }

    Ok(())
}
