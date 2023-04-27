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

  let drop_cont lenv cont : t = Cont.Map.remove cont lenv

  let drop_conts lenv conts : t = List.fold ~f:drop_cont ~init:lenv conts

  let replace_cont lenv cont_key cont_opt : t =
    match cont_opt with
    | None -> drop_cont lenv cont_key
    | Some cont -> Cont.Map.add cont_key cont lenv

  let restore_cont_from lenv ~from cont_key : t =
    let ctxopt = Cont.Map.find_opt cont_key from in
    replace_cont lenv cont_key ctxopt

  let restore_conts_from lenv ~from conts : t =
    List.fold ~f:(restore_cont_from ~from) ~init:lenv conts

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

let drop_cont env cont =
  let lenv = LEnv.drop_cont env.lenv cont in
  { env with lenv }

let drop_conts env conts =
  let lenv = LEnv.drop_conts env.lenv conts in
  { env with lenv }

let replace_cont env cont_key cont_opt =
  let lenv = LEnv.replace_cont env.lenv cont_key cont_opt in
  { env with lenv }

let restore_conts_from env ~from conts : env =
  let lenv = LEnv.restore_conts_from env.lenv ~from conts in
  { env with lenv }

let stash_and_do env conts f : env =
  let parent_locals = env.lenv in
  let env = drop_conts env conts in
  let env = f env in
  restore_conts_from env ~from:parent_locals conts

let union_cont_opt (constraints : constraint_ list) cont_opt1 cont_opt2 =
  match (cont_opt1, cont_opt2) with
  | (None, opt)
  | (opt, None) ->
    (constraints, opt)
  | (Some cont1, Some cont2) ->
    let (constraints, cont) = union_continuation constraints cont1 cont2 in
    (constraints, Some cont)

(* Union a list of continuations *)
let union_conts (env : env) lenv cont_keys =
  let union_two (constraints, cont_opt1) cont_key =
    let cont_opt2 = Cont.Map.find_opt cont_key lenv in
    union_cont_opt constraints cont_opt1 cont_opt2
  in
  let (constraints, cont_opt) =
    List.fold cont_keys ~f:union_two ~init:(env.constraints, None)
  in
  let env = { env with constraints } in
  (env, cont_opt)

(* Union a list of source continuations and store the result in a
 * destination continuation. *)
let union_conts_and_update (env : env) ~from_conts ~to_cont =
  let lenv = env.lenv in
  let (env, unioned_cont) = union_conts env lenv from_conts in
  replace_cont env to_cont unioned_cont

let update_next_from_conts env from_conts =
  union_conts_and_update env ~from_conts ~to_cont:Cont.Next

let save_and_merge_next_in_cont env to_cont =
  union_conts_and_update env ~from_conts:[Cont.Next; to_cont] ~to_cont

let move_and_merge_next_in_cont env cont_key =
  let env = save_and_merge_next_in_cont env cont_key in
  drop_cont env Cont.Next

let loop_continuation cont_key ~env_before_iteration ~env_after_iteration =
  let cont_before_iteration_opt =
    Cont.Map.find_opt cont_key env_before_iteration.lenv
  in
  let cont_after_iteration_opt =
    Cont.Map.find_opt cont_key env_after_iteration.lenv
  in
  let new_constraints =
    let combine constraints _key entity_before_opt entity_after_opt =
      let new_constraints =
        match (entity_before_opt, entity_after_opt) with
        | (Some (Some entity_before), Some (Some entity_after)) ->
          [Subset (entity_after, entity_before)]
        | _ -> []
      in
      let constraints = new_constraints @ constraints in
      (constraints, None)
    in
    match (cont_before_iteration_opt, cont_after_iteration_opt) with
    | (Some cont_before_iteration, Some cont_after_iteration) ->
      fst
      @@ LMap.merge_env [] cont_before_iteration cont_after_iteration ~combine
    | _ -> []
  in
  {
    env_after_iteration with
    constraints = new_constraints @ env_after_iteration.constraints;
  }
