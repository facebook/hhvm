(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Common
open Nast
open Typing_defs

module Env = Typing_env
module TUtils = Typing_utils
module Reason = Typing_reason
module TMT = Typing_make_type
module SubType = Typing_subtype

let err_witness env p = Reason.Rwitness p, TUtils.terr env

let error_array (env, tyvars) p (r, ty) =
  Errors.array_access p (Reason.to_pos r) (Typing_print.error ty);
  (env, tyvars), err_witness env p

let error_const_mutation (env, tyvars) p (r, ty) =
  Errors.const_mutation p (Reason.to_pos r) (Typing_print.error ty);
  (env, tyvars), err_witness env p

let error_assign_array_append env p (r, ty) =
  Errors.array_append p (Reason.to_pos r) (Typing_print.error ty);
  env, ((r, ty), err_witness env p)

let rec array_get ?(lhs_of_null_coalesce=false)
  is_lvalue p ((env, tyvars) as acc) ty1 e2 ty2 =
  Typing_log.(log_with_level env "typing" 1 (fun () ->
    log_types p env
      [Log_head ("array_get",
      [Log_type ("ty1", ty1); Log_type("ty2", ty2)])]));
  let env, ety1 = Env.expand_type env ty1 in
  (* This is a little weird -- we enforce the right arity when you use certain
   * collections, even in partial mode (where normally completely omitting the
   * type parameter list is admitted). Basically the "omit type parameter"
   * hole was for compatibility with certain interfaces like ArrayAccess, not
   * for collections! But it's hard to go back on now, so since we've always
   * errored (with an inscrutable error message) when you try to actually use
   * a collection with omitted type parameters, we can continue to error and
   * give a more useful error message. *)
  let arity_error (_, name) =
    Errors.array_get_arity p name (Reason.to_pos (fst ety1)) in
  let nullable_container_get ty =
    if lhs_of_null_coalesce
    (* Normally, we would not allow indexing into a nullable container,
       however, because the pattern shows up so frequently, we are allowing
       indexing into a nullable container as long as it is on the lhs of a
       null coalesce *)
    then
      array_get ~lhs_of_null_coalesce is_lvalue p (env, tyvars) ty e2 ty2
    else begin
      Errors.null_container p
        (Reason.to_string
          "This is what makes me believe it can be null"
          (fst ety1)
        );
      (env, tyvars), err_witness env p
    end in
  let type_index (env, tyvars) p ty_have ty_expect reason =
  Typing_log.(log_with_level env "typing" 1 (fun () ->
    log_types p env
    [Log_head ("array_get/type_index",
     [Log_type ("ty_have", ty_have); Log_type("ty_expect", ty_expect)])]));
    (* coerce if possible *)
    match Typing_coercion.try_coerce p reason env ty_have ty_expect with
    | Some env -> (env, tyvars)
    | None ->
        (* if subtype of dynamic, allow it to be used *)
        if Typing_subtype.is_sub_type env ty_have (fst ty_have, Tdynamic)
        then env, tyvars
        (* fail with useful error *)
        else
          let env = Typing_ops.sub_type p reason env ty_have ty_expect in
          env, tyvars
  in
  match snd ety1 with
  | Tunresolved tyl ->
      let acc, tyl = List.map_env acc tyl begin fun acc ty1 ->
        array_get ~lhs_of_null_coalesce is_lvalue p acc ty1 e2 ty2
      end in
      acc, (fst ety1, Tunresolved tyl)
  | Tarraykind (AKvarray ty | AKvec ty) ->
      let ty1 = Reason.Ridx (fst e2, fst ety1), Tprim Tint in
      let acc = type_index acc p ty2 ty1 Reason.index_array in
      acc, ty
  | Tarraykind (AKvarray_or_darray ty) ->
      let ty1 = Reason.Rvarray_or_darray_key p, Tprim Tarraykey in
      let acc = type_index acc p ty2 ty1 Reason.index_array in
      acc, ty
  | Tclass ((_, cn) as id, _, argl)
    when cn = SN.Collections.cVector
    || cn = SN.Collections.cVec ->
      let ty = match argl with
        | [ty] -> ty
        | _ -> arity_error id; err_witness env p in
      let ty1 = Reason.Ridx_vector (fst e2), Tprim Tint in
      let acc = type_index acc p ty2 ty1 (Reason.index_class cn) in
      acc, ty
  | Tclass ((_, cn) as id, _, argl)
    when cn = SN.Collections.cMap
    || cn = SN.Collections.cStableMap
    || cn = SN.Collections.cDict
    || cn = SN.Collections.cKeyset ->
      if cn = SN.Collections.cKeyset && is_lvalue then begin
        Errors.keyset_set p (Reason.to_pos (fst ety1));
        acc, err_witness env p
      end else
        let (k, v) = match argl with
          | [t] when cn = SN.Collections.cKeyset -> (t, t)
          | [k; v] when cn <> SN.Collections.cKeyset -> (k, v)
          | _ ->
              arity_error id;
              let any = err_witness env p in
              any, any
        in
        let env, ty2 = Env.unbind env ty2 in
        let acc = type_index (env, tyvars) p ty2 k (Reason.index_class cn) in
        acc, v
  (* Certain container/collection types are intended to be immutable/const,
   * thus they should never appear as a lvalue when indexing i.e.
   *
   *   $x[0] = 100; // ERROR
   *   $x[0]; // OK
   *)
  | Tclass ((_, cn) as id, _, argl)
      when cn = SN.Collections.cConstMap
        || cn = SN.Collections.cImmMap
        || cn = SN.Collections.cIndexish
        || cn = SN.Collections.cKeyedContainer ->
    if is_lvalue then
      error_const_mutation acc p ety1
    else
      let (k, v) = match argl with
        | [k; v] -> (k, v)
        | _ ->
            arity_error id;
            let any = err_witness env p in
            any, any
      in
      let acc = type_index acc p ty2 k (Reason.index_class cn) in
      acc, v
  | Tclass ((_, cn) as id, _, argl)
      when not is_lvalue &&
        (cn = SN.Collections.cConstVector || cn = SN.Collections.cImmVector) ->
      let ty = match argl with
        | [ty] -> ty
        | _ -> arity_error id; err_witness env p in
      let ty1 = Reason.Ridx (fst e2, fst ety1), Tprim Tint in
      let acc = type_index acc p ty2 ty1 (Reason.index_class cn) in
      acc, ty
  | Tclass ((_, cn), _, _)
      when is_lvalue &&
        (cn = SN.Collections.cConstVector || cn = SN.Collections.cImmVector) ->
    error_const_mutation acc p ety1
  | Tarraykind (AKdarray (k, v) | AKmap (k, v)) ->
      let env, ty2 = Env.unbind env ty2 in
      let acc = type_index (env, tyvars) p ty2 k Reason.index_array in
      acc, v
  | Tarraykind ((AKshape  _ |  AKtuple _) as akind) ->
      let key = Typing_arrays.static_array_access env (Some e2) in
      let (env, tyvars), result = match key, akind with
        | Typing_arrays.AKtuple_index index, AKtuple fields ->
            begin match IMap.get index fields with
              | Some ty ->
                  let ty1 = Reason.Ridx (fst e2, fst ety1), Tprim Tint in
                  let acc = type_index acc p ty2 ty1 Reason.index_array in
                  acc, Some ty
              | None -> acc, None
            end
        | Typing_arrays.AKshape_key field_name, AKshape fdm ->
            begin match ShapeMap.get field_name fdm with
              | Some (k, v) ->
                  let env, ty2 = Env.unbind env ty2 in
                  let acc = type_index (env, tyvars) p ty2 k Reason.index_array in
                  acc, Some v
              | None -> acc, None
            end
        | _ -> acc, None in
      begin match result with
        | Some ty -> (env, tyvars), ty
        | None ->
          (* Key is dynamic, or static and not in the array - treat it as
            regular map or vec like array *)
          let env, ty1 = Typing_arrays.downcast_aktypes env ety1 in
          array_get is_lvalue p (env, tyvars) ty1 e2 ty2
      end
  | Terr -> acc, err_witness env p
  | Tdynamic -> acc, ety1
  | Tany | Tarraykind (AKany | AKempty) ->
      acc, (Reason.Rnone, TUtils.tany env)
  | Tprim Tstring ->
      let ty = Reason.Rwitness p, Tprim Tstring in
      let ty1 = Reason.Ridx (fst e2, fst ety1), Tprim Tint in
      let acc = type_index acc p ty2 ty1 Reason.index_array in
      acc, ty
  | Ttuple tyl ->
      (* requires integer literal *)
      (match e2 with
      | p, Int n ->
          (try
            let idx = int_of_string n in
            let nth = List.nth_exn tyl idx in
            acc, nth
          with _ ->
            Errors.typing_error p (Reason.string_of_ureason Reason.index_tuple);
            acc, err_witness env p
          )
      | p, _ ->
          Errors.typing_error p (Reason.string_of_ureason Reason.URtuple_access);
          acc, err_witness env p
      )
  | Tclass ((_, cn) as id, _, argl) when cn = SN.Collections.cPair ->
      let (ty1, ty2) = match argl with
        | [ty1; ty2] -> (ty1, ty2)
        | _ ->
            arity_error id;
            let any = err_witness env p in
            any, any
      in (* requires integer literal *)
      (match e2 with
      | p, Int n ->
          (try
            let idx = int_of_string n in
            let nth = List.nth_exn [ty1; ty2] idx in
            acc, nth
          with _ ->
            Errors.typing_error p @@
            Reason.string_of_ureason (Reason.index_class cn);
            acc, err_witness env p
          )
      | p, _ ->
          Errors.typing_error p (Reason.string_of_ureason Reason.URpair_access);
          acc, err_witness env p
      )
  | Tshape (_, fdm) ->
    let p = fst e2 in
    (match TUtils.shape_field_name env e2 with
      | None ->
          (* there was already an error in shape_field name,
             don't report another one for a missing field *)
          acc, err_witness env p
      | Some field -> (match ShapeMap.get field fdm with
        | None ->
          Errors.undefined_field
            ~use_pos:p
            ~name:(TUtils.get_printable_shape_field_name field)
            ~shape_type_pos:(Reason.to_pos (fst ety1));
          acc, err_witness env p
        | Some { sft_optional = true; _ }
          when not is_lvalue && not lhs_of_null_coalesce ->
          let declared_field =
              List.find_exn
                ~f:(fun x -> Ast.ShapeField.compare field x = 0)
                (ShapeMap.keys fdm) in
          let declaration_pos = match declared_field with
            | Ast.SFlit_int (p, _) | Ast.SFlit_str (p, _) | Ast.SFclass_const ((p, _), _) -> p in
          Errors.array_get_with_optional_field
            p
            declaration_pos
            (TUtils.get_printable_shape_field_name field);
          acc, err_witness env p
        | Some { sft_optional = _; sft_ty } -> acc, sft_ty)
    )
  | Toption ty -> nullable_container_get ty
  | Tprim Tnull ->
      nullable_container_get (Reason.Rnone, Tany)
  | Tobject ->
      if Env.is_strict env
      then error_array acc p ety1
      else acc, (Reason.Rwitness p, TUtils.tany env)
  | Tabstract (AKnewtype (ts, [ty]), Some (r, Tshape (fk, fields)))
        when ts = SN.FB.cTypeStructure ->
      let env, fields = Typing_structure.transform_shapemap env ty fields in
      let ty = r, Tshape (fk, fields) in
      array_get ~lhs_of_null_coalesce is_lvalue p (env, tyvars) ty e2 ty2
  | Tabstract _ ->
    let resl =
    TUtils.try_over_concrete_supertypes env ety1
      begin fun env ty ->
        let (env, _tyvars), res =
          array_get ~lhs_of_null_coalesce is_lvalue p (env, tyvars) ty e2 ty2 in
        env, res
      end in
    begin match resl with
    | [res] -> let env, res = res in (env, tyvars), res
    | res::rest
      when List.for_all rest ~f:(fun x -> ty_equal (snd x) (snd res)) ->
      let env, res = res in (env, tyvars), res
    | _ -> error_array acc p ety1
    end
  | Tnonnull | Tprim _ | Tfun _
  | Tclass _ | Tanon (_, _) ->
      error_array acc p ety1
  (* Type-check array access as though it is the method
   * array_get<Tk,Tv>(KeyedContainer<Tk,Tv> $array, Tk $key): Tv
   * (We can already force Tk to be the type of the key argument because
   * Tk does not appear in the result of the call)
   *)
  | Tvar _ ->
    let env, value, tyvars = Env.fresh_unresolved_type_add_tyvars env p tyvars in
    let keyed_container = TMT.keyed_container (fst ety1) ty2 value in
    let env = SubType.sub_type env ty1 keyed_container in
    (env, tyvars), value

let rec assign_array_append pos ur env ty1 ty2 =
  let env, ety1 = Env.expand_type env ty1 in
  match ety1 with
  | r, (Tany | Tarraykind (AKany | AKempty)) ->
    env, (ty1, (r, TUtils.tany env))
  | r, Terr ->
    env, (ty1, (r, TUtils.terr env))
  | _, Tclass ((_, n), _, [tv])
    when n = SN.Collections.cVector || n = SN.Collections.cSet ->
    let env = Typing_ops.sub_type pos ur env ty2 tv in
    env, (ty1, tv)
  (* Handle the case where Vector or Set was used as a typehint
     without type parameters *)
  | r, Tclass ((_, n), _, [])
    when n = SN.Collections.cVector || n = SN.Collections.cSet ->
    env, (ty1, (r, TUtils.tany env))
  | _, Tclass ((_, n), _, [tk; tv]) when n = SN.Collections.cMap ->
    let tpair = TMT.pair (Reason.Rmap_append pos) tk tv in
    let env = Typing_ops.sub_type pos ur env ty2 tpair in
    env, (ty1, tpair)
  (* Handle the case where Map was used as a typehint without
     type parameters *)
  | _, Tclass ((_, n), _, []) when n = SN.Collections.cMap ->
    let tpair = TMT.class_type (Reason.Rmap_append pos) SN.Collections.cPair [] in
    let env = Typing_ops.sub_type pos ur env ty2 tpair in
    env, (ty1, tpair)
  | r, Tclass ((_, n) as id, e, [tv])
    when n = SN.Collections.cVec || n = SN.Collections.cKeyset ->
    let env, tv' = Typing_union.union env tv ty2 in
    env, ((r, Tclass (id, e, [tv'])), tv')
  | r, Tarraykind (AKvec tv) ->
    let  env, tv' = Typing_union.union env tv ty2 in
    env, ((r, Tarraykind (AKvec tv')), tv')
  | r, Tarraykind (AKvarray tv) ->
    let  env, tv' = Typing_union.union env tv ty2 in
    env, ((r, Tarraykind (AKvarray tv')), tv')
  | r, Tdynamic -> env, (ty1, (r, Tdynamic))
  | _, Tobject ->
    if Env.is_strict env
    then error_assign_array_append env pos ty1
    else env, (ty1, (Reason.Rwitness pos, TUtils.tany env))
  | r, Tunresolved ty1l ->
    let env, resl = List.map_env env ty1l (fun env ty1 -> assign_array_append pos ur env ty1 ty2) in
    let (ty1l', tyl') = List.unzip resl in
    env, ((r, Tunresolved ty1l'), (r, Tunresolved tyl'))
  | _, Tabstract _ ->
    let resl = TUtils.try_over_concrete_supertypes env ty1 begin fun env ty1 ->
      assign_array_append pos ur env ty1 ty2
    end in
    begin match resl with
    | [res] -> res
    | _ -> error_assign_array_append env pos ty1
    end
  | _, (Tnonnull | Tarraykind _ | Toption _ | Tprim _ | Tvar _ |
        Tfun _ | Tclass _ | Ttuple _ | Tanon _ | Tshape _) ->
    error_assign_array_append env pos ty1

(* Used for typing an assignment e1[key] = e2
 * where e1 has type ty1, key has type tkey and e2 has type ty2.
 * Return (ty1', ty2') where ty1' is the new array type, and ty2' is the element type
 *)
let rec assign_array_get pos ur env ty1 key tkey ty2 =
Typing_log.(log_with_level env "typing" 1 (fun () ->
  log_types pos env
  [Log_head ("assign_array_get",
   [Log_type ("ty1", ty1);
    Log_type ("tkey", tkey);
    Log_type ("ty2", ty2)])]));

  let env, ety1 = Env.expand_type env ty1 in
  let arity_error (_, name) =
    Errors.array_get_arity pos name (Reason.to_pos (fst ety1)) in
  let type_index env p ty_have ty_expect reason =
    (* coerce if possible *)
    match Typing_coercion.try_coerce p reason env ty_have ty_expect with
    | Some e -> e
    | None ->
      (* if subtype of dynamic, allow it to be used *)
      if Typing_subtype.is_sub_type env ty_have (fst ty_have, Tdynamic)
      then env
      (* fail with useful error *)
      else Typing_ops.sub_type p reason env ty_have ty_expect in
  let error = env, (ety1, err_witness env pos) in
  match snd ety1 with
  | Tunresolved ty1l ->
    let env, resl = List.map_env env ty1l (fun env ty1 ->
      assign_array_get pos ur env ty1 key tkey ty2) in
    let (ty1l', tyl') = List.unzip resl in
    env, ((fst ety1, Tunresolved ty1l'), (fst ety1, Tunresolved tyl'))
  | Tarraykind (AKvarray tv) ->
    let tk = Reason.Ridx (fst key, fst ety1), Tprim Tint in
    let env = type_index env pos tkey tk Reason.index_array in
    let env, tv' = Typing_union.union env tv ty2 in
    env, ((fst ety1, Tarraykind (AKvarray tv')), tv')
  | Tarraykind (AKvec tv) ->
    let tk = Reason.Ridx (fst key, fst ety1), Tprim Tint in
    let env = type_index env pos tkey tk Reason.index_array in
    let env, tv' = Typing_union.union env tv ty2 in
    env, ((fst ety1, Tarraykind (AKvec tv')), tv')
  | Tarraykind (AKvarray_or_darray tv) ->
    let tk = Reason.Rvarray_or_darray_key (Reason.to_pos (fst ety1)), Tprim Tarraykey in
    let env = type_index env pos tkey tk Reason.index_array in
    let env, tv' = Typing_union.union env tv ty2 in
    env, ((fst ety1, Tarraykind (AKvarray_or_darray tv')), tv')
  | Tclass ((_, cn) as id, _, argl) when cn = SN.Collections.cVector ->
    let tv = match argl with
      | [tv] -> tv
      | _ -> arity_error id; err_witness env pos in
    let tk = Reason.Ridx_vector (fst key), Tprim Tint in
    let env = type_index env pos tkey tk (Reason.index_class cn) in
    let env = Typing_ops.sub_type pos ur env ty2 tv in
    env, (ety1, tv)
  | Tclass ((_, cn) as id, e, argl) when cn = SN.Collections.cVec ->
    let tv = match argl with
      | [tv] -> tv
      | _ -> arity_error id; err_witness env pos in
    let tk = Reason.Ridx_vector (fst key), Tprim Tint in
    let env = type_index env pos tkey tk (Reason.index_class cn) in
    let env, tv' = Typing_union.union env tv ty2 in
    env, ((fst ety1, Tclass (id, e, [tv'])), tv')
  | Tclass ((_, cn) as id, _, argl)
    when cn = SN.Collections.cMap || cn = SN.Collections.cStableMap ->
    let (tk, tv) = match argl with
      | [tk; tv] -> (tk, tv)
      | _ -> arity_error id; let any = err_witness env pos in any, any in
    let env = type_index env pos tkey tk (Reason.index_class cn) in
    let env = Typing_ops.sub_type pos ur env ty2 tv in
    env, (ety1, tv)
  | Tclass ((_, cn) as id, e, argl) when cn = SN.Collections.cDict ->
    let (tk, tv) = match argl with
      | [tk; tv] -> (tk, tv)
      | _ -> arity_error id; let any = err_witness env pos in any, any in
    let env, tk' = Typing_union.union env tk tkey in
    let env, tv' = Typing_union.union env tv ty2 in
    env, ((fst ety1, Tclass (id, e, [tk'; tv'])), tv')
  | Tclass ((_, cn), _, _) when cn = SN.Collections.cKeyset ->
    Errors.keyset_set pos (Reason.to_pos (fst ety1));
    error
  | Tclass ((_, cn), _, _)
       when cn = SN.Collections.cConstMap
         || cn = SN.Collections.cImmMap
         || cn = SN.Collections.cIndexish
         || cn = SN.Collections.cKeyedContainer
         || cn = SN.Collections.cConstVector
         || cn = SN.Collections.cImmVector
         || cn = SN.Collections.cPair ->
    Errors.const_mutation pos (Reason.to_pos (fst ety1)) (Typing_print.error (snd ety1));
    error
  | Tarraykind (AKdarray (tk, tv)) ->
    let env, tk' = Typing_union.union env tk tkey in
    let env, tv' = Typing_union.union env tv ty2 in
    env, ((fst ety1, Tarraykind (AKdarray (tk', tv'))), tv')
  | Tarraykind (AKmap (tk, tv)) ->
    let env, tk' = Typing_union.union env tk tkey in
    let env, tv' = Typing_union.union env tv ty2 in
    env, ((fst ety1, Tarraykind (AKmap (tk', tv'))), tv')
  | Tarraykind ((AKshape _ | AKtuple _) as akind) ->
    let access_type = Typing_arrays.static_array_access env (Some key) in
    let fallback () =
      let env, ty1 = Typing_arrays.downcast_aktypes env ety1 in
      assign_array_get pos ur env ty1 key tkey ty2 in
    begin match access_type, akind with
    | Typing_arrays.AKtuple_index index, AKtuple fields ->
      begin match IMap.get index fields with
      | Some _ ->
        let tk = Reason.Ridx (fst key, fst ety1), Tprim Tint in
        let env = type_index env pos tkey tk Reason.index_array in
        let fields' = IMap.add index ty2 fields in
        env, ((fst ety1, Tarraykind (AKtuple fields')), ty2)
      | None -> fallback ()
      end
    | Typing_arrays.AKshape_key field_name, AKshape fdm ->
      begin match ShapeMap.get field_name fdm with
      | Some (tk, _) ->
        let env, tk' = Typing_union.union env tk tkey in
        let fdm' = ShapeMap.add field_name (tk', ty2) fdm in
        env, ((fst ety1, Tarraykind (AKshape fdm')), ty2)
      | None -> fallback ()
      end
    | _ -> fallback ()
    end
  | Terr -> error
  | Tdynamic ->
    let tv = Reason.Rwitness pos, Tdynamic in
    env, (ety1, tv)
  | (Tany | Tarraykind AKany) ->
    let tany = Reason.Rwitness pos, TUtils.tany env in
    env, (ety1, tany)
  | Tarraykind AKempty ->
    let tk = Reason.Rvarray_or_darray_key (Reason.to_pos (fst ety1)), Tprim Tarraykey in
    let env = type_index env pos tkey tk Reason.index_array in
    env, ((fst ety1, Tarraykind (AKvarray_or_darray ty2)), ty2)
  | Tprim Tstring ->
    let tk = Reason.Ridx (fst key, fst ety1), Tprim Tint in
    let tv = Reason.Rwitness pos, Tprim Tstring in
    let env = type_index env pos tkey tk Reason.index_array in
    let env = Typing_ops.sub_type pos ur env ty2 tv in
    env, (ety1, tv)
  | Ttuple tyl ->
    let fail reason =
      Errors.typing_error (fst key) (Reason.string_of_ureason reason);
      error in
    begin match key with
    | _, Int n ->
      (try
        let idx = int_of_string n in
        match List.split_n tyl idx with
        | (tyl', _ :: tyl'') ->
          env, ((fst ety1, Ttuple (tyl' @ ty2 :: tyl'')), ty2)
        | _ -> fail Reason.index_tuple
       with _ -> fail Reason.index_tuple)
    | _ -> fail Reason.URtuple_access
    end
  | Tshape (fields_known, fdm) ->
    begin match TUtils.shape_field_name env key with
    | None -> error
    | Some field ->
      let fields_known' = match fields_known with
        | FieldsFullyKnown -> FieldsFullyKnown
        | FieldsPartiallyKnown unset ->
          FieldsPartiallyKnown (ShapeMap.remove field unset) in
      let fdm' = ShapeMap.add field {sft_optional = false; sft_ty = ty2} fdm in
      env, ((fst ety1, Tshape (fields_known', fdm')), ty2)
    end
  | Tobject ->
    if Env.is_strict env
    then
      (Errors.array_access pos (Reason.to_pos (fst ety1)) (Typing_print.error (snd ety1));
      error)
    else
      env, (ety1, ty2)
  | Tabstract _ ->
    let resl = TUtils.try_over_concrete_supertypes env ty1 begin fun env ty1 ->
      assign_array_get pos ur env ty1 key tkey ty2
    end in
    begin match resl with
    | [res] -> res
    | (_, x as res)::rest
      when List.for_all rest ~f:(fun (_, y) ->
        ty_equal (fst x) (fst y) && ty_equal (snd x) (snd y)) -> res
    | _ ->
      Errors.array_access pos (Reason.to_pos (fst ety1)) (Typing_print.error (snd ety1));
      error
    end
  | (Toption _ | Tnonnull | Tprim _ |
     Tvar _ | Tfun _ | Tclass _ | Tanon _) ->
    Errors.array_access pos (Reason.to_pos (fst ety1)) (Typing_print.error (snd ety1));
    error
