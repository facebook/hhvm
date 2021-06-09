(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Aast
open Typing_defs
module Env = Typing_env
module TUtils = Typing_utils
module Reason = Typing_reason
module MakeType = Typing_make_type
module SubType = Typing_subtype
module Partial = Partial_provider
module GenericRules = Typing_generic_rules
module SN = Naming_special_names
open String.Replace_polymorphic_compare

let err_witness env p = TUtils.terr env (Reason.Rwitness p)

let error_array env p ty =
  Errors.array_access_read p (get_pos ty) (Typing_print.error env ty);
  (env, err_witness env p)

let error_const_mutation env p ty =
  Errors.const_mutation p (get_pos ty) (Typing_print.error env ty);
  (env, err_witness env p)

let error_assign_array_append env p ty =
  Errors.array_append p (get_pos ty) (Typing_print.error env ty);
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
    log_with_level env "typing" ~level:1 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos expr_pos)
          env
          [Log_head ("widen_for_array_get", [Log_type ("ty", ty)])]));
  match deref ty with
  (* The null type is valid only with a null-coalescing use of array get *)
  | (_, Tprim Tnull) when lhs_of_null_coalesce -> (env, Some ty)
  (* dynamic is valid for array get *)
  | (_, Tdynamic) -> (env, Some ty)
  (* All class-based containers, and keyset, vec and dict, are subtypes of
   * some instantiation of KeyedContainer
   *)
  | (r, Tclass ((_, cn), _, _))
    when cn = SN.Collections.cVector
         || cn = SN.Collections.cVec
         || cn = SN.Collections.cMap
         || cn = SN.Collections.cDict
         || cn = SN.Collections.cKeyset
         || cn = SN.Collections.cConstMap
         || cn = SN.Collections.cImmMap
         || cn = SN.Collections.cKeyedContainer
         || cn = SN.Collections.cAnyArray
         || cn = SN.Collections.cConstVector
         || cn = SN.Collections.cImmVector ->
    let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
    let (env, index_ty) = Env.fresh_type_invariant env expr_pos in
    let ty = MakeType.keyed_container r index_ty element_ty in
    (env, Some ty)
  (* The same is true of PHP arrays *)
  | (r, (Tvarray _ | Tdarray _ | Tvec_or_dict _ | Tvarray_or_darray _)) ->
    let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
    let (env, index_ty) = Env.fresh_type_invariant env expr_pos in
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
          List.map_env env tyl ~f:(fun env _ty ->
              Env.fresh_type_invariant env expr_pos)
        in
        (env, Some (MakeType.tuple r params))
      | _ -> (env, None)
    end
  (* Whatever the lower bound, construct an open, singleton shape type. *)
  | (r, Tshape (_, fdm)) ->
    begin
      match TUtils.shape_field_name env index_expr with
      | None -> (env, None)
      | Some field ->
        let field = TShapeField.of_ast Pos_or_decl.of_raw_pos field in
        (match TShapeMap.find_opt field fdm with
        (* If field is in the lower bound but is optional, then no upper bound makes sense
         * unless this is a null-coalesce access *)
        | Some { sft_optional = true; _ } when not lhs_of_null_coalesce ->
          (env, None)
        | _ ->
          let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
          let upper_fdm =
            TShapeMap.add
              field
              { sft_optional = lhs_of_null_coalesce; sft_ty = element_ty }
              TShapeMap.empty
          in
          let upper_shape_ty = mk (r, Tshape (Open_shape, upper_fdm)) in
          (env, Some upper_shape_ty))
    end
  | _ -> (env, None)

(* Check that an index to a map-like collection passes the basic test of
 * being a subtype of arraykey
 *)
let check_arraykey_index error env pos container_ty index_ty =
  if TypecheckerOptions.disallow_invalid_arraykey (Env.get_tcopt env) then
    let (env, container_ty) = Env.expand_type env container_ty in
    let reason =
      match get_node container_ty with
      | Tclass ((_, cn), _, _) -> Reason.index_class cn
      | _ -> Reason.index_array
    in
    let info_of_type ty = (get_pos ty, Typing_print.error env ty) in
    let ty_arraykey = MakeType.arraykey (Reason.Ridx_dict pos) in
    (* Wrap generic type mismatch error with special error code *)
    Result.fold
      ~ok:(fun env -> (env, Ok index_ty))
      ~error:(fun env -> (env, Error (index_ty, ty_arraykey)))
    @@ Typing_coercion.coerce_type_res
         pos
         reason
         env
         index_ty
         { et_type = ty_arraykey; et_enforced = Enforced }
         (fun ?code:_ _ _ ->
           error pos (info_of_type container_ty) (info_of_type index_ty))
  else
    (env, Ok index_ty)

let check_arraykey_index_read =
  check_arraykey_index Errors.invalid_arraykey_read

let check_arraykey_index_write =
  check_arraykey_index Errors.invalid_arraykey_write

let check_keyset_value = check_arraykey_index Errors.invalid_keyset_value

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
  in
  GenericRules.apply_rules_with_err
    ~ignore_type_structure:true
    env
    ty1
    (fun env ty1 ->
      let (r, ety1_) = deref ty1 in
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
            (Reason.to_string
               "This is what makes me believe it can be `null`."
               r);
          (env, err_witness env expr_pos, None)
        )
      in
      let type_index env p ty_have ty_expect reason =
        Typing_log.(
          log_with_level env "typing" ~level:1 (fun () ->
              log_types
                (Pos_or_decl.of_raw_pos p)
                env
                [
                  Log_head
                    ( "array_get/type_index",
                      [
                        Log_type ("ty_have", ty_have);
                        Log_type ("ty_expect", ty_expect.et_type);
                      ] );
                ]));
        Result.fold
          ~ok:(fun env -> (env, Ok ty_have))
          ~error:(fun env -> (env, Error (ty_have, ty_expect.et_type)))
        @@ Typing_coercion.coerce_type_res
             p
             reason
             env
             ty_have
             ty_expect
             Errors.index_type_mismatch
      in
      match ety1_ with
      | Tvarray ty ->
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx (fst e2, r))) in
        let (env, err_res) =
          type_index env expr_pos ty2 ty1 Reason.index_array
        in
        (env, ty, err_res)
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cVector
             || String.equal cn SN.Collections.cVec ->
        let ty =
          match argl with
          | [ty] -> ty
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let ty1 =
          MakeType.enforced (MakeType.int (Reason.Ridx_vector (fst e2)))
        in
        let (env, err_res) =
          type_index env expr_pos ty2 ty1 (Reason.index_class cn)
        in
        (env, ty, err_res)
      | Tclass (((_, cn) as id), _, argl)
        when cn = SN.Collections.cMap
             || cn = SN.Collections.cDict
             || cn = SN.Collections.cKeyset ->
        if cn = SN.Collections.cKeyset && is_lvalue then (
          Errors.keyset_set expr_pos (Reason.to_pos r);
          (env, err_witness env expr_pos, Ok ty2)
        ) else
          let (k, v) =
            match argl with
            | [t] when String.equal cn SN.Collections.cKeyset -> (t, t)
            | [k; v] when String.( <> ) cn SN.Collections.cKeyset -> (k, v)
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
          let (env, err_res1) =
            check_arraykey_index_read env expr_pos ty1 ty2
          in
          let (env, err_res2) =
            if String.equal cn SN.Collections.cMap then
              type_index
                env
                expr_pos
                ty2
                (MakeType.enforced k)
                (Reason.index_class cn)
            else
              (env, Ok ty2)
          in
          let err_res =
            match (err_res1, err_res2) with
            | (Error _, _) -> err_res1
            | (_, _) -> err_res2
          in
          (env, v, err_res)
      (* Certain container/collection types are intended to be immutable/const,
       * thus they should never appear as a lvalue when indexing i.e.
       *
       *   $x[0] = 100; // ERROR
       *   $x[0]; // OK
       *)
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cConstMap
             || String.equal cn SN.Collections.cImmMap
             || String.equal cn SN.Collections.cKeyedContainer
             || String.equal cn SN.Collections.cAnyArray ->
        if is_lvalue then
          let (env, ty1) = error_const_mutation env expr_pos ty1 in
          (env, ty1, Ok ty2)
        else
          let (_k, v) =
            match argl with
            | [k; v] -> (k, v)
            | _ ->
              arity_error id;
              let any = err_witness env expr_pos in
              (any, any)
          in
          let (env, err_res) = check_arraykey_index_read env expr_pos ty1 ty2 in
          (env, v, err_res)
      | Tclass (((_, cn) as id), _, argl)
        when (not is_lvalue)
             && ( String.equal cn SN.Collections.cConstVector
                || String.equal cn SN.Collections.cImmVector ) ->
        let ty =
          match argl with
          | [ty] -> ty
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx (fst e2, r))) in
        let (env, err_res) =
          type_index env expr_pos ty2 ty1 (Reason.index_class cn)
        in
        (env, ty, err_res)
      | Tclass ((_, cn), _, _)
        when is_lvalue
             && ( String.equal cn SN.Collections.cConstVector
                || String.equal cn SN.Collections.cImmVector ) ->
        let (env, ty1) = error_const_mutation env expr_pos ty1 in
        (env, ty1, Ok ty2)
      | Tdarray (_k, v)
      | Tvec_or_dict (_k, v)
      | Tvarray_or_darray (_k, v) ->
        let (env, err_res) = check_arraykey_index_read env expr_pos ty1 ty2 in
        (env, v, err_res)
      | Terr -> (env, err_witness env expr_pos, Ok ty2)
      | Tdynamic -> (env, ty1, Ok ty2)
      | Tany _ -> (env, TUtils.mk_tany env expr_pos, Ok ty2)
      | Tprim Tstring ->
        let ty = MakeType.string (Reason.Rwitness expr_pos) in
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx (fst e2, r))) in
        let (env, err_res) =
          type_index env expr_pos ty2 ty1 Reason.index_array
        in
        (env, ty, err_res)
      | Ttuple tyl ->
        (* requires integer literal *)
        (match e2 with
        | (p, Int n) ->
          let idx = int_of_string_opt n in
          (match Option.bind idx ~f:(List.nth tyl) with
          | Some nth -> (env, nth, Ok ty2)
          | None ->
            Errors.typing_error p (Reason.string_of_ureason Reason.index_tuple);
            (env, err_witness env p, Ok ty2))
        | (p, _) ->
          Errors.typing_error p (Reason.string_of_ureason Reason.URtuple_access);
          (env, err_witness env p, Error (ty2, MakeType.int Reason.none)))
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cPair ->
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
          let idx = int_of_string_opt n in
          (match Option.bind ~f:(List.nth [ty1; ty2]) idx with
          | Some nth -> (env, nth, Ok ty2)
          | None ->
            Errors.typing_error p
            @@ Reason.string_of_ureason (Reason.index_class cn);
            (env, err_witness env p, Ok ty2))
        | (p, _) ->
          Errors.typing_error p (Reason.string_of_ureason Reason.URpair_access);
          (env, err_witness env p, Error (ty2, MakeType.int Reason.none)))
      | Tshape (_, fdm) ->
        if is_lvalue || lhs_of_null_coalesce then
          (* The expression $s['x'] ?? $y is semantically equivalent to
           Shapes::idx ($s, 'x') ?? $y.  I.e., if $s['x'] occurs on
           the left of a coalesce operator, then for type checking it
           can be treated as if it evaluated to null instead of
           throwing an exception if the field 'x' doesn't exist in $s.
         *)
          let (env, ty) =
            Typing_shapes.idx
              env
              ty1
              e2
              None
              ~expr_pos
              ~fun_pos:Reason.Rnone
              ~shape_pos:array_pos
          in
          (env, ty, Ok ty2)
        else
          let p = fst e2 in
          begin
            match TUtils.shape_field_name env e2 with
            | None ->
              (* there was already an error in shape_field name,
               don't report another one for a missing field *)
              (env, err_witness env p, Ok ty2)
            | Some field ->
              let field = TShapeField.of_ast Pos_or_decl.of_raw_pos field in
              begin
                match TShapeMap.find_opt field fdm with
                | None ->
                  Errors.undefined_field
                    ~use_pos:p
                    ~name:(TUtils.get_printable_shape_field_name field)
                    ~shape_type_pos:(Reason.to_pos r);
                  (env, err_witness env p, Ok ty2)
                | Some { sft_optional; sft_ty } ->
                  if sft_optional then (
                    let declared_field =
                      List.find_exn
                        ~f:(fun x -> TShapeField.equal field x)
                        (TShapeMap.keys fdm)
                    in
                    Errors.array_get_with_optional_field
                      p
                      (Env.get_shape_field_name_pos declared_field)
                      (TUtils.get_printable_shape_field_name field);
                    (env, err_witness env p, Ok ty2)
                  ) else
                    (env, sft_ty, Ok ty2)
              end
          end
      | Toption ty ->
        let (env, ty, err_opt) = nullable_container_get env ty in
        let err_res =
          Option.value_map
            err_opt
            ~default:(Ok ty2)
            ~f:(fun (ty_have, ty_expect) -> Error (ty_have, ty_expect))
        in
        (env, ty, err_res)
      | Tprim Tnull ->
        let ty = MakeType.nothing Reason.Rnone in
        let (env, ty, err_opt) = nullable_container_get env ty in
        let err_res =
          Option.value_map
            err_opt
            ~default:(Ok ty2)
            ~f:(fun (ty_have, ty_expect) -> Error (ty_have, ty_expect))
        in
        (env, ty, err_res)
      | Tobject ->
        if Partial.should_check_error (Env.get_mode env) 4005 then
          let (env, ty1) = error_array env expr_pos ty1 in
          (env, ty1, Ok ty2)
        else
          (env, TUtils.mk_tany env expr_pos, Ok ty2)
      | Tnewtype (ts, [ty], bound) ->
        begin
          match deref bound with
          | (r, Tshape (shape_kind, fields))
            when String.equal ts SN.FB.cTypeStructure ->
            let (env, fields) =
              Typing_structure.transform_shapemap env array_pos ty fields
            in
            let ty = mk (r, Tshape (shape_kind, fields)) in
            let (env, ty, err_opt) =
              array_get
                ~array_pos
                ~expr_pos
                ~lhs_of_null_coalesce
                is_lvalue
                env
                ty
                e2
                ty2
            in
            let err_res =
              Option.value_map
                err_opt
                ~default:(Ok ty2)
                ~f:(fun (ty_have, ty_expect) -> Error (ty_have, ty_expect))
            in
            (env, ty, err_res)
          | _ ->
            let (env, ty1) = error_array env expr_pos ty1 in
            (env, ty1, Ok ty2)
        end
      | Tunapplied_alias _ ->
        Typing_defs.error_Tunapplied_alias_in_illegal_context ()
      | Tnonnull
      | Tprim _
      | Tfun _
      | Tclass _
      | Tgeneric _
      | Tnewtype _
      | Tdependent _
      | Tunion _
      | Tintersection _
      | Taccess _
      | Tneg _ ->
        let (env, ty1) = error_array env expr_pos ty1 in
        (env, ty1, Ok ty2)
      (* Type-check array access as though it is the method
       * array_get<Tk,Tv>(KeyedContainer<Tk,Tv> $array, Tk $key): Tv
       * (We can already force Tk to be the type of the key argument because
       * Tk does not appear in the result of the call)
       *)
      | Tvar _ ->
        let (env, value) = Env.fresh_type env expr_pos in
        let keyed_container = MakeType.keyed_container r ty2 value in
        let env =
          SubType.sub_type
            env
            ty1
            keyed_container
            (Errors.index_type_mismatch_at expr_pos)
        in
        (env, value, Ok ty2))

(* Given a type `ty` known to be a lower bound on the type of the array operand
 * to an array append operation, compute the largest upper bound on that type
 * that validates the get operation. For example, if `vec<string>` is a lower
 * bound, then `vec<#1>` is a suitable upper bound.`
 *)
let widen_for_assign_array_append ~expr_pos env ty =
  match deref ty with
  (* dynamic is valid for array append *)
  | (_, Tdynamic) -> (env, Some ty)
  | (r, Tclass (((_, cn) as id), _, tyl))
    when String.equal cn SN.Collections.cVec
         || String.equal cn SN.Collections.cKeyset
         || String.equal cn SN.Collections.cVector
         || String.equal cn SN.Collections.cMap ->
    let (env, params) =
      List.map_env env tyl ~f:(fun env _ty ->
          Env.fresh_type_invariant env expr_pos)
    in
    let ty = mk (r, Tclass (id, Nonexact, params)) in
    (env, Some ty)
  | (r, Tvarray _) ->
    let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
    (env, Some (mk (r, Tvarray element_ty)))
  | _ -> (env, None)

let assign_array_append_with_err ~array_pos ~expr_pos ur env ty1 ty2 =
  let (env, ty1) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"an array or collection"
      env
      (widen_for_assign_array_append ~expr_pos)
      array_pos
      ty1
  in
  GenericRules.apply_rules_with_err env ty1 (fun env ty1 ->
      match deref ty1 with
      | (_, Tany _) -> (env, ty1, Ok ty2)
      | (_, Terr) -> (env, ty1, Ok ty2)
      | (_, Tclass ((_, n), _, [tv]))
        when String.equal n SN.Collections.cVector
             || String.equal n SN.Collections.cSet ->
        let (env, err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty2))
            ~error:(fun env -> (env, Error (ty2, tv)))
          @@ Typing_ops.sub_type_res expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ty1, err_res)
      (* Handle the case where Vector or Set was used as a typehint
       without type parameters *)
      | (_, Tclass ((_, n), _, []))
        when String.equal n SN.Collections.cVector
             || String.equal n SN.Collections.cSet ->
        (env, ty1, Ok ty2)
      | (r, Tclass (((_, n) as id), e, [tv]))
        when String.equal n SN.Collections.cVec ->
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, mk (r, Tclass (id, e, [tv'])), Ok ty2)
      | (r, Tclass (((_, n) as id), e, [tv]))
        when String.equal n SN.Collections.cKeyset ->
        let (env, err_res) = check_keyset_value env expr_pos ty1 ty2 in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, mk (r, Tclass (id, e, [tv'])), err_res)
      | (r, Tvarray tv) ->
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, mk (r, Tvarray tv'), Ok ty2)
      | (_, Tdynamic) -> (env, ty1, Ok ty2)
      | (_, Tobject) ->
        if Partial.should_check_error (Env.get_mode env) 4006 then
          let (env, ty) = error_assign_array_append env expr_pos ty1 in
          (env, ty, Ok ty2)
        else
          (env, ty1, Ok ty2)
      | (_, Tunapplied_alias _) ->
        Typing_defs.error_Tunapplied_alias_in_illegal_context ()
      | ( _,
          ( Tnonnull | Tdarray _ | Tvec_or_dict _ | Tvarray_or_darray _
          | Toption _ | Tprim _ | Tvar _ | Tfun _ | Tclass _ | Ttuple _
          | Tshape _ | Tunion _ | Tintersection _ | Tgeneric _ | Tnewtype _
          | Tdependent _ | Taccess _ | Tneg _ ) ) ->
        let (env, ty) = error_assign_array_append env expr_pos ty1 in
        (env, ty, Ok ty2))

let widen_for_assign_array_get ~expr_pos index_expr env ty =
  Typing_log.(
    log_with_level env "typing" ~level:1 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos expr_pos)
          env
          [Log_head ("widen_for_assign_array_get", [Log_type ("ty", ty)])]));
  match deref ty with
  (* dynamic is valid for assign array get *)
  | (_, Tdynamic) -> (env, Some ty)
  | (r, Tclass (((_, cn) as id), _, tyl))
    when cn = SN.Collections.cVec
         || cn = SN.Collections.cKeyset
         || cn = SN.Collections.cVector
         || cn = SN.Collections.cDict
         || cn = SN.Collections.cMap ->
    let (env, params) =
      List.map_env env tyl ~f:(fun env _ty ->
          Env.fresh_type_invariant env expr_pos)
    in
    let ty = mk (r, Tclass (id, Nonexact, params)) in
    (env, Some ty)
  | (r, Tvarray _) ->
    let (env, tv) = Env.fresh_type_invariant env expr_pos in
    (env, Some (mk (r, Tvarray tv)))
  | (r, Tvarray_or_darray _) ->
    let (env, tk) = Env.fresh_type_invariant env expr_pos in
    let (env, tv) = Env.fresh_type_invariant env expr_pos in
    (env, Some (mk (r, Tvarray_or_darray (tk, tv))))
  | (r, Tdarray _) ->
    let (env, tk) = Env.fresh_type_invariant env expr_pos in
    let (env, tv) = Env.fresh_type_invariant env expr_pos in
    (env, Some (mk (r, Tdarray (tk, tv))))
  | (r, Ttuple tyl) ->
    (* requires integer literal *)
    begin
      match index_expr with
      (* Should freshen type variables *)
      | (_, Int _) ->
        let (env, params) =
          List.map_env env tyl ~f:(fun env _ty ->
              Env.fresh_type_invariant env expr_pos)
        in
        (env, Some (mk (r, Ttuple params)))
      | _ -> (env, None)
    end
  | _ -> (env, None)

(* Used for typing an assignment e1[key] = e2
 * where e1 has type ty1, key has type tkey and e2 has type ty2.
 * Return the new array type
 *)

let assign_array_get_with_err ~array_pos ~expr_pos ur env ty1 key tkey ty2 =
  let (env, ety1) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"an array or collection"
      env
      (widen_for_assign_array_get ~expr_pos key)
      array_pos
      ty1
  in
  GenericRules.apply_rules_with_index_value_errs env ety1 (fun env ety1 ->
      let (r, ety1_) = deref ety1 in
      let arity_error (_, name) =
        Errors.array_get_arity expr_pos name (Reason.to_pos r)
      in
      let type_index env p ty_have ty_expect reason =
        Result.fold
          ~ok:(fun env -> (env, Ok ty_have))
          ~error:(fun env -> (env, Error (ty_have, ty_expect.et_type)))
        @@ Typing_coercion.coerce_type_res
             p
             reason
             env
             ty_have
             ty_expect
             Errors.index_type_mismatch
      in
      let error = (env, ety1, Ok tkey, Ok ty2) in
      match ety1_ with
      | Tvarray tv ->
        let tk = MakeType.enforced (MakeType.int (Reason.Ridx (fst key, r))) in
        let (env, idx_err) =
          type_index env expr_pos tkey tk Reason.index_array
        in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, mk (r, Tvarray tv'), idx_err, Ok ty2)
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cVector ->
        let tv =
          match argl with
          | [tv] -> tv
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let tk =
          MakeType.enforced (MakeType.int (Reason.Ridx_vector (fst key)))
        in
        let (env, idx_err) =
          type_index env expr_pos tkey tk (Reason.index_class cn)
        in
        let (env, err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty2))
            ~error:(fun env -> (env, Error (ty2, tv)))
          @@ Typing_ops.sub_type_res expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ety1, idx_err, err_res)
      | Tclass (((_, cn) as id), e, argl)
        when String.equal cn SN.Collections.cVec ->
        let tv =
          match argl with
          | [tv] -> tv
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let tk =
          MakeType.enforced (MakeType.int (Reason.Ridx_vector (fst key)))
        in
        let (env, idx_err) =
          type_index env expr_pos tkey tk (Reason.index_class cn)
        in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, mk (r, Tclass (id, e, [tv'])), idx_err, Ok ty2)
      | Tclass (((_, cn) as id), _, argl) when cn = SN.Collections.cMap ->
        let (env, idx_err1) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (tk, tv) =
          match argl with
          | [tk; tv] -> (tk, tv)
          | _ ->
            arity_error id;
            let any = err_witness env expr_pos in
            (any, any)
        in
        let tk =
          if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
            (* With sound dynamic and partial enforcement, we don't have to
               decide up front whether tk = arraykey and hence should be enforced.
               We can let inference figure that out for us. *)
            {
              et_type = tk;
              et_enforced =
                PartiallyEnforced
                  (ArraykeyStyle, (Pos_or_decl.of_raw_pos expr_pos, "arraykey"));
            }
          else
            let ak_t = MakeType.arraykey (Reason.Ridx_vector (fst key)) in
            if Typing_utils.is_sub_type_for_union env ak_t tk then
              (* hhvm will enforce that the key is an arraykey, so if
                 $x : Map<arraykey, t>, then it should be allowed to
                 set $x[$d] = e where $d : dynamic. NB above, we don't need
                 to check that tk <: arraykey, because the Map ensures that. *)
              MakeType.enforced tk
            else
              (* It is unsound to allow $x[$d] = e if $x : Map<string, t>
                 since the dynamic $d might be an int and hhvm wouldn't
                 complain.*)
              MakeType.unenforced tk
        in
        let (env, idx_err2) =
          type_index env expr_pos tkey tk (Reason.index_class cn)
        in
        let idx_err =
          match (idx_err1, idx_err2) with
          | (Error _, _) -> idx_err1
          | _ -> idx_err2
        in
        let (env, err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty2))
            ~error:(fun env -> (env, Error (ty2, tv)))
          @@ Typing_ops.sub_type_res expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ety1, idx_err, err_res)
      | Tclass (((_, cn) as id), e, argl)
        when String.equal cn SN.Collections.cDict ->
        let (env, idx_err) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
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
        (env, mk (r, Tclass (id, e, [tk'; tv'])), idx_err, Ok ty2)
      | Tclass ((_, cn), _, _) when String.equal cn SN.Collections.cKeyset ->
        Errors.keyset_set expr_pos (Reason.to_pos r);
        error
      | Tclass ((_, cn), _, _)
        when String.equal cn SN.Collections.cConstMap
             || String.equal cn SN.Collections.cImmMap
             || String.equal cn SN.Collections.cKeyedContainer
             || String.equal cn SN.Collections.cAnyArray
             || String.equal cn SN.Collections.cConstVector
             || String.equal cn SN.Collections.cImmVector
             || String.equal cn SN.Collections.cPair ->
        Errors.const_mutation
          expr_pos
          (Reason.to_pos r)
          (Typing_print.error env ety1);
        error
      | Tdarray (tk, tv) ->
        let (env, idx_err) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, mk (r, Tdarray (tk', tv')), idx_err, Ok ty2)
      | Tvarray_or_darray (tk, tv) ->
        let (env, idx_err) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, mk (r, Tvarray_or_darray (tk', tv')), idx_err, Ok ty2)
      | Tvec_or_dict (tk, tv) ->
        let (env, idx_err) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv') = Typing_union.union env tv ty2 in
        (env, mk (r, Tvec_or_dict (tk', tv')), idx_err, Ok ty2)
      | Terr -> error
      | Tdynamic -> (env, ety1, Ok tkey, Ok ty2)
      | Tany _ -> (env, ety1, Ok tkey, Ok ty2)
      | Tprim Tstring ->
        let tk = MakeType.enforced (MakeType.int (Reason.Ridx (fst key, r))) in
        let tv = MakeType.string (Reason.Rwitness expr_pos) in
        let (env, idx_err) =
          type_index env expr_pos tkey tk Reason.index_array
        in
        let (env, err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty2))
            ~error:(fun env -> (env, Error (ty2, tv)))
          @@ Typing_ops.sub_type_res expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ety1, idx_err, err_res)
      | Ttuple tyl ->
        let fail key_err reason =
          Errors.typing_error (fst key) (Reason.string_of_ureason reason);
          (env, ety1, key_err, Ok ty2)
        in
        begin
          match key with
          | (_, Int n) ->
            let idx = int_of_string_opt n in
            (match Option.map ~f:(List.split_n tyl) idx with
            | Some (tyl', _ :: tyl'') ->
              (env, MakeType.tuple r (tyl' @ (ty2 :: tyl'')), Ok tkey, Ok ty2)
            | _ -> fail (Ok tkey) Reason.index_tuple)
          | _ ->
            fail (Error (tkey, MakeType.int Reason.none)) Reason.URtuple_access
        end
      | Tshape (shape_kind, fdm) ->
        begin
          match TUtils.shape_field_name env key with
          | None -> error
          | Some field ->
            let field = TShapeField.of_ast Pos_or_decl.of_raw_pos field in
            let fdm' =
              TShapeMap.add field { sft_optional = false; sft_ty = ty2 } fdm
            in
            (env, mk (r, Tshape (shape_kind, fdm')), Ok tkey, Ok ty2)
        end
      | Tobject ->
        if Partial.should_check_error (Env.get_mode env) 4370 then (
          Errors.array_access_write
            expr_pos
            (Reason.to_pos r)
            (Typing_print.error env ety1);
          error
        ) else
          (env, ety1, Ok tkey, Ok ty2)
      | Tunapplied_alias _ ->
        Typing_defs.error_Tunapplied_alias_in_illegal_context ()
      | Toption _
      | Tnonnull
      | Tprim _
      | Tunion _
      | Tintersection _
      | Tgeneric _
      | Tnewtype _
      | Tdependent _
      | Tvar _
      | Tfun _
      | Tclass _
      | Taccess _
      | Tneg _ ->
        Errors.array_access_write
          expr_pos
          (Reason.to_pos r)
          (Typing_print.error env ety1);
        error)
