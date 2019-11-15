(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Typing_env

(** Unions and intersections containing unsolved type variables may remain
in an unsimplified form once those type variables get solved.

For example, consider the union (#1 | int) where #1 is an unsolved type variable.
If #1 gets solved to int, then this union will remain in the unsimplified form
(int | int) which compromise the robustness of some of our logic and might
cause performance issues (by creating big unsimplified unions).

To solve this problem, we wrap each union and intersection in a type var,
so we'd get `#2 -> (#1 | int)` (This is done in Typing_union and
Typing_intersection), and register that #1 occurs in #2 in
[env.tyvar_occurrences]. Then when #1 gets solved, we simplify #2.

This module deals with this simplification.

The simplification is recursive: simplifying a type variable will
trigger simplification of its own occurrences. *)

let simplify_occurrences env v =
  Env.log_env_change "simplify_occurrences" env
  @@
  let rec simplify_occurrences env v ~seen_vars =
    let vars = Env.get_tyvar_occurrences env v in
    let (env, seen_vars) =
      ISet.fold
        (fun v' (env, seen_vars) ->
          (* This type variable is now solved and does not contain any unsolved
          type variable, so we can remove it from its occurrences. *)
          let env =
            Env.make_tyvar_no_more_occur_in_tyvar env v ~no_more_in:v'
          in
          (* Only simplify when the type of v' does not contain any more
          unsolved type variables. *)
          if not @@ Env.contains_unsolved_tyvars env v' then
            simplify_type_of_var env v' ~seen_vars
          else
            (env, seen_vars))
        vars
        (env, seen_vars)
    in
    (env, seen_vars)
  and simplify_type_of_var env v ~seen_vars =
    if ISet.mem v seen_vars then
      (env, seen_vars)
    else
      let seen_vars = ISet.add v seen_vars in
      let (env, v') = Env.get_var env v in
      let (env, ty) = Env.expand_var env Reason.Rnone v in
      let (env, ty) =
        (* Only simplify the type of variables which are bound directly to a
        concrete type to preserve the variable aliasings and save some memory. *)
        if Ident.equal v v' then
          (* The following call to simplify_unions might itself return a
          type variable v wrapping a union. If that union contains a type variable
          v', then at this point v has already been added to the tyvar occurrences
          of v' when doing the wrapping. *)
          Typing_utils.simplify_unions env ty
        else
          (env, ty)
      in
      (* Note that this call into add will itself maintain the occurrence maps *)
      let env = Env.add env v ty in
      (* Recursively simplify occurrences of v itself. *)
      simplify_occurrences env v ~seen_vars
  in
  (* Only simplify when the type of v does not contain any more
  unsolved type variables. *)
  if not @@ Env.contains_unsolved_tyvars env v then
    fst @@ simplify_occurrences env v ~seen_vars:ISet.empty
  else
    env
