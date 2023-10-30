// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::Allocator;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;

use super::decl_error::*;

impl<P: ToOcamlRep> ToOcamlRep for DeclError<P> {
    fn to_ocamlrep<'a, A: Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        match self {
            Self::WrongExtendKind {
                pos,
                kind,
                name,
                parent_pos,
                parent_kind,
                parent_name,
            } => {
                let mut block = alloc.block_with_size_and_tag(6usize, 0u8);
                alloc.set_field(&mut block, 0, alloc.add(pos));
                alloc.set_field(&mut block, 1, alloc.add(kind));
                alloc.set_field(&mut block, 2, alloc.add(name));
                alloc.set_field(&mut block, 3, alloc.add(parent_pos));
                alloc.set_field(&mut block, 4, alloc.add(parent_kind));
                alloc.set_field(&mut block, 5, alloc.add(parent_name));
                block.build()
            }
            Self::WrongUseKind {
                pos,
                name,
                parent_pos,
                parent_name,
            } => {
                let mut block = alloc.block_with_size_and_tag(4usize, 1u8);
                alloc.set_field(&mut block, 0, alloc.add(pos));
                alloc.set_field(&mut block, 1, alloc.add(name));
                alloc.set_field(&mut block, 2, alloc.add(parent_pos));
                alloc.set_field(&mut block, 3, alloc.add(parent_name));
                block.build()
            }
            Self::CyclicClassDef(pos, stack) => {
                // The stack is an SSet rather than a list in OCaml, so we need
                // to construct a tree set here. One way is sorting the list and
                // passing it to `sorted_iter_to_ocaml_set`.
                let mut stack = stack.clone();
                stack.sort_unstable();
                stack.dedup();
                let mut iter = stack.iter().copied().map(|s| alloc.add(s.as_str()));
                let (stack, _) = ocamlrep::sorted_iter_to_ocaml_set(&mut iter, alloc, stack.len());

                let mut block = alloc.block_with_size_and_tag(2usize, 2u8);
                alloc.set_field(&mut block, 0, alloc.add(pos));
                alloc.set_field(&mut block, 1, stack);
                block.build()
            }
        }
    }
}

impl<P: FromOcamlRep> FromOcamlRep for DeclError<P> {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let block = ocamlrep::from::expect_block(value)?;
        match block.tag() {
            0 => {
                ocamlrep::from::expect_block_size(block, 6)?;
                Ok(Self::WrongExtendKind {
                    pos: ocamlrep::from::field(block, 0)?,
                    kind: ocamlrep::from::field(block, 1)?,
                    name: ocamlrep::from::field(block, 2)?,
                    parent_pos: ocamlrep::from::field(block, 3)?,
                    parent_kind: ocamlrep::from::field(block, 4)?,
                    parent_name: ocamlrep::from::field(block, 5)?,
                })
            }
            1 => {
                ocamlrep::from::expect_block_size(block, 4)?;
                Ok(Self::WrongUseKind {
                    pos: ocamlrep::from::field(block, 0)?,
                    name: ocamlrep::from::field(block, 1)?,
                    parent_pos: ocamlrep::from::field(block, 2)?,
                    parent_name: ocamlrep::from::field(block, 3)?,
                })
            }
            2 => {
                ocamlrep::from::expect_block_size(block, 2)?;
                Ok(Self::CyclicClassDef(
                    ocamlrep::from::field(block, 0)?,
                    ocamlrep::vec_from_ocaml_set(block[1])?,
                ))
            }
            t => Err(ocamlrep::FromError::BlockTagOutOfRange { max: 1, actual: t }),
        }
    }
}
