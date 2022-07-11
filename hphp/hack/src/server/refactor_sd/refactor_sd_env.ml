(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Refactor_sd_types
module LMap = Local_id.Map
module Cont = Typing_continuations

let var_counter : int ref = ref 0

let fresh_var () : entity_ =
  var_counter := !var_counter + 1;
  Variable !var_counter

let union_continuation_at_lid (entity1 : entity) (entity2 : entity) :
    constraint_ list * entity =
  match (entity1, entity2) with
  | (Some entity1_, Some entity2_) ->
    let var = fresh_var () in
    let constraints = [Subset (entity1_, var); Subset (entity2_, var)] in
    (constraints, Some var)
  | (entity, None)
  | (None, entity) ->
    ([], entity)

let union_continuation (constraints : constraint_ list) cont1 cont2 =
  let union_continuation_at_lid constraints _lid entity1_opt entity2_opt :
      constraint_ list * entity option =
    match (entity1_opt, entity2_opt) with
    | (Some entity1, Some entity2) ->
      let (new_constraints, entity) =
        union_continuation_at_lid entity1 entity2
      in
      (new_constraints @ constraints, Some entity)
    | (Some entity, None)
    | (None, Some entity) ->
      (constraints, Some entity)
    | (None, None) -> (constraints, None)
  in
  let (constraints, cont) =
    LMap.merge_env constraints cont1 cont2 ~combine:union_continuation_at_lid
  in
  (constraints, cont)

module LEnv = struct
  type t = lenv

  let init bindings : t = Cont.Map.add Cont.Next bindings Cont.Map.empty

  let get_local_in_continuation lenv cont lid : entity =
    let open Option.Monad_infix in
    lenv |> Cont.Map.find_opt cont >>= LMap.find_opt lid |> Option.join

  let get_local lenv : LMap.key -> entity =
    get_local_in_continuation lenv Cont.Next

  let set_local_in_continuation lenv cont lid entity : t =
    let update_cont = function
      | None -> None
      | Some lenv_per_cont -> Some (LMap.add lid entity lenv_per_cont)
    in
    Cont.Map.update cont update_cont lenv

  let set_local lenv lid entity : t =
    set_local_in_continuation lenv Cont.Next lid entity

  let union (lenv1 : t) (lenv2 : t) : constraint_ list * t =
    let combine constraints _ cont1 cont2 =
      let (constraints, cont) = union_continuation constraints cont1 cont2 in
      (constraints, Some cont)
    in
    Cont.Map.union_env [] lenv1 lenv2 ~combine
end

let init tast_env constraints bindings =
  { constraints; lenv = LEnv.init bindings; tast_env }

let add_constraint env constraint_ =
  { env with constraints = constraint_ :: env.constraints }

let reset_constraints env = { env with constraints = [] }

let get_local env = LEnv.get_local env.lenv

let set_local env lid entity =
  let lenv = LEnv.set_local env.lenv lid entity in
  { env with lenv }

let union (parent_env : env) (env1 : env) (env2 : env) : env =
  let (points_to_constraints, lenv) = LEnv.union env1.lenv env2.lenv in
  let constraints =
    points_to_constraints
    @ env1.constraints
    @ env2.constraints
    @ parent_env.constraints
  in
  { parent_env with lenv; constraints }
