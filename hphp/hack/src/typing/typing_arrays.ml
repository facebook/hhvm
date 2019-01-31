(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Common
open Typing_defs
open Type_mapper

module Env = Typing_env
module TUtils = Typing_utils
module Reason = Typing_reason
module ShapeMap = Nast.ShapeMap

(* Mapper used by update_array* functions. It traverses Tunresolved and
 * modifies the type "inside" the Tvars - so it has side effects on the input
 * type (the type variables inside env change)! *)
class update_array_type_mapper: type_mapper_type = object
  inherit shallow_type_mapper
  inherit! tunresolved_type_mapper
  inherit! tvar_substituting_type_mapper
end

(* Abstract types declared "as array<...>" permit array operations, but if
 * those operations modify the array it has to be downgraded from generic
 * to just an array.*)
class virtual downcast_tabstract_to_array_type_mapper = object(this)
  method on_tabstract env r ak cstr =
    let ty = (r, Tabstract(ak, cstr)) in
    match TUtils.get_all_supertypes env ty with
    | _, [] -> env, ty
    | env, tyl ->
      let is_array = function
      | _, Tarraykind _ -> true
      | _ -> false in
      match List.filter tyl is_array with
      | [] ->
        env, ty
      | x::_ ->
        (* If the abstract type has multiple concrete supertypes
        which are arrays, just take the first one.
        TODO(jjwu): Try all of them and find one that works
        *)
        this#on_type env x

  method virtual on_type : env -> locl ty -> result
end

let union env tyl = match tyl with
  | [] -> Env.fresh_unresolved_type env Pos.none (* TODO: position *)
  | ty::tyl' -> List.fold_left_env env tyl' ~init:ty ~f:TUtils.union

let union_keys = union

let union_values env values =
  let unknown = List.find values (fun ty ->
    snd (snd (TUtils.fold_unresolved env ty)) = Tany) in
  match unknown with
  | Some (r, _) -> env, (r, TUtils.tany env)
  | None -> union env values

(* Apply this function to a type after lvalue array access that should update
 * array type (e.g from AKempty to AKmap after using it as a map). *)
let update_array_type p ~is_map env ty =
  let mapper = object
    inherit update_array_type_mapper
    inherit! downcast_tabstract_to_array_type_mapper

    method! on_tarraykind_akempty env _ =
      if is_map
      then
        let env, tk = Env.fresh_unresolved_type env p in
        let env, tv = Env.fresh_unresolved_type env p in
        env, (Reason.Rused_as_map p, Tarraykind (AKmap (tk, tv)))
      else
        let env, tv = Env.fresh_unresolved_type env p in
        env, (Reason.Rappend p, Tarraykind (AKvec tv))

  end in
  let env, ty = mapper#on_type (fresh_env env) ty in
  env, ty
