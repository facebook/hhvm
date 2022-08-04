(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hips_types

module Inter (I : Intra) = struct
  open Hips_types

  type inter_constraint = I.inter_constraint

  type intra_constraint = I.intra_constraint

  type any_constraint = I.any_constraint

  type solution =
    | Divergent of any_constraint list SMap.t
    | Convergent of any_constraint list SMap.t

  let equiv
      (constraint_map_1 : any_constraint list SMap.t)
      (constraint_map_2 : any_constraint list SMap.t) : bool =
    SMap.equal I.equiv constraint_map_1 constraint_map_2

  let substitute
      ~base_constraint_map
      (argument_constraint_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let substitute_inter_inter
        ~inter_constr_1 (inter_constr_2 : inter_constraint) : inter_constraint =
      match (inter_constr_1, inter_constr_2) with
      | ( I.Arg (param_entity_left, intra_entity_left),
          I.Arg (param_entity_right, intra_entity_right) ) ->
        if I.is_same_entity (Param param_entity_left) intra_entity_right then
          I.Arg (param_entity_right, intra_entity_left)
        else
          inter_constr_2
      (* TODO(T127947010) Add case for inter-procedural return constraint *)
    in
    let substitute_inter_any
        (inter_constr_1 : inter_constraint) (constr : any_constraint) :
        any_constraint =
      match constr with
      | I.Intra intra_constr ->
        I.Intra (I.substitute_inter_intra inter_constr_1 intra_constr)
      | I.Inter inter_constr_2 ->
        I.Inter (substitute_inter_inter ~inter_constr_1 inter_constr_2)
    in
    let substitute_any (constr : any_constraint) : any_constraint list =
      match constr with
      | I.Intra _ -> [constr]
      | I.Inter inter_constr ->
        begin
          match inter_constr with
          | I.Arg ((f, _), _) ->
            let constr_list_at = SMap.find_opt f base_constraint_map in
            (match constr_list_at with
            | None -> []
            | Some constr_list_at ->
              List.map constr_list_at ~f:(substitute_inter_any inter_constr))
            (* TODO(T127947010) Add case for inter-procedural return constraint *)
        end
    in
    let substitute_any_list (xs : any_constraint list) : any_constraint list =
      List.map xs ~f:substitute_any |> List.concat
    in
    SMap.map substitute_any_list argument_constraint_map

  let analyse (base_constraint_map : any_constraint list SMap.t) : solution =
    let deduce_any_list (any_constraint_list : any_constraint list) :
        any_constraint list =
      let destruct (any_constraint_list : any_constraint list) :
          intra_constraint list * inter_constraint list =
        let f (any_constraint : any_constraint) :
            (intra_constraint, inter_constraint) Base__.Either0.t =
          match any_constraint with
          | I.Intra intra_constr -> First intra_constr
          | I.Inter inter_constr -> Second inter_constr
        in
        List.partition_map ~f any_constraint_list
      in
      let construct
          ((intra_constraint_list, inter_constraint_list) :
            intra_constraint list * inter_constraint list) : any_constraint list
          =
        List.map
          ~f:(fun intra_constr -> I.Intra intra_constr)
          intra_constraint_list
        @ List.map
            ~f:(fun inter_constr -> I.Inter inter_constr)
            inter_constraint_list
      in
      destruct any_constraint_list
      |> (fun (intra_constr_list, inter_constr_list) ->
           (I.deduce intra_constr_list, inter_constr_list))
      |> construct
    in
    let rec analyse_help
        (completed_iterations : int)
        (argument_constraint_map : any_constraint list SMap.t) : solution =
      if Int.equal completed_iterations I.max_iteration then
        Divergent argument_constraint_map
      else
        let substituted_constraint_map =
          substitute ~base_constraint_map argument_constraint_map
        in
        let deduced_constraint_map =
          SMap.map deduce_any_list substituted_constraint_map
        in
        if equiv argument_constraint_map deduced_constraint_map then
          Convergent argument_constraint_map
        else
          analyse_help (completed_iterations + 1) deduced_constraint_map
    in
    analyse_help 0 base_constraint_map
end
