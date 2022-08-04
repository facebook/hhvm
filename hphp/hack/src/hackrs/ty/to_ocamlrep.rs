// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::decl_error::*;

impl<P: ocamlrep::ToOcamlRep> ocamlrep::ToOcamlRep for DeclError<P> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(
        &'a self,
        alloc: &'a A,
    ) -> ocamlrep::OpaqueValue<'a> {
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
            Self::CyclicClassDef(pos, stack) => {
                // The stack is an SSet rather than a list in OCaml, so we need
                // to construct a tree set here. One way is sorting the list and
                // passing it to `sorted_iter_to_ocaml_set`.
                let mut stack = stack.clone();
                stack.sort();
                stack.dedup();
                let mut iter = stack.iter().copied().map(|s| alloc.add(s.as_str()));
                let (stack, _) = ocamlrep::sorted_iter_to_ocaml_set(&mut iter, alloc, stack.len());

                let mut block = alloc.block_with_size_and_tag(2usize, 1u8);
                alloc.set_field(&mut block, 0, alloc.add(pos));
                alloc.set_field(&mut block, 1, stack);
                block.build()
            }
        }
    }
}
