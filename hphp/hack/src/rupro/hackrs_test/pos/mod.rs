// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]
use anyhow::Result;

use ::pos::BPos;
use ::pos::RelativePath;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use oxidized::file_pos_large::FilePosLarge;
use oxidized::relative_path::Prefix;

#[test]
fn bpos_from_ocamlrep() -> Result<()> {
    // make a pos
    let file = RelativePath::new(Prefix::Root, std::path::Path::new("yellow/brick/road"));
    let begin = FilePosLarge::from_line_column_offset(0usize, 0usize, 0usize);
    let until = FilePosLarge::from_line_column_offset(10usize, 0usize, 1024usize);
    let pos = BPos::new(file, begin, until);
    // convert it to an ocamlrep and back
    let alloc = &ocamlrep::Arena::new();
    let word: usize = pos.to_ocamlrep(alloc).to_bits();
    let value: ocamlrep::Value<'_> = unsafe { ocamlrep::Value::from_bits(word) };
    // check it round-tripped
    assert_eq!(pos, BPos::from_ocamlrep(value).unwrap());

    Ok(())
}
