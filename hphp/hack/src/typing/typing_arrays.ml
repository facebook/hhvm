(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Common
open Typing_defs
open Typing_env_types
open Type_mapper
module Env = Typing_env
module TUtils = Typing_utils
module Reason = Typing_reason

(* Mapper used by update_array* functions. It traverses Tunion and
 * modifies the type "inside" the Tvars - so it has side effects on the input
 * type (the type variables inside env change)! *)
class update_array_type_mapper : type_mapper_type =
  object
    inherit shallow_type_mapper

    inherit! tunion_type_mapper

    inherit! tvar_substituting_type_mapper
  end

(* Abstract types declared "as array<...>" permit array operations, but if
 * those operations modify the array it has to be downgraded from generic
 * to just an array.*)
class virtual downcast_tabstract_to_array_type_mapper =
  object (this)
    method private try_super_types env ty =
      match TUtils.get_all_supertypes env ty with
      | (_, []) -> (env, ty)
      | (env, tyl) ->
        let is_array ty =
          match get_node ty with
          | Tarraykind _ -> true
          | _ -> false
        in
        (match List.filter tyl is_array with
        | [] -> (env, ty)
        | x :: _ ->
          (* If the abstract type has multiple concrete supertypes
    which are arrays, just take the first one.
    TODO(jjwu): Try all of them and find one that works
    *)
          this#on_type env x)

    method on_tgeneric env r x =
      let ty = mk (r, Tgeneric x) in
      this#try_super_types env ty

    method on_tdependent env r x ty =
      let ty = mk (r, Tdependent (x, ty)) in
      this#try_super_types env ty

    method on_tnewtype env r x tyl ty =
      let ty = mk (r, Tnewtype (x, tyl, ty)) in
      this#try_super_types env ty

    method virtual on_type : env -> locl_ty -> result
  end

let union env tyl =
  match tyl with
  | [] -> Env.fresh_type env Pos.none (* TODO: position *)
  | ty :: tyl' -> List.fold_left_env env tyl' ~init:ty ~f:TUtils.union

let union_keys = union

let union_values env values =
  let unknown =
    List.find values (fun ty ->
        TUtils.is_sub_type_for_union env (mk (Reason.none, make_tany ())) ty)
  in
  match unknown with
  | Some ty -> (env, mk (get_reason ty, TUtils.tany env))
  | None -> union env values

(* Apply this function to a type after lvalue array access that should update
 * array type (e.g from AKempty to AKdarray after using it as a dict). *)
let update_array_type p ~is_map env ty =
  let mapper =
    object
      inherit update_array_type_mapper

      inherit! downcast_tabstract_to_array_type_mapper

      method! on_tarraykind_akempty env _ =
        if is_map then
          let (env, tk) = Env.fresh_type env p in
          let (env, tv) = Env.fresh_type env p in
          (env, mk (Reason.Rused_as_map p, Tarraykind (AKdarray (tk, tv))))
        else
          let (env, tv) = Env.fresh_type env p in
          (env, mk (Reason.Rappend p, Tarraykind (AKvarray tv)))
    end
  in
  let (env, ty) = mapper#on_type (fresh_env env) ty in
  (env, ty)
