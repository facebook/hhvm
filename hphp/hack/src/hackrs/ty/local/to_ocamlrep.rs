// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use pos::ToOxidized;

impl<R: crate::reason::Reason> ocamlrep::ToOcamlRep for super::ty::Ty<R> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(
        &'a self,
        alloc: &'a A,
    ) -> ocamlrep::OpaqueValue<'a> {
        // This implementation of `to_ocamlrep` (which allocates in an arena,
        // converts to OCaml, then drops the arena) violates a `ToOcamlRep`
        // requirement: we may not drop values after passing them to `alloc.add`
        // or invoking `to_ocamlrep` (else memoization will behave incorrectly
        // in `add_root`). This leads to bizarre behavior (particularly in
        // optimized builds).
        //
        // For example, suppose we're converting a typed AST via ToOcamlRep, and
        // it contains the types `int` and `float`. When converting `int`, we'll
        // construct an arena and arena-allocate Tint, in order to construct the
        // oxidized_by_ref value `Tprim(&Tint)`, and convert that to OCaml. The
        // `ocamlrep::Allocator` will remember that the address of the `&Tint`
        // pointer corresponds to a certain OCaml value, so that when it
        // encounters future instances of that pointer, it can use that same
        // OCaml value rather than allocating a new one. We'd then free the
        // arena once we're finished converting that type. When converting the
        // second type, we construct a new arena, arena-allocate Tfloat, and
        // attempt to construct `Tprim(&Tfloat)`. But if the new arena was
        // allocated in the same location as the old, it may choose the same
        // address for our arena-allocated `Tfloat` as our `Tint` was, and our
        // ocamlrep Allocator will incorrectly use the `Tint` OCaml value.
        //
        // This memoization behavior is only enabled if we invoke
        // `ocamlrep::Allocator::add_root`, so we must take care not to use it
        // (including indirectly, through macros like `ocaml_ffi`) on values
        // containing this type.
        let arena = &bumpalo::Bump::new();
        let ty = self.to_oxidized(arena);
        // SAFETY: Transmute away the lifetime to allow the arena-allocated
        // value to be converted to OCaml. Won't break type safety in Rust, but
        // will produce broken OCaml values if used with `add_root` (see above
        // comment).
        let ty = unsafe {
            std::mem::transmute::<
                &'_ oxidized_by_ref::typing_defs::Ty<'_>,
                &'a oxidized_by_ref::typing_defs::Ty<'a>,
            >(&ty)
        };
        ty.to_ocamlrep(alloc)
    }
}
