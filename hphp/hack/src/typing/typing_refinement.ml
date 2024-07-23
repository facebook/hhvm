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

let rec split_ty
    ~(expansions : SSet.t)
    (env : env)
    (ty : locl_ty)
    ~(predicate : type_predicate) : TyPartition.t =
  let predicate_datatype = DataType.of_predicate env predicate in
  let predicate_complement_datatype = DataType.complement predicate_datatype in
  let partition_tuple_by_tuple
      ty_reason
      (tuple_tyl : locl_ty list)
      (sub_predicates : type_predicate list) : TyPartition.t =
    let predicate_ty_pairs = List.zip sub_predicates tuple_tyl in
    begin
      match predicate_ty_pairs with
      | List.Or_unequal_lengths.Unequal_lengths ->
        TyPartition.mk_right ty (* mismatch arity *)
      | List.Or_unequal_lengths.Ok predicate_ty_pairs ->
        (* split each tuple element ty by its respective predicate *)
        let sub_splits =
          List.map predicate_ty_pairs ~f:(fun (predicate, ty) ->
              split_ty ~expansions env ty ~predicate)
        in
        TyPartition.product (Typing_make_type.tuple ty_reason) sub_splits
    end
  in
  let partition_f (env : env) (ty_datatype : DataType.t) : TyPartition.t =
    if DataType.are_disjoint env ty_datatype predicate_datatype then
      (* We use the DataType as a quick check but for some types the DataType is
         insufficiently precise.
         If either DataType is an underapproximation, then the disjointness
         check may not be valid.
         When we convert a type or predicate to a DataType, this will usually be
         an overapproximation, but if the type is negative or we otherwise
         complement the DataType, this becomes an underapproximation.
         Here, ty_datatype may have come from a negation and will be an
         underapproximation if the type was a negative of a predicate for which
         the negated predicate's datatype (before negation) is an
         overapproximation. This is true for the following negated predicates:

         - (only, for now) IsTupleOf -- in this case, the disjointness check
           will not hold if predicate is also IsTupleOf; and so we span
      *)
      match get_node ty with
      | Tneg neg -> begin
        match neg with
        (* we'll over-approximate the DataType for IsTupleOf *)
        | Neg_predicate (IsTupleOf _np_predicates) -> begin
          match predicate with
          | IsTupleOf _predicates -> TyPartition.mk_span ty
          | IsBool
          | IsInt
          | IsString
          | IsArraykey
          | IsFloat
          | IsNum
          | IsResource
          | IsNull ->
            TyPartition.mk_right ty
        end
        (* The DataType for these are precise *)
        | Neg_predicate
            ( IsBool | IsInt | IsString | IsArraykey | IsFloat | IsNum
            | IsResource | IsNull )
        (* We don't have a class-predicate right now *)
        | Neg_class _ ->
          TyPartition.mk_right ty
      end
      | _ -> TyPartition.mk_right ty
    else if DataType.are_disjoint env ty_datatype predicate_complement_datatype
    then
      (* Similar to above, here, predicate_complement_datatype may be an
         underapproximation if predicate is:

         - (only, for now) IsTupleOf -- in this case, if ty is a Ttuple, we can
           do a deeper split
      *)
      match predicate with
      | IsTupleOf predicates -> begin
        match get_node ty with
        | Ttuple tyl -> partition_tuple_by_tuple (get_reason ty) tyl predicates
        | _ -> TyPartition.mk_span ty
      end
      | IsBool
      | IsInt
      | IsString
      | IsArraykey
      | IsFloat
      | IsNum
      | IsResource
      | IsNull ->
        TyPartition.mk_left ty
    else
      TyPartition.mk_span ty
  in
  let split_union ~expansions (tys : locl_ty list) =
    let partitions = List.map ~f:(split_ty ~expansions ~predicate env) tys in
    List.fold ~init:TyPartition.mk_bottom ~f:TyPartition.join partitions
  in
  let split_intersection ~init ~expansions (tys : locl_ty list) =
    let partitions = List.map ~f:(split_ty ~expansions ~predicate env) tys in
    List.fold ~init ~f:TyPartition.meet partitions
  in
  let (env, ety) = Env.expand_type env ty in
  let partition =
    match get_node ety with
    (* Types we cannot split, that we know will end up being a part of both
       partitions. *)
    | Tvar _
    | Tany _
    | Tdynamic
    | Taccess _
    | Tunapplied_alias _ ->
      TyPartition.mk_span ty
    (* Types we cannot split *)
    | Tprim
        Aast.(
          (Tint | Tnull | Tvoid | Tbool | Tfloat | Tstring | Tresource) as prim)
      ->
      partition_f env DataType.(of_ty env @@ Primitive prim)
    | Tfun _ -> partition_f env DataType.(of_ty env Function)
    | Tnonnull -> partition_f env DataType.(of_ty env Nonnull)
    | Ttuple _ -> partition_f env DataType.(of_ty env Tuple)
    | Tshape _ -> partition_f env DataType.(of_ty env Shape)
    | Tlabel _ -> partition_f env DataType.(of_ty env Label)
    | Tclass ((_, name), _, _) ->
      partition_f env DataType.(of_ty env @@ Class name)
    | Tneg (Neg_class (_, name)) ->
      partition_f env DataType.(complement @@ of_ty env @@ Class name)
    | Tneg (Neg_predicate pred) ->
      partition_f env DataType.(complement @@ of_predicate env pred)
    | Tprim Aast.Tnoreturn -> TyPartition.mk_bottom
    (* Types we can split into a union of types *)
    | Tunion tyl -> split_union ~expansions tyl
    | Tprim Aast.Tnum ->
      split_union
        ~expansions
        [
          Typing_make_type.int (get_reason ty);
          Typing_make_type.float (get_reason ty);
        ]
    | Tprim Aast.Tarraykey ->
      split_union
        ~expansions
        [
          Typing_make_type.int (get_reason ty);
          Typing_make_type.string (get_reason ty);
        ]
    | Tvec_or_dict (tk, tv) ->
      split_union
        ~expansions
        [
          Typing_make_type.vec (get_reason ty) tv;
          Typing_make_type.dict (get_reason ty) tk tv;
        ]
    | Toption ty_opt ->
      split_union ~expansions [Typing_make_type.null (get_reason ty); ty_opt]
    (* Types we need to split across an intersection *)
    | Tintersection [] ->
      split_ty ~expansions ~predicate env
      @@ Typing_make_type.mixed (get_reason ty)
    | Tintersection (ty :: tyl) ->
      let init = split_ty ~expansions ~predicate env ty in
      split_intersection ~init ~expansions tyl
    (* Below are types of the form T <: U. We treat these as T & U *)
    | Tdependent (_, super_ty) ->
      TyPartition.(
        meet (mk_span ty) @@ split_ty ~expansions ~predicate env super_ty)
    | Tgeneric (name, _)
    | Tnewtype (name, _, _)
      when SSet.mem name expansions ->
      TyPartition.mk_span ty
    | Tgeneric (name, tyl) ->
      let expansions = SSet.add name expansions in
      let upper_bounds =
        Env.get_upper_bounds env name tyl |> Typing_set.elements
      in
      let init = TyPartition.mk_span ty in
      split_intersection ~init ~expansions upper_bounds
    | Tnewtype (name, tyl, as_ty) ->
      let init = TyPartition.mk_span ty in
      let expansions = SSet.add name expansions in
      begin
        match Env.get_typedef env name with
        | Decl_entry.Found
            { td_type = variants; td_vis = Aast.CaseType; td_tparams; _ } ->
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
          let tyl =
            match get_node variants with
            | Tunion tyl -> tyl
            | _ -> [variants]
          in
          let tyl =
            List.map tyl ~f:(fun variant ->
                let ((_env, _ty_err_opt), variant) = localize env variant in
                variant)
          in
          TyPartition.(
            meet (split_union ~expansions tyl)
            @@ split_intersection ~init ~expansions [as_ty])
        | _ -> split_intersection ~init ~expansions [as_ty]
      end
  in
  (* If one side of the partition is empty that means [ty] falls completely
     under the other side. Set the partition equal to the type to avoid
     computing any unions or intersections *)
  TyPartition.simplify partition ty

let partition_ty (env : env) (ty : locl_ty) (predicate : type_predicate) =
  let partition = split_ty ~expansions:SSet.empty ~predicate env ty in
  {
    predicate;
    left = TyPartition.left partition;
    span = TyPartition.span partition;
    right = TyPartition.right partition;
  }

module TyPredicate = struct
  let rec of_ty env ty =
    match get_node ty with
    | Tprim Aast.Tbool -> Some (env, IsBool)
    | Tprim Aast.Tint -> Some (env, IsInt)
    | Tprim Aast.Tstring -> Some (env, IsString)
    | Tprim Aast.Tarraykey -> Some (env, IsArraykey)
    | Tprim Aast.Tfloat -> Some (env, IsFloat)
    | Tprim Aast.Tnum -> Some (env, IsNum)
    | Tprim Aast.Tresource -> Some (env, IsResource)
    | Tprim Aast.Tnull -> Some (env, IsNull)
    | Ttuple tys -> begin
      match
        List.fold_left
          tys
          ~init:(Some (env, []))
          ~f:(fun acc ty ->
            match acc with
            | None -> None
            | Some (env, predicates) -> begin
              match of_ty env ty with
              | Some (env, predicate) -> Some (env, predicate :: predicates)
              | None -> None
            end)
      with
      | None -> None
      | Some (env, predicates) -> Some (env, IsTupleOf (List.rev predicates))
    end
    | _ -> None

  let rec to_ty reason predicate =
    match predicate with
    | IsBool -> Typing_make_type.bool reason
    | IsInt -> Typing_make_type.int reason
    | IsString -> Typing_make_type.string reason
    | IsArraykey -> Typing_make_type.arraykey reason
    | IsFloat -> Typing_make_type.float reason
    | IsNum -> Typing_make_type.num reason
    | IsResource -> Typing_make_type.resource reason
    | IsNull -> Typing_make_type.null reason
    | IsTupleOf predicates ->
      Typing_make_type.tuple reason (List.map predicates ~f:(to_ty reason))
end
