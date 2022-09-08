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
      | ( Arg (param_entity_left, intra_entity_left),
          Arg (param_entity_right, intra_entity_right) ) ->
        if I.is_same_entity (Param param_entity_left) intra_entity_right then
          Arg (param_entity_right, intra_entity_left)
        else
          inter_constr_2
      | _ -> inter_constr_2
      (* TODO(T127947010) Add case for inter-procedural return constraint *)
    in
    let substitute_inter_any
        (inter_constr_1 : inter_constraint) (constr : any_constraint) :
        any_constraint =
      match constr with
      | Intra intra_constr ->
        Intra (I.substitute_inter_intra inter_constr_1 intra_constr)
      | Inter inter_constr_2 ->
        Inter (substitute_inter_inter ~inter_constr_1 inter_constr_2)
    in
    let substitute_any (constr : any_constraint) : any_constraint list =
      match constr with
      | Intra _ -> [constr]
      | Inter inter_constr ->
        begin
          match inter_constr with
          | Arg ((f, _, _), _) ->
            let constr_list_at = SMap.find_opt f base_constraint_map in
            (match constr_list_at with
            | None -> []
            | Some constr_list_at ->
              List.map constr_list_at ~f:(substitute_inter_any inter_constr))
          | _ ->
            []
            (* TODO(T127947010) Add case for inter-procedural return constraint *)
        end
    in
    let substitute_any_list (xs : any_constraint list) : any_constraint list =
      List.map xs ~f:substitute_any |> List.concat
    in
    SMap.map substitute_any_list argument_constraint_map

  (** For a given map, at every key, we iterate through its identifier constraints.
      For a fixed identifier, we modify the map by adding to the value/constraints
      at the respective global constant key: i) a subset constraint between identifier
      and global constant; and ii) the constraints/value of the map at the identifier
      key. *)
  let close_identifier (current_constraint_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let close
        (_ : string)
        (constr_list : any_constraint list)
        (input_constr_map : any_constraint list SMap.t) :
        any_constraint list SMap.t =
      let add_constraints
          (input_constr_map_2 : any_constraint list SMap.t)
          (ident_ent : identifier_entity) : any_constraint list SMap.t =
        let constr_list_at_const_ent =
          SMap.find (snd ident_ent) current_constraint_map
          (* This raises Not_found, if no binding exists. We assume
             that identifiers only refer to existing constants. *)
        in
        let is_const_constr (constr : any_constraint) : bool =
          match constr with
          | Inter (Constant _) -> true
          | _ -> false
        in
        let const_constraint_opt =
          List.find ~f:is_const_constr constr_list_at_const_ent
          (* This raises Not_found, if no value in constr_list_at_const_ent
             satisifes is_const_constr. We assume that our map is well-behaved
             in this sense. *)
        in
        let const_ent =
          match const_constraint_opt with
          | Some (Inter (Constant const_ent)) -> const_ent
          | _ -> failwith "Used invalid identifier"
        in
        let subset_any_constr = Intra (I.subsets ident_ent const_ent) in
        let append_opt (any_constr_list_opt : any_constraint list option) :
            any_constraint list option =
          match any_constr_list_opt with
          | None -> None
          | Some any_constr_list ->
            Some (subset_any_constr :: (any_constr_list @ constr_list))
        in
        SMap.update (snd const_ent) append_opt input_constr_map_2
      in
      let only_identifier (constr : any_constraint) : identifier_entity option =
        match constr with
        | Inter (Identifier ident_ent) -> Some ident_ent
        | _ -> None
      in
      let only_identifier_list =
        List.filter_map ~f:only_identifier constr_list
      in
      List.fold_left
        ~f:add_constraints
        ~init:input_constr_map
        only_identifier_list
    in
    SMap.fold close current_constraint_map current_constraint_map

  let analyse (base_constraint_map : any_constraint list SMap.t) : solution =
    let deduce_any_list (any_constraint_list : any_constraint list) :
        any_constraint list =
      let destruct (any_constraint_list : any_constraint list) :
          intra_constraint list * inter_constraint list =
        let f (any_constraint : any_constraint) :
            (intra_constraint, inter_constraint) Base__.Either0.t =
          match any_constraint with
          | Intra intra_constr -> First intra_constr
          | Inter inter_constr -> Second inter_constr
        in
        List.partition_map ~f any_constraint_list
      in
      let construct
          ((intra_constraint_list, inter_constraint_list) :
            intra_constraint list * inter_constraint list) : any_constraint list
          =
        List.map
          ~f:(fun intra_constr -> Intra intra_constr)
          intra_constraint_list
        @ List.map
            ~f:(fun inter_constr -> Inter inter_constr)
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
    analyse_help 0 (close_identifier base_constraint_map)
end
