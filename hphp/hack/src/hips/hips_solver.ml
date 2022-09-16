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

  (** This function returns as first argument:
      - Some (any_constr_list, CONST), if the constant CONST identified by ident_ent
      is defined; any_constr_list is base_constraint_map at CONST.
      - None otherwise
      This function returns as second argument:
      - Some (any_constr_list, CONST), if the constant identified by ident_ent is a
      class constant, and the respective class contains an extend keyword. In this
      case CONST is the first constant along the class-hierarchy, at which a definition
      occurs. any_constr_list is base_constraint_map at CONST.
      - None otherwise
      Some examples:
      a)
      class C { const dict<string, mixed> DICT = dict['a' => 42]; }
      class D extends C { }
      function main(): void { idx(D::DICT, 'b'); }
      ->
      (None, Some (_, C::DICT))

      b1)
      class C { const dict<string, mixed> DICT = dict['a' => 42]; }
      function main(): void { idx(C::DICT, 'b'); }
      ->
      (Some (_, C::DICT), None)

      b2)
      const dict<string, mixed> DICT = dict['a' => 42];
      function main(): void { idx(DICT, 'b'); }
      ->
      (Some (_, DICT), None)

      c)
      class C { const dict<string, mixed> DICT = dict['a' => 42]; }
      class E extends C { const dict<string, mixed> DICT = dict['b' => true]; }
      function main(): void { idx(E::DICT, 'e'); }
      ->
      (Some (_, E::DICT), Some (_, C::DICT)) *)
  let constr_list_and_string_of
      ({ class_name_opt; const_name; _ } as ident_ent :
        constant_identifier_entity)
      (base_constraint_map : any_constraint list SMap.t) :
      (any_constraint list * string) option
      * (any_constraint list * string) option =
    let const_ident_string = string_of_const_ident_ent ident_ent in
    let fst_map_entry =
      match SMap.find_opt const_ident_string base_constraint_map with
      | Some constr_list -> Some (constr_list, const_ident_string)
      | None -> None
    in
    let snd_map_entry =
      let rec snd_map_entry_iterator (class_name_opt : string option) :
          (any_constraint list * string) option =
        match class_name_opt with
        | Some class_name ->
          (match SMap.find_opt class_name base_constraint_map with
          | Some constr_list ->
            (match List.find ~f:is_class_extends_constr constr_list with
            | Some (Inter (ClassExtends class_identifier_ent)) ->
              let new_class_name = snd class_identifier_ent in
              let new_const_ident_string = new_class_name ^ "::" ^ const_name in
              (match
                 SMap.find_opt new_const_ident_string base_constraint_map
               with
              | Some constr_list_at_const_ent ->
                Some (constr_list_at_const_ent, new_const_ident_string)
              | None -> snd_map_entry_iterator (Some new_class_name))
            | _ -> None)
          | None -> None)
        | None -> None
      in
      snd_map_entry_iterator class_name_opt
    in
    (fst_map_entry, snd_map_entry)

  let propagate_constraints_1
      ~current_func_constr_list
      ~current_func_id
      (ident_ent : constant_identifier_entity)
      (new_const_ident_string : string)
      (constr_list_at_const_ent : any_constraint list)
      (input_constr_list_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let constr_list_backwards =
      List.filter_map
        constr_list_at_const_ent
        ~f:
          (substitute_inter_any_backwards
             (ConstantInitial (const_initial_ent_of constr_list_at_const_ent)))
    in
    let constr_list_forwards =
      List.filter_map
        current_func_constr_list
        ~f:(substitute_inter_any_forwards (ConstantIdentifier ident_ent))
    in
    input_constr_list_map
    |> SMap.update
         new_const_ident_string
         (Option.map ~f:(fun x -> x @ constr_list_forwards))
    |> SMap.update
         current_func_id
         (Option.map ~f:(fun x -> x @ constr_list_backwards))

  let propagate_constraints_2
      (constr_list_at_const_ent : any_constraint list)
      (new_const_ident_string : string)
      (input_constr_list_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let const_ent =
      match const_constraint_of constr_list_at_const_ent with
      | Inter (Constant const_ent) -> const_ent
      | _ -> failwith "Used invalid identifier"
    in
    let constr_list_forwards =
      List.filter_map
        constr_list_at_const_ent
        ~f:(substitute_inter_any_forwards (Constant const_ent))
    in
    input_constr_list_map
    |> SMap.update
         new_const_ident_string
         (Option.map ~f:(fun x -> x @ constr_list_forwards))

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
          | ConstantIdentifier ident_ent ->
            (match
               constr_list_and_string_of ident_ent argument_constraint_map
             with
            | (Some (constr_list_at_const_ent, new_const_ident_string), None)
            | (None, Some (constr_list_at_const_ent, new_const_ident_string)) ->
              propagate_constraints_1
                ~current_func_constr_list
                ~current_func_id
                ident_ent
                new_const_ident_string
                constr_list_at_const_ent
                input_constr_list_map
            | ( Some (constr_list_at_const_ent_1, new_const_ident_string_1),
                Some (_, new_const_ident_string_2) ) ->
              propagate_constraints_1
                ~current_func_constr_list
                ~current_func_id
                ident_ent
                new_const_ident_string_1
                constr_list_at_const_ent_1
                input_constr_list_map
              |> propagate_constraints_2
                   constr_list_at_const_ent_1
                   new_const_ident_string_2
            | (None, None) -> input_constr_list_map)
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

  (** Modifies input_constr_map by adding:
      i) to the constraints at the key corresponding to the constant const_ent
      in constr_list_at_const_ent: a subset constraint between constant identifier
      ident_ent and the constant const_ent;
      ii) to the constraints at the key f: a subset constraint between the constant
      initialiser const_initial_ent in constr_list_at_const_ent and the constant
      identifier ident_ent. *)
  let add_subset_constraints_1
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

  (** Modifies input_constr_map by adding:
      i) to the constraints at the key corresponding to the constant const_ent_2
      in constr_list_at_const_ent_2 (i.e. to constr_list_at_const_ent_2): a subset
      constraint between the constant const_ent_1 in constr_list_at_const_ent_1,
      and const_ent_2. *)
  let add_subset_constraints_2
      ~constr_list_at_const_ent_1
      (constr_list_at_const_ent_2 : any_constraint list)
      (input_constr_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let const_constraint_1 = const_constraint_of constr_list_at_const_ent_1 in
    let const_constraint_2 = const_constraint_of constr_list_at_const_ent_2 in
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

  (** For a given map, at every key, we iterate through its constant identifier
      constraints. For a fixed identifier, we modify the map by adding constraints via
      add_subset_constraints_1 and add_subset_constraints_2:
      i) a subset constraint between identifier and global/class constant (if undefined,
      we use inheritance);
      ii) a subset constraint between the instantiation of a constant (if undefined, we
      use inheritance) and the identifier;
      ii) a subset constraint between two constants, in case of inheritance with over-
      write. *)
  let close_identifier (current_constraint_map : any_constraint list SMap.t) :
      any_constraint list SMap.t =
    let close
        (f : string)
        (constr_list : any_constraint list)
        (input_constr_map : any_constraint list SMap.t) :
        any_constraint list SMap.t =
      let add_constraints
          (input_constr_map_2 : any_constraint list SMap.t)
          (ident_ent : constant_identifier_entity) : any_constraint list SMap.t
          =
        match constr_list_and_string_of ident_ent current_constraint_map with
        | (Some (constr_list_at_const_ent, _), None)
        | (None, Some (constr_list_at_const_ent, _)) ->
          add_subset_constraints_1
            constr_list_at_const_ent
            ident_ent
            input_constr_map_2
            f
        | ( Some (constr_list_at_const_ent_1, _),
            Some (constr_list_at_const_ent_2, _) ) ->
          add_subset_constraints_1
            constr_list_at_const_ent_1
            ident_ent
            input_constr_map_2
            f
          |> add_subset_constraints_2
               ~constr_list_at_const_ent_1
               constr_list_at_const_ent_2
        | (None, None) -> input_constr_map_2
      in
      let only_identifier (constr : any_constraint) :
          constant_identifier_entity option =
        match constr with
        | Inter (ConstantIdentifier ident_ent) -> Some ident_ent
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
