(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types
module LMap = Local_id.Map

let init saved_env = { constraints = []; lenv = LMap.empty; saved_env }

let add_constraint env constraint_ =
  { env with constraints = constraint_ :: env.constraints }

let get_local lid env = LMap.find_opt lid env.lenv |> Option.join

let set_local lid entity env = { env with lenv = LMap.add lid entity env.lenv }

let var_counter : int ref = ref 0

let fresh_var () : entity_ =
  var_counter := !var_counter + 1;
  Variable !var_counter

let merge (env1 : env) (env2 : env) : env =
  let merge_lenv_at_lid constraints _lid entity1_opt entity2_opt :
      constraint_ list * entity option =
    match (entity1_opt, entity2_opt) with
    | (Some (Some entity1), Some (Some entity2)) ->
      let var = fresh_var () in
      let constraints =
        [Points_to (var, entity1); Points_to (var, entity2)] @ constraints
      in
      (constraints, Some (Some var))
    | (Some entity, Some None)
    | (Some None, Some entity)
    | (Some entity, None)
    | (None, Some entity) ->
      ([], Some entity)
    | (None, None) -> ([], None)
  in
  let merge_lenv (lenv1 : lenv) (lenv2 : lenv) : constraint_ list * lenv =
    LMap.merge_env [] lenv1 lenv2 ~combine:merge_lenv_at_lid
  in
  let (points_to_constraints, lenv) = merge_lenv env1.lenv env2.lenv in
  (* Saved environment does not change within a callable, so we can pick either. *)
  let saved_env = env1.saved_env in
  (* TODO: The following is gross because it duplciates existing constraints
     not just new ones. Either change to a set representation of constraints or
     restructure the code so that we only combine new constraints. *)
  let constraints =
    points_to_constraints @ env1.constraints @ env2.constraints
  in
  { lenv; saved_env; constraints }
