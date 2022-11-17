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

  let debug_constraint_map label m =
    Format.printf "%s:\n" label;
    m
    |> SMap.iter (fun key cs ->
           Format.printf "  key %s:\n" key;
           cs
           |> List.iter ~f:(fun c ->
                  Format.printf "    %s\n" @@ I.debug_any_constraint c));
    m

  let substitute_inter_any_backwards
      (inter_constr_1 : inter_constraint) (constr : any_constraint) :
      any_constraint option =
    match constr with
    | Intra intra_constr ->
      Option.map
        ~f:(fun x -> Intra x)
        (I.substitute_inter_intra_backwards inter_constr_1 intra_constr)
    | Inter _inter_constr_2 -> None

  let substitute_inter_any_forwards
      (inter_constr_1 : inter_constraint) (constr : any_constraint) :
      any_constraint option =
    match constr with
    | Intra intra_constr ->
      Option.map
        ~f:(fun x -> Intra x)
        (I.substitute_inter_intra_forwards inter_constr_1 intra_constr)
    | Inter _ -> None

  let string_of_const_ident_ent
      ({ class_name_opt; const_name; _ } : constant_identifier_entity) : string
      =
    match class_name_opt with
    | Some class_name -> class_name ^ "::" ^ const_name
    | None -> const_name

  let is_const_initial_constr = function
    | Inter (ConstantInitial _) -> true
    | _ -> false

  let is_const_constr = function
    | Inter (Constant _) -> true
    | _ -> false

  let is_class_extends_constr = function
    | Inter (ClassExtends _) -> true
    | _ -> false

  let const_constraint_of (constr_list : any_constraint list) : any_constraint =
    match List.find ~f:is_const_constr constr_list with
    | Some const_constraint -> const_constraint
    | None -> failwith "Couldn't find Constant constraint"

  let const_initial_ent_of (constr_list : any_constraint list) : I.intra_entity
      =
    match List.find ~f:is_const_initial_constr constr_list with
    | Some (Inter (ConstantInitial const_initial_ent)) -> const_initial_ent
    | _ -> failwith "Used invalid identifier"

  let find_const_in_ancestors ~constraint_map ~const_name class_name :
      (any_constraint list * string) option =
    let rec aux class_name =
      let open Option.Monad_infix in
      SMap.find_opt class_name constraint_map
      >>= List.find ~f:is_class_extends_constr
      >>= function
      | Inter (ClassExtends class_identifier_ent) ->
        let new_class_name = snd class_identifier_ent in
        let new_const_ident_string = new_class_name ^ "::" ^ const_name in
        (match SMap.find_opt new_const_ident_string constraint_map with
        | Some constr_list_at_const_ent ->
          Some (constr_list_at_const_ent, new_const_ident_string)
        | None -> aux new_class_name)
      | _ -> None
    in
    aux class_name

  let find_const_in_current_class
      (qualified_name : string)
      (base_constraint_map : any_constraint list SMap.t) :
      (any_constraint list * string) option =
    match SMap.find_opt qualified_name base_constraint_map with
    | Some constr_list -> Some (constr_list, qualified_name)
    | None -> None

  let find_const
      ({ class_name_opt; const_name; _ } as ident_ent :
        constant_identifier_entity)
      (constraint_map : any_constraint list SMap.t) :
      (any_constraint list * string) option =
    let current_class_qualified_name = string_of_const_ident_ent ident_ent in
    let current_class_const =
      find_const_in_current_class current_class_qualified_name constraint_map
    in
    if Option.is_some current_class_const then
      current_class_const
    else
      Option.bind
        class_name_opt
        ~f:(find_const_in_ancestors ~constraint_map ~const_name)

  let propagate_const_initializer_and_identifier_constraints
      ~current_func_constr_list
      ~current_func_id
      (ident_ent : constant_identifier_entity)
      (new_const_ident_string : string)
      (constr_list_at_const_ent : any_constraint list)
      (input_constr_list_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let to_append_at_current_func =
      List.filter_map
        constr_list_at_const_ent
        ~f:
          (substitute_inter_any_backwards
             (ConstantInitial (const_initial_ent_of constr_list_at_const_ent)))
    in
    let to_append_at_const =
      List.filter_map
        current_func_constr_list
        ~f:(substitute_inter_any_backwards (ConstantIdentifier ident_ent))
    in
    input_constr_list_map
    |> SMap.update
         current_func_id
         (Option.map ~f:(fun x -> x @ to_append_at_current_func))
    |> SMap.update
         new_const_ident_string
         (Option.map ~f:(fun x -> x @ to_append_at_const))

  let propagate_const_inheritance_constraints
      (constr_list_at_const_ent : any_constraint list)
      (new_const_ident_string : string)
      (input_constr_list_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let const_ent =
      match const_constraint_of constr_list_at_const_ent with
      | Inter (Constant const_ent) -> const_ent
      | _ -> failwith "Used invalid identifier"
    in
    let to_append_at_const_ident =
      List.filter_map
        constr_list_at_const_ent
        ~f:(substitute_inter_any_backwards (Constant const_ent))
    in
    input_constr_list_map
    |> SMap.update
         new_const_ident_string
         (Option.map ~f:(fun x -> x @ to_append_at_const_ident))

  (**
  example: `"C::DICT_NAME" -> Some ("C", "DICT_NAME")`

  This is silly, but saves memory compared to storing more information in `const` *)
  let split_const_name (qualified_name : string) : (string * string) option =
    let matches = Caml.String.split_on_char ':' qualified_name in
    match matches with
    | [class_name; _; const_name] -> Some (class_name, const_name)
    | _ -> None

  let substitute (constraint_map : any_constraint list SMap.t) :
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
          | ArgLike (((_, f), f_idx), intra_ent) ->
            let constr_list_at = SMap.find_opt f constraint_map in
            let param_constr : inter_constraint =
              match constr_list_at with
              | Some constr_list_at_ ->
                let param_ent_opt =
                  List.find
                    ~f:(function
                      | Inter (ParamLike ((_, g), g_idx)) ->
                        String.equal f g
                        && Hips_types.equal_param_like_index f_idx g_idx
                      | _ -> false)
                    constr_list_at_
                in
                (match param_ent_opt with
                | Some (Inter (ParamLike param_ent)) ->
                  ArgLike (param_ent, intra_ent)
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
          | ConstantIdentifier ident_ent ->
            (match find_const ident_ent constraint_map with
            | Some (constr_list_at_const_ent, new_const_ident_string) ->
              propagate_const_initializer_and_identifier_constraints
                ~current_func_constr_list
                ~current_func_id
                ident_ent
                new_const_ident_string
                constr_list_at_const_ent
                input_constr_list_map
            | None -> input_constr_list_map)
          | Constant (_, qualified_name) ->
            let open Option.Monad_infix in
            split_const_name qualified_name
            >>= (fun (class_name, const_name) ->
                  let current_class_const =
                    find_const_in_current_class
                      qualified_name
                      input_constr_list_map
                  in
                  let ancestor_class_const =
                    find_const_in_ancestors
                      ~constraint_map:input_constr_list_map
                      ~const_name
                      class_name
                  in
                  Option.both current_class_const ancestor_class_const)
            >>| (fun ((constr_list_at_const, _), (_, ancestor_const_name)) ->
                  input_constr_list_map
                  |> propagate_const_inheritance_constraints
                       constr_list_at_const
                       ancestor_const_name)
            |> Option.value ~default:input_constr_list_map
          | _ -> input_constr_list_map)
      in
      List.fold_left
        ~f:substitute_any
        ~init:input_constr_list_map2
        current_func_constr_list
    in
    SMap.fold substitute_any_list constraint_map constraint_map

  (** Modifies input_constr_map by adding:
      i) to the constraints at the key corresponding to the constant const_ent
      in constr_list_at_const_ent: a subset constraint between constant identifier
      ident_ent and the constant const_ent;
      ii) to the constraints at the key f: a subset constraint between the constant
      initializer const_initial_ent in constr_list_at_const_ent and the constant
      identifier ident_ent. *)
  let add_const_identifier_and_initializer_subset_constraints
      (constr_list_at_const_ent : any_constraint list)
      (ident_ent : constant_identifier_entity)
      (input_constr_map : any_constraint list SMap.t)
      (f : string) : any_constraint list SMap.t =
    let const_constraint = const_constraint_of constr_list_at_const_ent in
    let const_initial_constraint =
      match List.find ~f:is_const_initial_constr constr_list_at_const_ent with
      | Some const_initial_constraint -> const_initial_constraint
      | None -> failwith "Couldn't find ConstantInitial constraint"
    in
    let (const_ent, const_initial_ent) =
      match (const_constraint, const_initial_constraint) with
      | (Inter (Constant const_ent), Inter (ConstantInitial const_initial_ent))
        ->
        (const_ent, const_initial_ent)
      | _ -> failwith "Used invalid identifier"
    in
    let subset_any_constr =
      Intra
        (I.subsets
           (I.embed_entity (ConstantIdentifier ident_ent))
           (I.embed_entity (Constant const_ent)))
    in
    let subset_initial_any_constr =
      Intra
        (I.subsets
           const_initial_ent
           (I.embed_entity (ConstantIdentifier ident_ent)))
    in
    let append_opt = Option.map ~f:(fun x -> subset_any_constr :: x) in
    let append_initial_opt =
      Option.map ~f:(fun x -> subset_initial_any_constr :: x)
    in
    SMap.update (snd const_ent) append_opt input_constr_map
    |> SMap.update f append_initial_opt

  let add_const_identifier_constraints ~key constraint_map const_identifier =
    match find_const const_identifier constraint_map with
    | Some (constr_list_at_const_ent, _) ->
      add_const_identifier_and_initializer_subset_constraints
        constr_list_at_const_ent
        const_identifier
        constraint_map
        key
    | None -> constraint_map

  let add_const_inheritance_constraints
      ~(ent1_constraints : any_constraint list)
      ~(ent2_constraints : any_constraint list)
      (input_constr_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let const_constraint_1 = const_constraint_of ent1_constraints in
    let const_constraint_2 = const_constraint_of ent2_constraints in
    let (const_ent_1, const_ent_2) =
      match (const_constraint_1, const_constraint_2) with
      | (Inter (Constant const_ent_1), Inter (Constant const_ent_2)) ->
        (const_ent_1, const_ent_2)
      | _ -> failwith "Used invalid identifier"
    in
    let subset_any_constr =
      Intra
        (I.subsets
           (I.embed_entity (Constant const_ent_1))
           (I.embed_entity (Constant const_ent_2)))
    in
    let append_opt = Option.map ~f:(fun x -> subset_any_constr :: x) in
    SMap.update (snd const_ent_2) append_opt input_constr_map

  (** Add subset constraints for constant overrides.

    ```
    % Datalog:
    subset(constant1, constant2) :- overrides(constant1, constant2).

    overrides(constant1, constant2) :-
      member_of(constant1, class1),
      inherits_from(class1, class2),
      constant_name(constant1) == constant_name(constant2).
    ```
   *)
  let add_const_constraints constraint_map const =
    let open Option.Monad_infix in
    split_const_name (snd const)
    >>= (fun (class_name, const_name) ->
          let current_class_const =
            find_const_in_current_class (snd const) constraint_map
          in
          let ancestor_class_const =
            find_const_in_ancestors ~constraint_map ~const_name class_name
          in
          Option.both current_class_const ancestor_class_const)
    >>| (fun ((constr_list_in_class, _), (constr_list_in_ancestor, _)) ->
          add_const_inheritance_constraints
            ~ent1_constraints:constr_list_in_class
            ~ent2_constraints:constr_list_in_ancestor
            constraint_map)
    |> Option.value ~default:constraint_map

  (* add new interprocedural constraints by examining the cross-join of all constraints *)
  let close (constraint_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let add_constraints_for_key
        (key : string)
        (constr_list : any_constraint list)
        (constraint_map : any_constraint list SMap.t) :
        any_constraint list SMap.t =
      let add_constraints_for_constraint
          (constraint_map : any_constraint list SMap.t)
          (constr : any_constraint) : any_constraint list SMap.t =
        match constr with
        | Inter (ConstantIdentifier const_identifier) ->
          add_const_identifier_constraints ~key constraint_map const_identifier
        | Inter (Constant const) -> add_const_constraints constraint_map const
        | _ -> constraint_map
      in
      List.fold_left
        ~f:add_constraints_for_constraint
        ~init:constraint_map
        constr_list
    in
    SMap.fold add_constraints_for_key constraint_map constraint_map

  let analyse (base_constraint_map : any_constraint list SMap.t) ~verbose :
      solution =
    let debug =
      if verbose then
        debug_constraint_map
      else
        fun _ m ->
      m
    in
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
      if verbose then Format.printf "\n=== iteration: %d\n" completed_iterations;
      if Int.equal completed_iterations I.max_iteration then
        Divergent argument_constraint_map
      else
        let substituted_constraint_map =
          debug "substituted_constraint_map"
          @@ substitute argument_constraint_map
        in
        let deduced_constraint_map =
          debug "deduced_constraint_map"
          @@ SMap.map deduce_any_list substituted_constraint_map
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
    analyse_help
      0
      (debug "closed base_constraint_map" @@ close base_constraint_map)
end
