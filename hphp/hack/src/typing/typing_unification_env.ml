(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* A "unification environment" to track certain metadata about the unification
 * or subtyping of a type that isn't captured by the type itself. This is
 * currently managed manually by unify and sub_type, but TODO we should probably
 * have some sort of "apply" function that handles this sort of thing for
 * recursive (but not corecursive) functions, managing to make sure they don't
 * get stuck in a loop with Tvar, mapping across Tunresolved, dealing with "as"
 * constraints, etc. *)
type uenv = {
  (* We have gone under a Toption, so we should drop any
   * further Toption we see. This can happen with types which are
   * Toption[Tunresolved[Toption]] or similar. *)
  unwrappedToption: bool;

  (* The first expression dependent type we've seen while subtyping. This is
   * used in Typing_subtype to properly handle the 'this' type for inheritance.
   *)
  this_ty: Typing_defs.locl Typing_defs.ty option;

}

let empty = { unwrappedToption = false; this_ty = None; }

let update_this_if_unset uenv ty =
  match uenv.this_ty with
  | Some _ ->
    uenv
  | None ->
    { uenv with this_ty=(Some ty) }
