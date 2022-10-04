(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [expand_typedef ctx name ty_argl] returns the full expansion of the type alias named [name].
    E.g. if `type A<T> = Vec<B<T>>; type B<T> = Map<int, T>`, [expand_typedef ctx "A" [string]] returns `Vec<Map<int, string>>`.

    Parameters:
    - name: name of type alias to expand
    - ty_argl: list of type arguments used to substitute the expanded typedef's type parameters

    All transparent aliases in the expansion are expanded recursively, and type parameter substitution is performed.
    Opaque typedefs are not expanded, regardless of the current file.
    Detects cycles, but does not raise an error - it just stops expanding instead. *)
val expand_typedef :
  Provider_context.t ->
  string ->
  Typing_defs.decl_ty list ->
  Typing_defs.decl_ty
