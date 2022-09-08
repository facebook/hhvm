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

  let substitute_inter_inter_forwards
      ~inter_constr_1 (inter_constr_2 : inter_constraint) :
      inter_constraint option =
    match (inter_constr_1, inter_constr_2) with
    | ( Arg (param_entity_left, intra_entity_left),
        Arg (param_entity_right, intra_entity_right) ) ->
      if
        I.is_same_entity
          (I.embed_entity (Param param_entity_left))
          intra_entity_right
      then
        Some (Arg (param_entity_right, intra_entity_left))
      else
        Some inter_constr_2
    | _ -> None
  (* TODO(T127947010) Add case for inter-procedural return constraint *)

  let substitute_inter_inter_backwards
      ~inter_constr_1 (inter_constr_2 : inter_constraint) :
      inter_constraint option =
    match (inter_constr_1, inter_constr_2) with
    | ( Arg (param_entity_left, intra_entity_left),
        Arg (param_entity_right, intra_entity_right) ) ->
      if I.is_same_entity intra_entity_left intra_entity_right then
        Some
          (Arg (param_entity_right, I.embed_entity (Param param_entity_left)))
      else
        Some inter_constr_2
    | _ -> None
  (* TODO(T127947010) Add case for inter-procedural return constraint *)

  let substitute_inter_any_backwards
      (inter_constr_1 : inter_constraint) (constr : any_constraint) :
      any_constraint option =
    match constr with
    | Intra intra_constr ->
      Option.map
        ~f:(fun x -> Intra x)
        (I.substitute_inter_intra_backwards inter_constr_1 intra_constr)
    | Inter inter_constr_2 ->
      Option.map
        ~f:(fun x -> Inter x)
        (substitute_inter_inter_backwards ~inter_constr_1 inter_constr_2)

  let substitute_inter_any_forwards
      (inter_constr_1 : inter_constraint) (constr : any_constraint) :
      any_constraint option =
    match constr with
    | Intra intra_constr ->
      Option.map
        ~f:(fun x -> Intra x)
        (I.substitute_inter_intra_forwards inter_constr_1 intra_constr)
    | Inter inter_constr_2 ->
      Option.map
        ~f:(fun x -> Inter x)
        (substitute_inter_inter_forwards ~inter_constr_1 inter_constr_2)

  let substitute
      ~base_constraint_map
      (argument_constraint_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let substitute_any_list
        (current_func_id : string)
        (current_func_constr_list : any_constraint list)
        (input_constr_list_map2 : any_constraint list SMap.t) :
        any_constraint list SMap.t =
      let substitute_any
          (input_constr_list_map : any_constraint list SMap.t)
          (constr : any_constraint) : any_constraint list SMap.t =
        match constr with
        | Intra _ -> input_constr_list_map
        | Inter inter_constr ->
          (match inter_constr with
          | Arg (((_, f), f_idx), intra_ent) ->
            let constr_list_at = SMap.find_opt f base_constraint_map in
            let param_constr : inter_constraint =
              match constr_list_at with
              | Some constr_list_at_ ->
                let param_ent_opt =
                  List.find
                    ~f:(function
                      | Inter (Param ((_, g), g_idx)) ->
                        String.equal f g && Int.equal f_idx g_idx
                      | _ -> false)
                    constr_list_at_
                in
                (match param_ent_opt with
                | Some (Inter (Param param_ent)) -> Arg (param_ent, intra_ent)
                | _ -> failwith "Used invalid function identifier")
              | None -> failwith "Used invalid function identifier"
            in
            let constr_list_backwards =
              match constr_list_at with
              | None -> []
              | Some constr_list_at_ ->
                List.filter_map
                  constr_list_at_
                  ~f:(substitute_inter_any_backwards inter_constr)
            in
            let constr_list_forwards =
              List.filter_map
                current_func_constr_list
                ~f:(substitute_inter_any_forwards param_constr)
            in
            input_constr_list_map
            |> SMap.update f (Option.map ~f:(fun x -> x @ constr_list_forwards))
            |> SMap.update
                 current_func_id
                 (Option.map ~f:(fun x -> x @ constr_list_backwards))
          | Identifier ((_, const_name) as ident_ent) ->
            let constr_list_at = SMap.find_opt const_name base_constraint_map in
            let constr_list_backwards =
              match constr_list_at with
              | None -> []
              | Some constr_list_at_ ->
                let is_const_initial_constr (constr : any_constraint) : bool =
                  match constr with
                  | Inter (ConstantInitial _) -> true
                  | _ -> false
                in
                let const_initial_constraint_opt =
                  List.find ~f:is_const_initial_constr constr_list_at_
                in
                let const_initial_ent =
                  match const_initial_constraint_opt with
                  | Some (Inter (ConstantInitial const_initial_ent)) ->
                    const_initial_ent
                  | _ -> failwith "Used invalid identifier"
                in
                List.filter_map
                  constr_list_at_
                  ~f:
                    (substitute_inter_any_backwards
                       (ConstantInitial const_initial_ent))
            in
            let constr_list_forwards =
              List.filter_map
                current_func_constr_list
                ~f:(substitute_inter_any_forwards (Identifier ident_ent))
            in
            input_constr_list_map
            |> SMap.update
                 const_name
                 (Option.map ~f:(fun x -> x @ constr_list_forwards))
            |> SMap.update
                 current_func_id
                 (Option.map ~f:(fun x -> x @ constr_list_backwards))
          | _ -> input_constr_list_map)
        (* TODO(T127947010) Add case for inter-procedural return constraint *)
      in
      List.fold_left
        ~f:substitute_any
        ~init:input_constr_list_map2
        current_func_constr_list
    in
    SMap.fold
      substitute_any_list
      argument_constraint_map
      argument_constraint_map

  (** For a given map, at every key, we iterate through its identifier constraints.
      For a fixed identifier, we modify the map by adding to the value/constraints
      at the respective global constant key: i) a subset constraint between identifier
      and global constant; and ii) the constraints/value of the map at the identifier
      key. *)
  let close_identifier (current_constraint_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let close
        (f : string)
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
        let is_const_initial_constr (constr : any_constraint) : bool =
          match constr with
          | Inter (ConstantInitial _) -> true
          | _ -> false
        in
        let const_constraint_opt =
          List.find ~f:is_const_constr constr_list_at_const_ent
          (* This raises Not_found, if no value in constr_list_at_const_ent
             satisifes is_const_constr. We assume that our map is well-behaved
             in this sense. *)
        in
        let const_initial_constraint_opt =
          List.find ~f:is_const_initial_constr constr_list_at_const_ent
        in
        let (const_ent, const_initial_ent) =
          match (const_constraint_opt, const_initial_constraint_opt) with
          | ( Some (Inter (Constant const_ent)),
              Some (Inter (ConstantInitial const_initial_ent)) ) ->
            (const_ent, const_initial_ent)
          | _ -> failwith "Used invalid identifier"
        in
        let subset_any_constr =
          Intra
            (I.subsets
               (I.embed_entity (Identifier ident_ent))
               (I.embed_entity (Constant const_ent)))
        in
        let subset_initial_any_constr =
          Intra
            (I.subsets
               const_initial_ent
               (I.embed_entity (Identifier ident_ent)))
        in
        let append_opt = Option.map ~f:(fun x -> subset_any_constr :: x) in
        let append_initial_opt =
          Option.map ~f:(fun x -> subset_initial_any_constr :: x)
        in
        SMap.update (snd const_ent) append_opt input_constr_map_2
        |> SMap.update f append_initial_opt
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
        let no_dupl_deduced_constraint_map =
          SMap.map
            (List.dedup_and_sort ~compare:I.compare_any_constraint)
            deduced_constraint_map
        in
        let no_dupl_argument_constraint_map =
          SMap.map
            (List.dedup_and_sort ~compare:I.compare_any_constraint)
            argument_constraint_map
        in
        if equiv no_dupl_argument_constraint_map no_dupl_deduced_constraint_map
        then
          Convergent no_dupl_argument_constraint_map
        else
          analyse_help (completed_iterations + 1) no_dupl_deduced_constraint_map
    in
    analyse_help 0 (close_identifier base_constraint_map)
end
