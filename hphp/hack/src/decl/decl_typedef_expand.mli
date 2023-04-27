(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type cyclic_td_usage = {
  td_name: string;
  decl_pos: Pos_or_decl.t;
      (** Position of type alias usage that caused the cycle *)
}

(** [expand_typedef ctx name ty_argl] returns the full expansion of the type alias named [name].
    E.g. if `type A<T> = Vec<B<T>>; type B<T> = Map<int, T>`, [expand_typedef ctx r "A" [string]] returns `Vec<Map<int, string>>`.

    Parameters:
    - force_expand: expand regardless of typedef visibility (default: false)
    - reason: reason to use as part of the returned decl_ty
    - name: name of type alias to expand
    - ty_argl: list of type arguments used to substitute the expanded typedef's type parameters

    All transparent aliases in the expansion are expanded recursively, and type parameter substitution is performed.
    Opaque typedefs are not expanded, regardless of the current file.
    Detects cycles, but does not raise an error - it just stops expanding instead. *)
val expand_typedef :
  ?force_expand:bool ->
  Provider_context.t ->
  Typing_defs_core.decl_phase Typing_reason.t_ ->
  string ->
  Typing_defs.decl_ty list ->
  Typing_defs.decl_ty

(** [expand_typedef ctx name ty_argl] returns the full expansion of the type alias named [name],
    and the list of cyclic usages in the definition of that typedef.
    E.g. if `type A<T> = Vec<B<T>>; type B<T> = Map<int, T>`, [expand_typedef ctx r "A"] returns `Vec<Map<int, string>>, []`.

    Parameters:
    - force_expand: expand regardless of typedef visibility (default: false)
    - reason: reason to use as part of the returned decl_ty
    - name: name of type alias to expand

    All transparent aliases in the expansion are expanded recursively, and type parameter substitution is performed.
    Opaque typedefs are not expanded, regardless of the current file. *)
val expand_typedef_with_error :
  ?force_expand:bool ->
  Provider_context.t ->
  Typing_defs_core.decl_phase Typing_reason.t_ ->
  string ->
  Typing_defs.decl_ty * cyclic_td_usage list
