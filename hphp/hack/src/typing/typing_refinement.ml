(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Typing_env_types
module Env = Typing_env
module DataType = Typing_case_types.AtomicDataTypes

module TyPartition = Partition.Make (struct
  type t = locl_ty

  let compare = Typing_defs.compare_locl_ty ~normalize_lists:true
end)

type dnf_ty = locl_ty list list

type ty_partition = {
  predicate: type_predicate;
  left: dnf_ty;
  span: dnf_ty;
  right: dnf_ty;
}

let rec split_ty_by_tuple
    ~(expansions : SSet.t)
    ~(ty_datatype : DataType.t)
    (env : env)
    (ty : locl_ty)
    (sub_predicates : type_predicate list) : env * TyPartition.t =
  match deref ty with
  (* TODO: optional and variadic fields T201398626 T201398652 *)
  | ( ty_reason,
      Ttuple
        { t_required; t_extra = Textra { t_optional = []; t_variadic = _ } } )
    ->
    let predicate_ty_pairs = List.zip sub_predicates t_required in
    begin
      match predicate_ty_pairs with
      | List.Or_unequal_lengths.Unequal_lengths ->
        (env, TyPartition.mk_right ty (* mismatch arity *))
      | List.Or_unequal_lengths.Ok predicate_ty_pairs ->
        (* split each tuple element ty by its respective predicate *)
        let (env, sub_splits) =
          List.fold_map
            predicate_ty_pairs
            ~init:env
            ~f:(fun env (predicate, ty) ->
              split_ty ~expansions env ty ~predicate)
        in
        (env, TyPartition.product (Typing_make_type.tuple ty_reason) sub_splits)
    end
  | _ ->
    (* Tuples are vecs at runtime, thus if the type's data type is disjoint from a vec
       we can conclude the type must be in the right partition. Otherwise we do not
       precisely know the relationship with the [IsTupleOf] predicate so default to
       spanning
    *)
    let (env, predicate_datatype) = DataType.(of_ty env Tuple) in
    if DataType.are_disjoint env ty_datatype predicate_datatype then
      (env, TyPartition.mk_right ty)
    else
      (env, TyPartition.mk_span ty)

and split_ty_by_shape
    ~(expansions : SSet.t)
    ~(ty_datatype : DataType.t)
    (env : env)
    (ty : locl_ty)
    { sp_fields = field_predicate_map } : env * TyPartition.t =
  match deref ty with
  | ( ty_reason,
      Tshape { s_origin = _; s_unknown_value; s_fields = field_ty_map } ) ->
    let has_class_const_field map =
      TShapeMap.exists
        (fun field _val ->
          match field with
          | TSFclass_const _ -> true
          | TSFregex_group _
          | TSFlit_str _ ->
            false)
        map
    in
    if
      has_class_const_field field_ty_map
      || has_class_const_field field_predicate_map
    then
      (* class const field names are unsound, so fall back to span *)
      (env, TyPartition.mk_span ty)
    else if
      (* Ignore (span) open shape, optional fields for now (T196048813) *)

      (* We're going to remove this test later (T196048813) so don't worry so
         much about this being too specific of a check *)
      (not (Typing_defs.is_nothing s_unknown_value))
      || TShapeMap.exists
           (fun _field { sft_optional; sft_ty = _ } -> sft_optional)
           field_ty_map
    then
      (env, TyPartition.mk_span ty)
    else
      let field_ty_pairs =
        List.sort
          (TShapeMap.elements field_ty_map)
          ~compare:(fun (f1, _) (f2, _) -> TShapeField.compare f1 f2)
      in
      let field_predicate_pairs =
        List.sort
          (TShapeMap.elements field_predicate_map)
          ~compare:(fun (f1, _) (f2, _) -> TShapeField.compare f1 f2)
      in
      if
        not
        @@ List.equal
             TShapeField.equal
             (List.map field_ty_pairs ~f:fst)
             (List.map field_predicate_pairs ~f:fst)
      then
        (* mismatched fields *)
        (env, TyPartition.mk_right ty)
      else
        (* Calculate the splits for each field *)
        let (env, field_split_pairs) =
          List.fold_map
            (List.zip_exn field_ty_pairs field_predicate_pairs)
            ~init:env
            ~f:(fun
                 env
                 ( (field, { sft_optional = _; sft_ty }),
                   (_field, { sfp_predicate }) )
               ->
              let (env, splits) =
                split_ty ~expansions env sft_ty ~predicate:sfp_predicate
              in
              (env, (field, splits)))
        in
        let (fields, splits) = List.unzip field_split_pairs in
        let mk_shape tys =
          Typing_make_type.shape ty_reason (Typing_make_type.nothing ty_reason)
          @@ TShapeMap.of_list
          @@ List.zip_exn fields
          @@ List.map tys ~f:(fun ty -> { sft_optional = false; sft_ty = ty })
        in
        (env, TyPartition.product mk_shape splits)
  | _ ->
    (* Shapes are dicts at runtime, thus if the type's data type is disjoint from a dict
       we can conclude the type must be in the right partition. Otherwise we do not
       precisely know the relationship with the [IsShapeOf] prediacte so default to
       spanning
    *)
    let (env, predicate_datatype) = DataType.(of_ty env Shape) in
    if DataType.are_disjoint env ty_datatype predicate_datatype then
      (env, TyPartition.mk_right ty)
    else
      (env, TyPartition.mk_span ty)

and split_ty_by_tag
    ~(ty_datatype : DataType.t) (env : env) (ty : locl_ty) (tag : type_tag) :
    env * TyPartition.t =
  let (env, predicate_datatype) = DataType.of_tag env tag in
  let predicate_complement_datatype = DataType.complement predicate_datatype in
  if DataType.are_disjoint env ty_datatype predicate_datatype then
    (env, TyPartition.mk_right ty)
  else if DataType.are_disjoint env ty_datatype predicate_complement_datatype
  then
    (env, TyPartition.mk_left ty)
  else
    (env, TyPartition.mk_span ty)

and split_ty
    ~(expansions : SSet.t)
    (env : env)
    (ty : locl_ty)
    ~(predicate : type_predicate) : env * TyPartition.t =
  let (env, ety) = Env.expand_type env ty in
  let partition_f ((env : env), (ty_datatype : DataType.t)) :
      env * TyPartition.t =
    match snd predicate with
    | IsTupleOf { tp_required = sub_predicates } ->
      split_ty_by_tuple ~expansions ~ty_datatype env ety sub_predicates
    | IsShapeOf shape_predicate ->
      split_ty_by_shape ~expansions ~ty_datatype env ety shape_predicate
    | IsTag tag -> split_ty_by_tag ~ty_datatype env ety tag
  in
  let split_union ~expansions env (tys : locl_ty list) =
    let (env, partitions) =
      List.fold_map ~init:env ~f:(split_ty ~expansions ~predicate) tys
    in
    let partition =
      List.fold ~init:TyPartition.mk_bottom ~f:TyPartition.join partitions
    in
    (env, partition)
  in
  let split_intersection ~init env ~expansions (tys : locl_ty list) =
    let (env, partitions) =
      List.fold_map ~init:env ~f:(split_ty ~expansions ~predicate) tys
    in
    let partition = List.fold ~init ~f:TyPartition.meet partitions in
    (env, partition)
  in
  let (env, partition) =
    match get_node ety with
    (* Types we cannot split, that we know will end up being a part of both
       partitions. *)
    | Tvar _
    | Tany _
    | Tdynamic
    | Taccess _
    | Tunapplied_alias _ ->
      (env, TyPartition.mk_span ty)
    (* Types we cannot split *)
    | Tprim
        Aast.(
          (Tint | Tnull | Tvoid | Tbool | Tfloat | Tstring | Tresource) as prim)
      ->
      partition_f DataType.(of_ty env @@ Primitive prim)
    | Tfun _ -> partition_f DataType.(of_ty env Function)
    | Tnonnull -> partition_f DataType.(of_ty env Nonnull)
    | Ttuple _ -> partition_f DataType.(of_ty env Tuple)
    | Tshape _ -> partition_f DataType.(of_ty env Shape)
    | Tlabel _ -> partition_f DataType.(of_ty env Label)
    | Tclass ((_, name), _, _) -> partition_f DataType.(of_ty env @@ Class name)
    | Tneg (_, predicate) ->
      let (env, dty) =
        match predicate with
        | IsTag tag -> DataType.of_tag env tag
        | IsTupleOf _ -> DataType.(of_ty env Tuple)
        | IsShapeOf _ -> DataType.(of_ty env Shape)
      in
      let dty = DataType.complement dty in
      partition_f (env, dty)
    | Tprim Aast.Tnoreturn -> (env, TyPartition.mk_bottom)
    (* Types we can split into a union of types *)
    | Tunion tyl -> split_union ~expansions env tyl
    | Tprim Aast.Tnum ->
      split_union
        env
        ~expansions
        [
          Typing_make_type.int (get_reason ty);
          Typing_make_type.float (get_reason ty);
        ]
    | Tprim Aast.Tarraykey ->
      split_union
        env
        ~expansions
        [
          Typing_make_type.int (get_reason ty);
          Typing_make_type.string (get_reason ty);
        ]
    | Tvec_or_dict (tk, tv) ->
      split_union
        env
        ~expansions
        [
          Typing_make_type.vec (get_reason ty) tv;
          Typing_make_type.dict (get_reason ty) tk tv;
        ]
    | Toption ty_opt ->
      split_union
        env
        ~expansions
        [Typing_make_type.null (get_reason ty); ty_opt]
    (* Types we need to split across an intersection *)
    | Tintersection [] ->
      split_ty ~expansions ~predicate env
      @@ Typing_make_type.mixed (get_reason ty)
    | Tintersection (ty :: tyl) ->
      let (env, init) = split_ty ~expansions ~predicate env ty in
      split_intersection env ~init ~expansions tyl
    (* Below are types of the form T <: U. We treat these as T & U *)
    | Tdependent (_, super_ty) ->
      let (env, partition) = split_ty ~expansions ~predicate env super_ty in
      (env, TyPartition.(meet (mk_span ty) partition))
    | Tgeneric (name, _)
    | Tnewtype (name, _, _)
      when SSet.mem name expansions ->
      (env, TyPartition.mk_span ty)
    | Tgeneric (name, tyl) ->
      let expansions = SSet.add name expansions in
      let upper_bounds =
        Env.get_upper_bounds env name tyl |> Typing_set.elements
      in
      let init = TyPartition.mk_span ty in
      split_intersection env ~init ~expansions upper_bounds
    | Tnewtype (name, tyl, as_ty) ->
      let init = TyPartition.mk_span ty in
      let expansions = SSet.add name expansions in
      begin
        match Env.get_typedef env name with
        | Decl_entry.Found
            { td_type_assignment = CaseType (variant, variants); td_tparams; _ }
          ->
          (* The this_ty does not need to be set because case types cannot
           * appear within classes thus cannot us the this type.
           * If we ever change that this could needs to be changed *)
          let localize =
            Typing_utils.localize
              ~ety_env:
                {
                  empty_expand_env with
                  substs =
                    (if List.is_empty tyl then
                      SMap.empty
                    else
                      Decl_subst.make_locl td_tparams tyl);
                }
          in
          (* TODO T201569125 - do I need to do something with the where constraints here? *)
          let tyl = List.map (variant :: variants) ~f:fst in
          let (env, tyl) =
            List.fold_map tyl ~init:env ~f:(fun env variant ->
                let ((env, _ty_err_opt), variant) = localize env variant in
                (env, variant))
          in
          let (env, partition_tyl) = split_union env ~expansions tyl in
          let (env, partition_as_ty) =
            split_intersection env ~init ~expansions [as_ty]
          in
          (env, TyPartition.meet partition_tyl partition_as_ty)
        | _ -> split_intersection env ~init ~expansions [as_ty]
      end
  in
  (* If one side of the partition is empty that means [ty] falls completely
     under the other side. Set the partition equal to the type to avoid
     computing any unions or intersections *)
  let partition = TyPartition.simplify partition ty in
  (env, partition)

let partition_ty (env : env) (ty : locl_ty) (predicate : type_predicate) =
  let (env, partition) = split_ty ~expansions:SSet.empty ~predicate env ty in
  ( env,
    {
      predicate;
      left = TyPartition.left partition;
      span = TyPartition.span partition;
      right = TyPartition.right partition;
    } )

module TyPredicate = struct
  let rec of_ty env (ty : locl_ty) =
    Result.map ~f:(fun pred -> (get_reason ty, pred))
    @@
    match get_node ty with
    | Tprim Aast.Tbool -> Result.Ok (IsTag BoolTag)
    | Tprim Aast.Tint -> Result.Ok (IsTag IntTag)
    | Tprim Aast.Tstring -> Result.Ok (IsTag StringTag)
    | Tprim Aast.Tarraykey -> Result.Ok (IsTag ArraykeyTag)
    | Tprim Aast.Tfloat -> Result.Ok (IsTag FloatTag)
    | Tprim Aast.Tnum -> Result.Ok (IsTag NumTag)
    | Tprim Aast.Tresource -> Result.Ok (IsTag ResourceTag)
    | Tprim Aast.Tnull -> Result.Ok (IsTag NullTag)
    (* TODO: optional and variadic fields T201398626 T201398652 *)
    | Ttuple { t_required; t_extra = Textra { t_optional = []; t_variadic } }
      when is_nothing t_variadic -> begin
      match
        List.fold_left t_required ~init:(Result.Ok []) ~f:(fun acc ty ->
            let open Result.Monad_infix in
            acc >>= fun predicates ->
            of_ty env ty >>| fun predicate -> predicate :: predicates)
      with
      | Result.Error err -> Result.Error ("tuple-" ^ err)
      | Result.Ok predicates ->
        Result.Ok (IsTupleOf { tp_required = List.rev predicates })
    end
    | Tshape { s_origin = _; s_unknown_value; s_fields } ->
      if
        not
        @@ TypecheckerOptions.type_refinement_partition_shapes env.genv.tcopt
      then
        Result.Error "shape"
      else if
        (* this type comes from localizing a hint so a closed shape should have
           the canonical form of the nothing type *)
        Typing_defs.is_nothing s_unknown_value
      then begin
        match
          TShapeMap.fold
            (fun key s_field acc ->
              if s_field.sft_optional then
                (* Skip shapes with optional fields for now T196048813 *)
                Result.Error "optional_field"
              else
                let open Result.Monad_infix in
                acc >>= fun sf_predicates ->
                of_ty env s_field.sft_ty >>| fun sfp_predicate ->
                (key, { sfp_predicate }) :: sf_predicates)
            s_fields
            (Result.Ok [])
        with
        | Result.Error err -> Result.Error ("shape-" ^ err)
        | Result.Ok elts ->
          Result.Ok (IsShapeOf { sp_fields = TShapeMap.of_list elts })
      end else
        Result.Error "open_shape"
    | Tprim Aast.Tvoid -> Result.Error "void"
    | Tprim Aast.Tnoreturn -> Result.Error "noreturn"
    | Tnonnull -> Result.Error "nonnull"
    | Tdynamic -> Result.Error "dynamic"
    | Tany _ -> Result.Error "any"
    | Toption _ -> Result.Error "option"
    | Tfun _ -> Result.Error "fun"
    | Tgeneric (_, _) -> Result.Error "generic"
    | Tunion _ -> Result.Error "union"
    | Tintersection _ -> Result.Error "intersection"
    | Tvec_or_dict _ -> Result.Error "vec_or_dict"
    | Taccess _ -> Result.Error "access"
    | Tvar _ -> Result.Error "tvar"
    | Tnewtype (s, _, _) -> Result.Error ("newtype-" ^ s)
    | Tunapplied_alias _ -> Result.Error "unapplied_alias"
    | Tdependent _ -> Result.Error "dependent"
    | Tclass _ -> Result.Error "class"
    | Tneg _ -> Result.Error "neg"
    | Tlabel _ -> Result.Error "label"
    | Ttuple _ -> Result.Error "tuple"

  let rec to_ty predicate =
    let tag_to_ty reason tag =
      match tag with
      | BoolTag -> Typing_make_type.bool reason
      | IntTag -> Typing_make_type.int reason
      | StringTag -> Typing_make_type.string reason
      | ArraykeyTag -> Typing_make_type.arraykey reason
      | FloatTag -> Typing_make_type.float reason
      | NumTag -> Typing_make_type.num reason
      | ResourceTag -> Typing_make_type.resource reason
      | NullTag -> Typing_make_type.null reason
      | ClassTag id -> Typing_make_type.class_type reason id []
    in
    match predicate with
    | (reason, IsTag tag) -> tag_to_ty reason tag
    | (reason, IsTupleOf { tp_required }) ->
      mk
        ( reason,
          Ttuple
            {
              t_required = List.map tp_required ~f:to_ty;
              t_extra =
                Textra
                  {
                    t_optional = [];
                    t_variadic = Typing_make_type.nothing reason;
                  };
            } )
    | (reason, IsShapeOf { sp_fields }) ->
      let map =
        TShapeMap.map
          (fun { sfp_predicate } ->
            { sft_optional = false; sft_ty = to_ty sfp_predicate })
          sp_fields
      in
      Typing_make_type.shape reason (Typing_make_type.nothing reason) map
end
