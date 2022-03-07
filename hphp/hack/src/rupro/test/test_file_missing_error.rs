// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]

use crate::{FacebookInit, TestContext};
use anyhow::Result;
use hackrs::{decl_defs::shallow, folded_decl_provider::FoldedDeclProvider, reason::NReason};
use maplit::btreemap;
use pos::{Prefix, RelativePath, TypeName};

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

    // check we can decl parse 'd.php'
    let mut saw_err = false;
    for decl in ctx
        .decl_parser
        .parse::<NReason>(RelativePath::new(Prefix::Root, "d.php"))?
    {
        match decl {
            shallow::Decl::Class(TypeName(cls), _decl) => {
                assert_eq!(cls.as_ref(), r#"\D"#);
            }
            _ => saw_err = true,
        }
    }
    assert!(
        !saw_err,
        "something unexpected happened decl parsing 'D.php'"
    );

    // check we can get a folded decl for 'D'
    assert!(
        ctx.folded_decl_provider
            .get_class(TypeName::new(r#"\D"#))
            .is_ok(),
        "failure getting folded decl 'D' "
    );

    // TODO: implement the logic of 'decl_error_test.sh' here!

    Ok(())
}
