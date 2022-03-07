// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]

use crate::{FacebookInit, TestContext};
use anyhow::Result;
use hackrs::{decl_defs::shallow, folded_decl_provider::FoldedDeclProvider, reason::BReason};
use maplit::btreemap;
use pos::{Prefix, RelativePath, TypeName};
use std::fs;

#[fbinit::test]
fn test_file_missing_error(fb: FacebookInit) -> Result<()> {
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
        .parse::<BReason>(RelativePath::new(Prefix::Root, "d.php"))?
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
    use hackrs::folded_decl_provider::Error;
    match ctx.folded_decl_provider.get_class(d) {
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
