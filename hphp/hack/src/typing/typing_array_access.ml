(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Common
open Aast
open Typing_defs
module Env = Typing_env
module TUtils = Typing_utils
module Reason = Typing_reason
module Union = Typing_union
module Inter = Typing_intersection
module MakeType = Typing_make_type
module SubType = Typing_subtype
module Partial = Partial_provider
module GenericRules = Typing_generic_rules

let err_witness env p = (Reason.Rwitness p, TUtils.terr env)

let error_array env p ty =
  Errors.array_access p (Reason.to_pos (fst ty)) (Typing_print.error env ty);
  (env, err_witness env p)

let error_const_mutation env p ty =
  Errors.const_mutation p (Reason.to_pos (fst ty)) (Typing_print.error env ty);
  (env, err_witness env p)

let error_assign_array_append env p ty =
  Errors.array_append p (Reason.to_pos (fst ty)) (Typing_print.error env ty);
  (env, ty)

(* Given a type `ty` known to be a lower bound on the type of the array operand
 * to an array get operation, compute the largest upper bound on that type
 * that validates the get operation. For example, if `vec<string>` is a lower
 * bound, then the type must be a subtype of `KeyedContainer<#1,#2>` for some
 * unresolved types #1 and #2. If `(string,int)` is a lower bound, then
 * `(#1, #2)` is a suitable upper bound. Shapes are the most complex. If
 * `shape(known_fields, ...)` is a lower bound (an open shape type), and the
 * access is through a literal field f, then `shape('f' => #1, ...)` is a
 * suitable upper bound.
 *)
let widen_for_array_get ~lhs_of_null_coalesce ~expr_pos index_expr env ty =
  Typing_log.(
    log_with_level env "typing" 1 (fun () ->
        log_types
          expr_pos
          env
          [Log_head ("widen_for_array_get", [Log_type ("ty", ty)])]));
  match ty with
  (* The null type is valid only with a null-coalescing use of array get *)
  | (_, Tprim Tnull) when lhs_of_null_coalesce -> (env, Some ty)
  (* All class-based containers, and keyset, vec and dict, are subtypes of
   * some instantiation of KeyedContainer
   *)
  | (r, Tclass ((_, cn), _, _))
    when cn = SN.Collections.cVector
         || cn = SN.Collections.cVec
         || cn = SN.Collections.cMap
         || cn = SN.Collections.cStableMap
         || cn = SN.Collections.cDict
         || cn = SN.Collections.cKeyset
         || cn = SN.Collections.cConstMap
         || cn = SN.Collections.cImmMap
         || cn = SN.Collections.cKeyedContainer
         || cn = SN.Collections.cConstVector
         || cn = SN.Collections.cImmVector ->
    let (env, element_ty) = Env.fresh_invariant_type_var env expr_pos in
    let (env, index_ty) = Env.fresh_invariant_type_var env expr_pos in
    let ty = MakeType.keyed_container r index_ty element_ty in
    (env, Some ty)
  (* The same is true of PHP arrays *)
  | (r, Tarraykind _) ->
    let (env, element_ty) = Env.fresh_invariant_type_var env expr_pos in
    let (env, index_ty) = Env.fresh_invariant_type_var env expr_pos in
    let ty = MakeType.keyed_container r index_ty element_ty in
    (env, Some ty)
  (* For tuples, we just freshen the element types *)
  | (r, Ttuple tyl) ->
    (* requires integer literal *)
    begin
      match index_expr with
      (* Should freshen type variables *)
      | (_, Int _) ->
        let (env, params) =
          List.map_env env tyl (fun env _ty ->
              Env.fresh_invariant_type_var env expr_pos)
        in
        (env, Some (r, Ttuple params))
      | _ -> (env, None)
    end
  (* Whatever the lower bound, construct an open, singleton shape type. *)
  | (r, Tshape (_, fdm)) ->
    begin
      match TUtils.shape_field_name env index_expr with
      | None -> (env, None)
      | Some field ->
        (match ShapeMap.get field fdm with
        (* If field is in the lower bound but is optional, then no upper bound makes sense
         * unless this is a null-coalesce access *)
        | Some { sft_optional = true; _ } when not lhs_of_null_coalesce ->
          (env, None)
        | _ ->
          let (env, element_ty) = Env.fresh_invariant_type_var env expr_pos in
          let upper_fdm =
            ShapeMap.add
              field
              { sft_optional = lhs_of_null_coalesce; sft_ty = element_ty }
              ShapeMap.empty
          in
          let upper_shape_ty = (r, Tshape (Open_shape, upper_fdm)) in
          (env, Some upper_shape_ty))
    end
  | _ -> (env, None)

(* Check that an index to a map-like collection passes the basic test of
 * being a subtype of arraykey
 *)
let check_arraykey_index env pos container_ty index_ty =
  if TypecheckerOptions.disallow_invalid_arraykey (Env.get_tcopt env) then
    let reason =
      match container_ty with
      | (_, Tclass ((_, cn), _, _)) -> Reason.index_class cn
      | _ -> Reason.index_array
    in
    let info_of_type ty =
      (Reason.to_pos (fst ty), Typing_print.error env ty)
    in
    let ty_arraykey = MakeType.arraykey (Reason.Ridx_dict pos) in
    (* Wrap generic type mismatch error with special error code *)
    Errors.try_
      (fun () ->
        Typing_coercion.coerce_type
          pos
          reason
          env
          index_ty
          { et_type = ty_arraykey; et_enforced = true }
          Errors.unify_error)
      (fun _ ->
        Errors.invalid_arraykey
          pos
          (info_of_type container_ty)
          (info_of_type index_ty);
        env)
  else
    env

let rec array_get
    ~array_pos
    ~expr_pos
    ?(lhs_of_null_coalesce = false)
    is_lvalue
    env
    ty1
    e2
    ty2 =
  let (env, ty1) =
    Typing_solver.expand_type_and_narrow
      env
      ~description_of_expected:"an array or collection"
      (widen_for_array_get ~lhs_of_null_coalesce ~expr_pos e2)
      array_pos
      ty1
      Errors.unify_error
  in
  GenericRules.apply_rules ~ignore_type_structure:true env ty1 (fun env ty1 ->
      let ((r, ety1_) as ety1) = ty1 in
      (* This is a little weird -- we enforce the right arity when you use certain
       * collections, even in partial mode (where normally completely omitting the
       * type parameter list is admitted). Basically the "omit type parameter"
       * hole was for compatibility with certain interfaces like Arrayaccess, not
       * for collections! But it's hard to go back on now, so since we've always
       * errored (with an inscrutable error message) when you try to actually use
       * a collection with omitted type parameters, we can continue to error and
       * give a more useful error message. *)
      let arity_error (_, name) =
        Errors.array_get_arity expr_pos name (Reason.to_pos r)
      in
      let nullable_container_get env ty =
        if
          lhs_of_null_coalesce
          (* Normally, we would not allow indexing into a nullable container,
         however, because the pattern shows up so frequently, we are allowing
         indexing into a nullable container as long as it is on the lhs of a
         null coalesce *)
        then
          array_get
            ~array_pos
            ~expr_pos
            ~lhs_of_null_coalesce
            is_lvalue
            env
            ty
            e2
            ty2
        else (
          Errors.null_container
            expr_pos
            (Reason.to_string "This is what makes me believe it can be null" r);
          (env, err_witness env expr_pos)
        )
      in
      let type_index env p ty_have ty_expect reason =
        Typing_log.(
          log_with_level env "typing" 1 (fun () ->
              log_types
                p
                env
                [
                  Log_head
                    ( "array_get/type_index",
                      [
                        Log_type ("ty_have", ty_have);
                        Log_type ("ty_expect", ty_expect);
                      ] );
                ]));

        (* coerce if possible *)
        match
          Typing_coercion.try_coerce
            p
            reason
            env
            ty_have
            { et_type = ty_expect; et_enforced = true }
        with
        | Some env -> env
        | None ->
          (* if subtype of dynamic, allow it to be used *)
          if
            Typing_solver.is_sub_type
              env
              ty_have
              (MakeType.dynamic Reason.none)
          then
            env
          (* fail with useful error *)
          else
            Typing_ops.sub_type
              p
              reason
              env
              ty_have
              ty_expect
              Errors.index_type_mismatch
      in
      match ety1_ with
      | Tarraykind (AKvarray ty | AKvec ty) ->
        let ty1 = MakeType.int (Reason.Ridx (fst e2, r)) in
        let env = type_index env expr_pos ty2 ty1 Reason.index_array in
        (env, ty)
      | Tarraykind (AKvarray_or_darray ty) ->
        let ty1 = MakeType.arraykey (Reason.Rvarray_or_darray_key expr_pos) in
        let env = type_index env expr_pos ty2 ty1 Reason.index_array in
        (env, ty)
      | Tclass (((_, cn) as id), _, argl)
        when cn = SN.Collections.cVector || cn = SN.Collections.cVec ->
        let ty =
          match argl with
          | [ty] -> ty
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let ty1 = MakeType.int (Reason.Ridx_vector (fst e2)) in
        let env = type_index env expr_pos ty2 ty1 (Reason.index_class cn) in
        (env, ty)
      | Tclass (((_, cn) as id), _, argl)
        when cn = SN.Collections.cMap
             || cn = SN.Collections.cStableMap
             || cn = SN.Collections.cDict
             || cn = SN.Collections.cKeyset ->
        if cn = SN.Collections.cKeyset && is_lvalue then (
          Errors.keyset_set expr_pos (Reason.to_pos r);
          (env, err_witness env expr_pos)
        ) else
          let (k, v) =
            match argl with
            | [t] when cn = SN.Collections.cKeyset -> (t, t)
            | [k; v] when cn <> SN.Collections.cKeyset -> (k, v)
            | _ ->
              arity_error id;
              let any = err_witness env expr_pos in
              (any, any)
          in
          (* dict and keyset are covariant in the key type, so subsumption
           * lets you upcast the key type beyond ty2 to arraykey.
           * e.g. consider $d: dict<string,int> and $i:int
           * and $d[$i] should actually type check because
           * dict<string,int> <: dict<arraykey,int>
           *)
          let (env, k) = Env.expand_type env k in
          let env =
            if cn = SN.Collections.cDict || cn = SN.Collections.cKeyset then
              check_arraykey_index env expr_pos ety1 ty2
            else
              type_index env expr_pos ty2 k (Reason.index_class cn)
          in
          (env, v)
      (* Certain container/collection types are intended to be immutable/const,
       * thus they should never appear as a lvalue when indexing i.e.
       *
       *   $x[0] = 100; // ERROR
       *   $x[0]; // OK
       *)
      | Tclass (((_, cn) as id), _, argl)
        when cn = SN.Collections.cConstMap
             || cn = SN.Collections.cImmMap
             || cn = SN.Collections.cKeyedContainer ->
        if is_lvalue then
          error_const_mutation env expr_pos ety1
        else
          let (_k, v) =
            match argl with
            | [k; v] -> (k, v)
            | _ ->
              arity_error id;
              let any = err_witness env expr_pos in
              (any, any)
          in
          (env, v)
      | Tclass (((_, cn) as id), _, argl)
        when (not is_lvalue)
             && ( cn = SN.Collections.cConstVector
                || cn = SN.Collections.cImmVector ) ->
        let ty =
          match argl with
          | [ty] -> ty
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let ty1 = MakeType.int (Reason.Ridx (fst e2, r)) in
        let env = type_index env expr_pos ty2 ty1 (Reason.index_class cn) in
        (env, ty)
      | Tclass ((_, cn), _, _)
        when is_lvalue
             && ( cn = SN.Collections.cConstVector
                || cn = SN.Collections.cImmVector ) ->
        error_const_mutation env expr_pos ety1
      | Tarraykind (AKdarray (_k, v) | AKmap (_k, v)) ->
        let env = check_arraykey_index env expr_pos ety1 ty2 in
        (env, v)
      | Terr -> (env, err_witness env expr_pos)
      | Tdynamic -> (env, ety1)
      | Tany _ -> (env, (Reason.Rnone, TUtils.tany env))
      | Tarraykind (AKany | AKempty) ->
        let env = check_arraykey_index env expr_pos ety1 ty2 in
        (env, (Reason.Rnone, TUtils.tany env))
      | Tprim Tstring ->
        let ty = MakeType.string (Reason.Rwitness expr_pos) in
        let ty1 = MakeType.int (Reason.Ridx (fst e2, r)) in
        let env = type_index env expr_pos ty2 ty1 Reason.index_array in
        (env, ty)
      | Ttuple tyl ->
        (* requires integer literal *)
        (match e2 with
        | (p, Int n) ->
          (try
             let idx = int_of_string n in
             let nth = List.nth_exn tyl idx in
             (env, nth)
           with _ ->
             Errors.typing_error
               p
               (Reason.string_of_ureason Reason.index_tuple);
             (env, err_witness env p))
        | (p, _) ->
          Errors.typing_error
            p
            (Reason.string_of_ureason Reason.URtuple_access);
          (env, err_witness env p))
      | Tclass (((_, cn) as id), _, argl) when cn = SN.Collections.cPair ->
        let (ty1, ty2) =
          match argl with
          | [ty1; ty2] -> (ty1, ty2)
          | _ ->
            arity_error id;
            let any = err_witness env expr_pos in
            (any, any)
        in
        (* requires integer literal *)
        (match e2 with
        | (p, Int n) ->
          (try
             let idx = int_of_string n in
             let nth = List.nth_exn [ty1; ty2] idx in
             (env, nth)
           with _ ->
             Errors.typing_error p
             @@ Reason.string_of_ureason (Reason.index_class cn);
             (env, err_witness env p))
        | (p, _) ->
          Errors.typing_error p (Reason.string_of_ureason Reason.URpair_access);
          (env, err_witness env p))
      | Tshape (_, fdm) ->
        if is_lvalue || lhs_of_null_coalesce then
          (* The expression $s['x'] ?? $y is semantically equivalent to
           Shapes::idx ($s, 'x') ?? $y.  I.e., if $s['x'] occurs on
           the left of a coalesce operator, then for type checking it
           can be treated as if it evaluated to null instead of
           throwing an exception if the field 'x' doesn't exist in $s.
         *)
          Typing_shapes.idx
            env
            ty1
            e2
            None
            ~expr_pos
            ~fun_pos:Reason.Rnone
            ~shape_pos:array_pos
        else
          let p = fst e2 in
          begin
            match TUtils.shape_field_name env e2 with
            | None ->
              (* there was already an error in shape_field name,
               don't report another one for a missing field *)
              (env, err_witness env p)
            | Some field ->
              begin
                match ShapeMap.get field fdm with
                | None ->
                  Errors.undefined_field
                    ~use_pos:p
                    ~name:(TUtils.get_printable_shape_field_name field)
                    ~shape_type_pos:(Reason.to_pos r);
                  (env, err_witness env p)
                | Some { sft_optional; sft_ty } ->
                  if sft_optional then (
                    let declared_field =
                      List.find_exn
                        ~f:(fun x -> Ast_defs.ShapeField.compare field x = 0)
                        (ShapeMap.keys fdm)
                    in
                    Errors.array_get_with_optional_field
                      p
                      (Env.get_shape_field_name_pos declared_field)
                      (TUtils.get_printable_shape_field_name field);
                    (env, err_witness env p)
                  ) else
                    (env, sft_ty)
              end
          end
      | Toption ty -> nullable_container_get env ty
      | Tprim Tnull ->
        let ty = (Reason.Rnone, Tunion []) in
        nullable_container_get env ty
      | Tobject ->
        if Partial.should_check_error (Env.get_mode env) 4005 then
          error_array env expr_pos ety1
        else
          (env, (Reason.Rwitness expr_pos, TUtils.tany env))
      | Tabstract (AKnewtype (ts, [ty]), Some (r, Tshape (shape_kind, fields)))
        when ts = SN.FB.cTypeStructure ->
        let (env, fields) =
          Typing_structure.transform_shapemap env array_pos ty fields
        in
        let ty = (r, Tshape (shape_kind, fields)) in
        array_get
          ~array_pos
          ~expr_pos
          ~lhs_of_null_coalesce
          is_lvalue
          env
          ty
          e2
          ty2
      | Tnonnull
      | Tprim _
      | Tfun _
      | Tdestructure _
      | Tclass _
      | Tanon _
      | Tabstract _
      | Tunion _
      | Tintersection _
      | Tpu _
      | Tpu_access _ ->
        error_array env expr_pos ety1
      (* Type-check array access as though it is the method
       * array_get<Tk,Tv>(KeyedContainer<Tk,Tv> $array, Tk $key): Tv
       * (We can already force Tk to be the type of the key argument because
       * Tk does not appear in the result of the call)
       *)
      | Tvar _ ->
        let (env, value) = Env.fresh_type env expr_pos in
        let keyed_container = MakeType.keyed_container r ty2 value in
        let env =
          SubType.sub_type env ty1 keyed_container Errors.index_type_mismatch
        in
        (env, value))

(* Given a type `ty` known to be a lower bound on the type of the array operand
 * to an array append operation, compute the largest upper bound on that type
 * that validates the get operation. For example, if `vec<string>` is a lower
 * bound, then `vec<#1>` is a suitable upper bound.`
 *)
let widen_for_assign_array_append ~expr_pos env ty =
  match ty with
  | (r, Tclass (((_, cn) as id), _, tyl))
    when cn = SN.Collections.cVec
         || cn = SN.Collections.cKeyset
         || cn = SN.Collections.cVector
         || cn = SN.Collections.cMap ->
    let (env, params) =
      List.map_env env tyl (fun env _ty ->
          Env.fresh_invariant_type_var env expr_pos)
    in
    let ty = (r, Tclass (id, Nonexact, params)) in
    (env, Some ty)
  | (r, Tarraykind (AKvec _)) ->
    let (env, element_ty) = Env.fresh_invariant_type_var env expr_pos in
    (env, Some (r, Tarraykind (AKvec element_ty)))
  | (r, Tarraykind (AKvarray _)) ->
    let (env, element_ty) = Env.fresh_invariant_type_var env expr_pos in
    (env, Some (r, Tarraykind (AKvarray element_ty)))
  | _ -> (env, None)

let assign_array_append ~array_pos ~expr_pos ur env ty1 ty2 =
  let (env, ty1) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"an array or collection"
      env
      (widen_for_assign_array_append ~expr_pos)
      array_pos
      ty1
      Errors.unify_error
  in
  GenericRules.apply_rules env ty1 (fun env ty1 ->
      match ty1 with
      | (_, (Tany _ | Tarraykind (AKany | AKempty))) -> (env, ty1)
      | (_, Terr) -> (env, ty1)
      | (_, Tclass ((_, n), _, [tv]))
        when n = SN.Collections.cVector || n = SN.Collections.cSet ->
        let env =
          Typing_ops.sub_type expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ty1)
      (* Handle the case where Vector or Set was used as a typehint
       without type parameters *)
      | (_, Tclass ((_, n), _, []))
        when n = SN.Collections.cVector || n = SN.Collections.cSet ->
        (env, ty1)
      | (_, Tclass ((_, n), _, [tk; tv])) when n = SN.Collections.cMap ->
        let tpair = MakeType.pair (Reason.Rmap_append expr_pos) tk tv in
        let env =
          Typing_ops.sub_type expr_pos ur env ty2 tpair Errors.unify_error
        in
        (env, ty1)
      (* Handle the case where Map was used as a typehint without
       type parameters *)
      | (_, Tclass ((_, n), _, [])) when n = SN.Collections.cMap ->
        let tpair =
          MakeType.class_type
            (Reason.Rmap_append expr_pos)
            SN.Collections.cPair
            []
        in
        let env =
          Typing_ops.sub_type expr_pos ur env ty2 tpair Errors.unify_error
        in
        (env, ty1)
      | (r, Tclass (((_, n) as id), e, [tv]))
        when n = SN.Collections.cVec || n = SN.Collections.cKeyset ->
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tclass (id, e, [tv'])))
      | (r, Tarraykind (AKvec tv)) ->
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tarraykind (AKvec tv')))
      | (r, Tarraykind (AKvarray tv)) ->
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tarraykind (AKvarray tv')))
      | (_, Tdynamic) -> (env, ty1)
      | (_, Tobject) ->
        if Partial.should_check_error (Env.get_mode env) 4006 then
          error_assign_array_append env expr_pos ty1
        else
          (env, ty1)
      | ( _,
          ( Tnonnull | Tarraykind _ | Toption _ | Tprim _ | Tvar _ | Tfun _
          | Tclass _ | Ttuple _ | Tanon _ | Tshape _ | Tdestructure _
          | Tunion _ | Tintersection _ | Tabstract _ | Tpu _ | Tpu_access _ )
        ) ->
        error_assign_array_append env expr_pos ty1)

let widen_for_assign_array_get ~expr_pos index_expr env ty =
  Typing_log.(
    log_with_level env "typing" 1 (fun () ->
        log_types
          expr_pos
          env
          [Log_head ("widen_for_assign_array_get", [Log_type ("ty", ty)])]));
  match ty with
  | (r, Tclass (((_, cn) as id), _, tyl))
    when cn = SN.Collections.cVec
         || cn = SN.Collections.cKeyset
         || cn = SN.Collections.cVector
         || cn = SN.Collections.cDict
         || cn = SN.Collections.cMap
         || cn = SN.Collections.cStableMap ->
    let (env, params) =
      List.map_env env tyl (fun env _ty ->
          Env.fresh_invariant_type_var env expr_pos)
    in
    let ty = (r, Tclass (id, Nonexact, params)) in
    (env, Some ty)
  | (r, Tarraykind (AKvarray _)) ->
    let (env, tv) = Env.fresh_invariant_type_var env expr_pos in
    (env, Some (r, Tarraykind (AKvarray tv)))
  | (r, Tarraykind (AKvec _)) ->
    let (env, tv) = Env.fresh_invariant_type_var env expr_pos in
    (env, Some (r, Tarraykind (AKvec tv)))
  | (r, Tarraykind (AKvarray_or_darray _)) ->
    let (env, tv) = Env.fresh_invariant_type_var env expr_pos in
    (env, Some (r, Tarraykind (AKvarray_or_darray tv)))
  | (r, Tarraykind (AKdarray _)) ->
    let (env, tk) = Env.fresh_invariant_type_var env expr_pos in
    let (env, tv) = Env.fresh_invariant_type_var env expr_pos in
    (env, Some (r, Tarraykind (AKdarray (tk, tv))))
  | (r, Tarraykind (AKmap _)) ->
    let (env, tk) = Env.fresh_invariant_type_var env expr_pos in
    let (env, tv) = Env.fresh_invariant_type_var env expr_pos in
    (env, Some (r, Tarraykind (AKmap (tk, tv))))
  | (r, Ttuple tyl) ->
    (* requires integer literal *)
    begin
      match index_expr with
      (* Should freshen type variables *)
      | (_, Int _) ->
        let (env, params) =
          List.map_env env tyl (fun env _ty ->
              Env.fresh_invariant_type_var env expr_pos)
        in
        (env, Some (r, Ttuple params))
      | _ -> (env, None)
    end
  | _ -> (env, None)

(* Used for typing an assignment e1[key] = e2
 * where e1 has type ty1, key has type tkey and e2 has type ty2.
 * Return the new array type
 *)
let assign_array_get ~array_pos ~expr_pos ur env ty1 key tkey ty2 =
  let (env, ety1) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"an array or collection"
      env
      (widen_for_assign_array_get ~expr_pos key)
      array_pos
      ty1
      Errors.unify_error
  in
  GenericRules.apply_rules env ety1 (fun env ety1 ->
      let (r, ety1_) = ety1 in
      let arity_error (_, name) =
        Errors.array_get_arity expr_pos name (Reason.to_pos r)
      in
      let type_index env p ty_have ty_expect reason =
        (* coerce if possible *)
        match
          Typing_coercion.try_coerce
            p
            reason
            env
            ty_have
            { et_type = ty_expect; et_enforced = true }
        with
        | Some e -> e
        | None ->
          (* if subtype of dynamic, allow it to be used *)
          if
            Typing_solver.is_sub_type
              env
              ty_have
              (MakeType.dynamic Reason.none)
          then
            env
          (* fail with useful error *)
          else
            Typing_ops.sub_type
              p
              reason
              env
              ty_have
              ty_expect
              Errors.index_type_mismatch
      in
      let error = (env, ety1) in
      match ety1_ with
      | Tarraykind (AKvarray tv) ->
        let tk = MakeType.int (Reason.Ridx (fst key, r)) in
        let env = type_index env expr_pos tkey tk Reason.index_array in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tarraykind (AKvarray tv')))
      | Tarraykind (AKvec tv) ->
        let tk = MakeType.int (Reason.Ridx (fst key, r)) in
        let env = type_index env expr_pos tkey tk Reason.index_array in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tarraykind (AKvec tv')))
      | Tarraykind (AKvarray_or_darray tv) ->
        let tk =
          MakeType.arraykey (Reason.Rvarray_or_darray_key (Reason.to_pos r))
        in
        let env = type_index env expr_pos tkey tk Reason.index_array in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tarraykind (AKvarray_or_darray tv')))
      | Tclass (((_, cn) as id), _, argl) when cn = SN.Collections.cVector ->
        let tv =
          match argl with
          | [tv] -> tv
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let tk = MakeType.int (Reason.Ridx_vector (fst key)) in
        let env = type_index env expr_pos tkey tk (Reason.index_class cn) in
        let env =
          Typing_ops.sub_type expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ety1)
      | Tclass (((_, cn) as id), e, argl) when cn = SN.Collections.cVec ->
        let tv =
          match argl with
          | [tv] -> tv
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let tk = MakeType.int (Reason.Ridx_vector (fst key)) in
        let env = type_index env expr_pos tkey tk (Reason.index_class cn) in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tclass (id, e, [tv'])))
      | Tclass (((_, cn) as id), _, argl)
        when cn = SN.Collections.cMap || cn = SN.Collections.cStableMap ->
        let env = check_arraykey_index env expr_pos ety1 tkey in
        let (tk, tv) =
          match argl with
          | [tk; tv] -> (tk, tv)
          | _ ->
            arity_error id;
            let any = err_witness env expr_pos in
            (any, any)
        in
        let env = type_index env expr_pos tkey tk (Reason.index_class cn) in
        let env =
          Typing_ops.sub_type expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ety1)
      | Tclass (((_, cn) as id), e, argl) when cn = SN.Collections.cDict ->
        let env = check_arraykey_index env expr_pos ety1 tkey in
        let (tk, tv) =
          match argl with
          | [tk; tv] -> (tk, tv)
          | _ ->
            arity_error id;
            let any = err_witness env expr_pos in
            (any, any)
        in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tclass (id, e, [tk'; tv'])))
      | Tclass ((_, cn), _, _) when cn = SN.Collections.cKeyset ->
        Errors.keyset_set expr_pos (Reason.to_pos r);
        error
      | Tclass ((_, cn), _, _)
        when cn = SN.Collections.cConstMap
             || cn = SN.Collections.cImmMap
             || cn = SN.Collections.cKeyedContainer
             || cn = SN.Collections.cConstVector
             || cn = SN.Collections.cImmVector
             || cn = SN.Collections.cPair ->
        Errors.const_mutation
          expr_pos
          (Reason.to_pos r)
          (Typing_print.error env ety1);
        error
      | Tarraykind (AKdarray (tk, tv)) ->
        let env = check_arraykey_index env expr_pos ety1 tkey in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tarraykind (AKdarray (tk', tv'))))
      | Tarraykind (AKmap (tk, tv)) ->
        let env = check_arraykey_index env expr_pos ety1 tkey in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, (r, Tarraykind (AKmap (tk', tv'))))
      | Terr -> error
      | Tdynamic -> (env, ety1)
      | Tany _
      | Tarraykind AKany ->
        (env, ety1)
      | Tarraykind AKempty ->
        let env = check_arraykey_index env expr_pos ety1 tkey in
        let tk =
          MakeType.arraykey (Reason.Rvarray_or_darray_key (Reason.to_pos r))
        in
        let env = type_index env expr_pos tkey tk Reason.index_array in
        (env, (r, Tarraykind (AKvarray_or_darray ty2)))
      | Tprim Tstring ->
        let tk = MakeType.int (Reason.Ridx (fst key, r)) in
        let tv = MakeType.string (Reason.Rwitness expr_pos) in
        let env = type_index env expr_pos tkey tk Reason.index_array in
        let env =
          Typing_ops.sub_type expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ety1)
      | Ttuple tyl ->
        let fail reason =
          Errors.typing_error (fst key) (Reason.string_of_ureason reason);
          error
        in
        begin
          match key with
          | (_, Int n) ->
            (try
               let idx = int_of_string n in
               match List.split_n tyl idx with
               | (tyl', _ :: tyl'') ->
                 (env, (r, Ttuple (tyl' @ (ty2 :: tyl''))))
               | _ -> fail Reason.index_tuple
             with _ -> fail Reason.index_tuple)
          | _ -> fail Reason.URtuple_access
        end
      | Tshape (shape_kind, fdm) ->
        begin
          match TUtils.shape_field_name env key with
          | None -> error
          | Some field ->
            let fdm' =
              ShapeMap.add field { sft_optional = false; sft_ty = ty2 } fdm
            in
            (env, (r, Tshape (shape_kind, fdm')))
        end
      | Tobject ->
        if Partial.should_check_error (Env.get_mode env) 4005 then (
          Errors.array_access
            expr_pos
            (Reason.to_pos r)
            (Typing_print.error env ety1);
          error
        ) else
          (env, ety1)
      | Toption _
      | Tnonnull
      | Tprim _
      | Tdestructure _
      | Tunion _
      | Tintersection _
      | Tabstract _
      | Tvar _
      | Tfun _
      | Tclass _
      | Tanon _
      | Tpu _
      | Tpu_access _ ->
        Errors.array_access
          expr_pos
          (Reason.to_pos r)
          (Typing_print.error env ety1);
        error)
