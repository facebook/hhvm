(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module LMap = Local_id.Map
module Cont = Typing_continuations

let init_lenv = Cont.Map.add Cont.Next LMap.empty Cont.Map.empty

let init saved_env = { constraints = []; lenv = init_lenv; saved_env }

let add_constraint env constraint_ =
  { env with constraints = constraint_ :: env.constraints }

let reset_constraints env = { env with constraints = [] }

let get_local_in_continuation cont lid env =
  let open Option.Monad_infix in
  env.lenv |> Cont.Map.find_opt cont >>= LMap.find_opt lid |> Option.join

let get_local = get_local_in_continuation Cont.Next

let add_to_continuation cont lid entity lenv =
  let update_cont = function
    | None -> None
    | Some lenv_per_cont -> Some (LMap.add lid entity lenv_per_cont)
  in
  Cont.Map.update cont update_cont lenv

let set_local lid entity env =
  let lenv = add_to_continuation Cont.Next lid entity env.lenv in
  { env with lenv }

let var_counter : int ref = ref 0

let fresh_var () : entity_ =
  var_counter := !var_counter + 1;
  Variable !var_counter

let union (parent_env : env) (env1 : env) (env2 : env) : env =
  let union_lenv_at_lid constraints _lid entity1_opt entity2_opt :
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
  let union_at_continuation constraints _ lenv_per_cont1 lenv_per_cont2 =
    let (constraints, lenv) =
      LMap.merge_env
        constraints
        lenv_per_cont1
        lenv_per_cont2
        ~combine:union_lenv_at_lid
    in
    (constraints, Some lenv)
  in
  let union_continuations lenv1 lenv2 =
    Cont.Map.union_env [] lenv1 lenv2 ~combine:union_at_continuation
  in
  let (points_to_constraints, lenv) = union_continuations env1.lenv env2.lenv in
  let constraints =
    points_to_constraints
    @ env1.constraints
    @ env2.constraints
    @ parent_env.constraints
  in
  { parent_env with lenv; constraints }
