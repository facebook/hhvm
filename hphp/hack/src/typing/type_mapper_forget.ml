(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Map = Type_mapper_generic

type env = {
  (* the tyvar to be removed *)
  tyvar: Tvid.t;
  (* why did we need to forget this type? *)
  forget_reason: Reason.t option;
}

let make_env tyvar =
  let forget_reason = None in
  { tyvar; forget_reason }

class type forget_tyvar_mapper_type =
  object
    inherit [env] Map.internal_type_mapper_type

    inherit [env] Map.locl_constraint_type_mapper_type

    method recurse_opt : env -> locl_ty -> locl_ty option

    method recurse_internal_type_opt :
      env -> internal_type -> internal_type option
  end

(** Type mapper which forgets a type variable, i.e. removes all occurences of a type variable *)
class forget_tyvar_mapper : forget_tyvar_mapper_type =
  object (this)
    inherit [env] Map.deep_type_mapper

    inherit! [env] Map.constraint_type_mapper

    inherit! [env] Map.internal_type_mapper

    method private result_opt : type a. env -> a -> a option =
      fun env x ->
        match env.forget_reason with
        | Some _ -> None
        | None -> Some x

    method recurse_opt env locl_ty =
      let (env, locl_ty) = this#on_type env locl_ty in
      this#result_opt env locl_ty

    method recurse_internal_type_opt env ity =
      let (env, ity) = this#on_internal_type env ity in
      this#result_opt env ity

    method! on_tvar env r var =
      if Tvid.equal var env.tyvar then
        ({ env with forget_reason = Some r }, mk (r, Tunion []))
      else
        (env, mk (r, Tvar var))

    method! on_tunion env r tyl =
      let tyl =
        List.filter_map tyl ~f:(fun x ->
            let (env, ty) = this#on_type env x in
            this#recurse_opt env ty)
      in
      (env, mk (r, Tunion tyl))

    method! on_tintersection env r tyl =
      let tyl =
        List.filter_map tyl ~f:(fun x ->
            let (env, ty) = this#on_type env x in
            this#recurse_opt env ty)
      in
      (env, mk (r, Tintersection tyl))
  end
