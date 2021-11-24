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
      | (_, _, Int _) ->
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
    let container_info = info_of_type container_ty
    and index_info = info_of_type index_ty in
    let ty_arraykey = MakeType.arraykey (Reason.Ridx_dict pos) in
    let ty_expected = { et_type = ty_arraykey; et_enforced = Enforced } in
    (* If we have an error in coercion here, we will add a `Hole` indicating the
       actual and expected type. The `Hole` may then be used in a codemod to
       add a call to `UNSAFE_CAST` so we need to consider what type we expect.
       There is a somewhat common pattern in older parts of www to do something like:

       ```
       function keyset_issue(?string $x): keyset<string> {
         $xs = keyset<string>[];
         ...
         /* HH_FIXME[4435] keyset values must be arraykeys */
         $xs[] = $x;
         return Keyset\filter_nulls($xs);
       }
       ```
       (even though it is impossible for keysets to contain nulls).

       If we were to add an expected type of 'arraykey' here it would be
       correct but adding an `UNSAFE_CAST<?string,arraykey>($x)` means we
       get cascading errors; here, we now have the wrong return type.

       To try and prevent this, if this is an optional type where the nonnull
       part can be coerced to arraykey, we prefer that type as our expected type.
    *)
    let (ok, ty_actual) =
      match deref index_ty with
      | (_, Toption inner_ty) ->
        ( (fun env ->
            (* We actually failed to subtype against arraykey so generate
               the error we would have seen *)
            error pos container_info index_info;
            (env, Error (index_ty, inner_ty))),
          inner_ty )
      | _ -> ((fun env -> (env, Ok index_ty)), index_ty)
    in
    Result.fold ~ok ~error:(fun env -> (env, Error (index_ty, ty_arraykey)))
    @@ Typing_coercion.coerce_type_res
         ~coerce_for_op:true
         pos
         reason
         env
         ty_actual
         ty_expected
         (fun ?code:_ ?quickfixes:_ _ _ -> error pos container_info index_info)
  else
    (env, Ok index_ty)

let check_arraykey_index_read =
  check_arraykey_index Errors.invalid_arraykey_read

let check_arraykey_index_write =
  check_arraykey_index Errors.invalid_arraykey_write

let check_keyset_value = check_arraykey_index Errors.invalid_keyset_value

let check_set_value = check_arraykey_index Errors.invalid_set_value

let pessimise_type env ty =
  Typing_union.union env ty (Typing_make_type.dynamic Reason.none)

let maybe_pessimise_type env ty =
  if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
    pessimise_type env ty
  else
    (env, ty)

let pessimised_tup_assign p env arg_ty =
  let (env, ty) = Env.fresh_type env p in
  let (env, pess_ty) = pessimise_type env ty in
  (* There can't be an error since the type variable is fresh *)
  let env =
    SubType.sub_type env arg_ty pess_ty (fun ?code:_ ?quickfixes:_ _ ->
        Errors.internal_error p "Subtype of fresh type variable")
  in
  (env, ty)

(* Assignment into a pessimised vec or dict should behave as though it has type
   forall T1 T2. vec<T1> -> idx -> ~T2 -> vec<T1|T2> *)
let pessimised_vec_dict_assign p env vec_ty arg_ty =
  let (env, ty) = pessimised_tup_assign p env arg_ty in
  Typing_union.union env vec_ty ty

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
  GenericRules.apply_rules_with_index_value_errs
    ~ignore_type_structure:true
    env
    ty1
    (fun env ty1 ->
      let (r, ety1_) = deref ty1 in
      let dflt_arr_res = Ok ty1 in
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
      let nullable_container_get env ty_actual ty =
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
        else
          let mixed = MakeType.mixed Reason.none in
          (* If our non-null type, ty, is a subtype of `KeyedContainer`
             use it in the hole, otherwise suggest KeyedContainer *)
          let ty_expected =
            if
              SubType.is_sub_type
                env
                ty
                (MakeType.keyed_container Reason.none mixed mixed)
            then
              ty
            else
              let nothing = MakeType.nothing Reason.none in
              MakeType.keyed_container Reason.none nothing nothing
          in
          Errors.null_container
            expr_pos
            (Reason.to_string
               "This is what makes me believe it can be `null`."
               r);
          (env, err_witness env expr_pos, Some (ty_actual, ty_expected), None)
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
             ~coerce_for_op:true
             p
             reason
             env
             ty_have
             ty_expect
             Errors.index_type_mismatch
      in
      match ety1_ with
      | Tvarray ty ->
        let (_, p2, _) = e2 in
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx (p2, r))) in
        let (env, idx_err_res) =
          type_index env expr_pos ty2 ty1 Reason.index_array
        in
        (env, ty, dflt_arr_res, idx_err_res)
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cVector
             || String.equal cn SN.Collections.cVec ->
        let (env, ty) =
          match argl with
          | [ty] -> maybe_pessimise_type env ty
          | _ ->
            arity_error id;
            (env, err_witness env expr_pos)
        in
        let (_, p2, _) = e2 in
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx_vector p2)) in
        let (env, idx_err_res) =
          type_index env expr_pos ty2 ty1 (Reason.index_class cn)
        in
        (env, ty, dflt_arr_res, idx_err_res)
      | Tclass (((_, cn) as id), _, argl)
        when cn = SN.Collections.cMap
             || cn = SN.Collections.cDict
             || cn = SN.Collections.cKeyset ->
        if cn = SN.Collections.cKeyset && is_lvalue then (
          Errors.keyset_set expr_pos (Reason.to_pos r);
          (env, err_witness env expr_pos, Ok ty2, dflt_arr_res)
        ) else
          let (k, (env, v)) =
            match argl with
            | [t] when String.equal cn SN.Collections.cKeyset ->
              (t, maybe_pessimise_type env t)
            | [k; v] when String.( <> ) cn SN.Collections.cKeyset ->
              (k, maybe_pessimise_type env v)
            | _ ->
              arity_error id;
              let any = err_witness env expr_pos in
              (any, (env, any))
          in
          (* dict and keyset are covariant in the key type, so subsumption
           * lets you upcast the key type beyond ty2 to arraykey.
           * e.g. consider $d: dict<string,int> and $i:int
           * and $d[$i] should actually type check because
           * dict<string,int> <: dict<arraykey,int>
           *)
          let (env, idx_err_res) =
            if String.equal cn SN.Collections.cMap then
              let (env, k) = Env.expand_type env k in
              let (env, k) = maybe_pessimise_type env k in
              type_index
                env
                expr_pos
                ty2
                (MakeType.enforced k)
                (Reason.index_class cn)
            else
              check_arraykey_index_read env expr_pos ty1 ty2
          in
          (env, v, dflt_arr_res, idx_err_res)
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
          let ty_nothing = Typing_make_type.nothing Reason.none in
          (env, ty1, Ok ty2, Error (ty1, ty_nothing))
        else
          let (_k, (env, v)) =
            match argl with
            | [k; v] -> (k, maybe_pessimise_type env v)
            | _ ->
              arity_error id;
              let any = err_witness env expr_pos in
              (any, (env, any))
          in
          let (env, idx_err_res) =
            check_arraykey_index_read env expr_pos ty1 ty2
          in
          (env, v, dflt_arr_res, idx_err_res)
      | Tclass (((_, cn) as id), _, argl)
        when (not is_lvalue)
             && (String.equal cn SN.Collections.cConstVector
                || String.equal cn SN.Collections.cImmVector) ->
        let (env, ty) =
          match argl with
          | [ty] -> maybe_pessimise_type env ty
          | _ ->
            arity_error id;
            (env, err_witness env expr_pos)
        in
        let (_, p2, _) = e2 in
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx (p2, r))) in
        let (env, idx_err_res) =
          type_index env expr_pos ty2 ty1 (Reason.index_class cn)
        in
        (env, ty, dflt_arr_res, idx_err_res)
      | Tclass ((_, cn), _, tys)
        when is_lvalue
             && (String.equal cn SN.Collections.cConstVector
                || String.equal cn SN.Collections.cImmVector) ->
        let (env, ty1) = error_const_mutation env expr_pos ty1 in
        let ty_vector =
          Typing_make_type.class_type Reason.none SN.Collections.cVector tys
        in
        (env, ty1, Error (ty1, ty_vector), Ok ty2)
      | Tdarray (_k, v)
      | Tvec_or_dict (_k, v)
      | Tvarray_or_darray (_k, v) ->
        let (env, idx_err_res) =
          check_arraykey_index_read env expr_pos ty1 ty2
        in
        let (env, tv) = maybe_pessimise_type env v in
        (env, tv, dflt_arr_res, idx_err_res)
      | Terr -> (env, err_witness env expr_pos, dflt_arr_res, Ok ty2)
      | Tdynamic
        when Typing_env_types.(
               TypecheckerOptions.enable_sound_dynamic env.genv.tcopt) ->
        let tv = Typing_make_type.dynamic r in
        let (env, idx_err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty2))
            ~error:(fun env -> (env, Error (ty2, tv)))
          @@ Typing_utils.sub_type_res
               env
               ~coerce:(Some Typing_logic.CoerceToDynamic)
               ty2
               tv
               (Errors.unify_error_at expr_pos)
        in
        (env, ty1, idx_err_res, dflt_arr_res)
      | Tdynamic -> (env, ty1, Ok ty2, dflt_arr_res)
      | Tany _ -> (env, TUtils.mk_tany env expr_pos, dflt_arr_res, Ok ty2)
      | Tprim Tstring ->
        let ty = MakeType.string (Reason.Rwitness expr_pos) in
        let (_, p2, _) = e2 in
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx (p2, r))) in
        let (env, idx_err_res) =
          type_index env expr_pos ty2 ty1 Reason.index_array
        in
        (env, ty, dflt_arr_res, idx_err_res)
      | Ttuple tyl ->
        (* requires integer literal *)
        (match e2 with
        | (_, p, Int n) ->
          let idx = int_of_string_opt n in
          (match Option.bind idx ~f:(List.nth tyl) with
          | Some nth ->
            let (env, pess_ty) = maybe_pessimise_type env nth in
            (env, pess_ty, dflt_arr_res, Ok ty2)
          | None ->
            Errors.typing_error p (Reason.string_of_ureason Reason.index_tuple);
            (env, err_witness env p, dflt_arr_res, Ok ty2))
        | (_, p, _) ->
          Errors.typing_error p (Reason.string_of_ureason Reason.URtuple_access);
          ( env,
            err_witness env p,
            dflt_arr_res,
            Error (ty2, MakeType.int Reason.none) ))
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cPair ->
        let (ty_fst, ty_snd) =
          match argl with
          | [ty_fst; ty_snd] -> (ty_fst, ty_snd)
          | _ ->
            arity_error id;
            let any = err_witness env expr_pos in
            (any, any)
        in
        (* requires integer literal *)
        (match e2 with
        | (_, p, Int n) ->
          let idx = int_of_string_opt n in
          (match Option.bind ~f:(List.nth [ty_fst; ty_snd]) idx with
          | Some nth -> (env, nth, dflt_arr_res, Ok ty2)
          | None ->
            Errors.typing_error p
            @@ Reason.string_of_ureason (Reason.index_class cn);
            (env, err_witness env p, dflt_arr_res, Ok ty2))
        | (_, p, _) ->
          Errors.typing_error p (Reason.string_of_ureason Reason.URpair_access);
          ( env,
            err_witness env p,
            dflt_arr_res,
            Error (ty2, MakeType.int Reason.none) ))
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
          (env, ty, dflt_arr_res, Ok ty2)
        else
          let (_, p, _) = e2 in
          begin
            match TUtils.shape_field_name env e2 with
            | None ->
              (* there was already an error in shape_field name,
                 don't report another one for a missing field *)
              (env, err_witness env p, dflt_arr_res, Ok ty2)
            | Some field ->
              let field = TShapeField.of_ast Pos_or_decl.of_raw_pos field in
              begin
                match TShapeMap.find_opt field fdm with
                | None ->
                  Errors.undefined_field
                    ~use_pos:p
                    ~name:(TUtils.get_printable_shape_field_name field)
                    ~shape_type_pos:(Reason.to_pos r);
                  (env, err_witness env p, dflt_arr_res, Ok ty2)
                | Some { sft_optional; sft_ty } ->
                  if sft_optional then (
                    let declared_field =
                      List.find_exn
                        ~f:(fun x -> TShapeField.equal field x)
                        (TShapeMap.keys fdm)
                    in
                    Errors.array_get_with_optional_field
                      p
                      (Typing_defs.TShapeField.pos declared_field)
                      (TUtils.get_printable_shape_field_name field);
                    (env, err_witness env p, dflt_arr_res, Ok ty2)
                  ) else
                    let (env, pess_sft_ty) = maybe_pessimise_type env sft_ty in
                    (env, pess_sft_ty, dflt_arr_res, Ok ty2)
              end
          end
      | Toption ty ->
        let (env, ty, err_opt_arr, err_opt_idx) =
          nullable_container_get env ty1 ty
        in
        let err_res_arr =
          Option.value_map
            err_opt_arr
            ~default:dflt_arr_res
            ~f:(fun (ty_have, ty_expect) -> Error (ty_have, ty_expect))
        in
        let err_res_idx =
          Option.value_map
            err_opt_idx
            ~default:(Ok ty2)
            ~f:(fun (ty_have, ty_expect) -> Error (ty_have, ty_expect))
        in
        (env, ty, err_res_arr, err_res_idx)
      | Tprim Tnull ->
        let ty = MakeType.nothing Reason.Rnone in
        let (env, ty, err_opt_arr, err_opt_idx) =
          nullable_container_get env ty1 ty
        in
        let err_res_arr =
          Option.value_map
            err_opt_arr
            ~default:dflt_arr_res
            ~f:(fun (ty_have, ty_expect) -> Error (ty_have, ty_expect))
        in
        let err_res_idx =
          Option.value_map
            err_opt_idx
            ~default:(Ok ty2)
            ~f:(fun (ty_have, ty_expect) -> Error (ty_have, ty_expect))
        in
        (env, ty, err_res_arr, err_res_idx)
      | Tnewtype (ts, [ty], bound) ->
        begin
          match deref bound with
          | (r, Tshape (shape_kind, fields))
            when String.equal ts SN.FB.cTypeStructure ->
            let (env, fields) =
              Typing_structure.transform_shapemap env array_pos ty fields
            in
            let ty = mk (r, Tshape (shape_kind, fields)) in
            let (env, ty, err_opt_arr, err_opt_idx) =
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
            let err_res_idx =
              Option.value_map
                err_opt_idx
                ~default:(Ok ty2)
                ~f:(fun (ty_have, ty_expect) -> Error (ty_have, ty_expect))
            in
            let err_res_arr =
              Option.value_map
                err_opt_arr
                ~default:dflt_arr_res
                ~f:(fun (ty_have, ty_expect) -> Error (ty_have, ty_expect))
            in
            (env, ty, err_res_arr, err_res_idx)
          | _ ->
            let (env, ty1) = error_array env expr_pos ty1 in
            let ty_nothing = Typing_make_type.nothing Reason.none in
            let ty_keyedcontainer =
              Typing_make_type.(keyed_container Reason.none ty2 ty_nothing)
            in
            (env, ty1, Error (ty1, ty_keyedcontainer), Ok ty2)
        end
      | Tunapplied_alias _ ->
        Typing_defs.error_Tunapplied_alias_in_illegal_context ()
      | Tnonnull
      | Tsupportdynamic
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
        let ty_nothing = Typing_make_type.nothing Reason.none in
        let ty_keyedcontainer =
          Typing_make_type.(keyed_container Reason.none ty2 ty_nothing)
        in
        (env, ty1, Error (ty1, ty_keyedcontainer), Ok ty2)
      (* Type-check array access as though it is the method
       * array_get<Tk,Tv>(KeyedContainer<Tk,Tv> $array, Tk $key): Tv
       * (We can already force Tk to be the type of the key argument because
       * Tk does not appear in the result of the call)
       *)
      | Tvar _ ->
        let (env, value) = Env.fresh_type env expr_pos in
        let keyed_container = MakeType.keyed_container r ty2 value in
        let (env, arr_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty1))
            ~error:(fun env -> (env, Error (ty1, keyed_container)))
          @@ SubType.sub_type_res
               env
               ty1
               keyed_container
               (Errors.index_type_mismatch_at expr_pos)
        in
        (env, value, arr_res, Ok ty2))

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
  GenericRules.apply_rules_with_index_value_errs env ty1 (fun env ty1 ->
      match deref ty1 with
      | (_, Tany _) -> (env, ty1, Ok ty1, Ok ty2)
      | (_, Terr) -> (env, ty1, Ok ty1, Ok ty2)
      | (_, Tclass ((_, n), _, [tv])) when String.equal n SN.Collections.cVector
        ->
        let (env, tv) = maybe_pessimise_type env tv in
        let (env, val_err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty2))
            ~error:(fun env -> (env, Error (ty2, tv)))
          @@ Typing_ops.sub_type_res expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ty1, Ok ty1, val_err_res)
      (* Handle the case where Vector or Set was used as a typehint
         without type parameters *)
      | (_, Tclass ((_, n), _, [])) when String.equal n SN.Collections.cVector
        ->
        (env, ty1, Ok ty1, Ok ty2)
      | (r, Tclass (((_, n) as id), e, []))
        when String.equal n SN.Collections.cKeyset
             || String.equal n SN.Collections.cSet ->
        let (env, val_err_res) =
          if String.equal n SN.Collections.cKeyset then
            check_keyset_value env expr_pos ty1 ty2
          else
            check_set_value env expr_pos ty1 ty2
        in
        let ty = mk (r, Tclass (id, e, [ty2])) in
        (env, ty, Ok ty, val_err_res)
      | (r, Tclass (((_, n) as id), e, [tv]))
        when String.equal n SN.Collections.cVec ->
        let (env, tv') =
          if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
            pessimised_vec_dict_assign expr_pos env tv ty2
          else
            Typing_union.union env tv ty2
        in
        let ty = mk (r, Tclass (id, e, [tv'])) in
        (env, ty, Ok ty, Ok ty2)
      | (r, Tclass (((_, n) as id), e, [tv]))
        when String.equal n SN.Collections.cKeyset ->
        let (env, err_res) = check_keyset_value env expr_pos ty1 ty2 in
        let (env, tk') =
          let dyn_t = MakeType.dynamic Reason.Rnone in
          if
            (* TODO: Remove the test for sound dynamic. It is never ok to put
               dynamic as the key to a dict since the key must be a
               subtype of arraykey. *)
            Typing_env_types.(
              TypecheckerOptions.enable_sound_dynamic env.genv.tcopt)
            &&
            match err_res with
            | Ok _ -> Typing_utils.is_sub_type_for_union env dyn_t ty2
            | _ -> false
          then
            (* if there weren't any errors with the key then either it is dynamic
               or a subtype of arraykey. If it's also a supertype of dynamic, then
               set the keytype to arraykey, since that the only thing that hhvm won't
               error on.
            *)
            (env, MakeType.arraykey (Reason.Rkey_value_collection_key expr_pos))
          else
            Typing_union.union env tv ty2
        in
        let ty = mk (r, Tclass (id, e, [tk'])) in
        (env, ty, Ok ty, err_res)
      | (_, Tclass ((_, n), _, [tv])) when String.equal n SN.Collections.cSet ->
        let (env, err_res) =
          match check_set_value env expr_pos ty1 ty2 with
          | (_, Error _) as err_res -> err_res
          | (env, _) ->
            let tv' =
              let ak_t = MakeType.arraykey (Reason.Ridx_vector expr_pos) in
              if Typing_utils.is_sub_type_for_union env ak_t tv then
                (* hhvm will enforce that the key is an arraykey, so if
                   $x : Set<arraykey>, then it should be allowed to
                   set $x[] = e where $d : dynamic. *)
                MakeType.enforced tv
              else
                (* It is unsound to allow $x[] = e if $x : Set<string>
                   since the dynamic $d might be an int and hhvm wouldn't
                   complain.*)
                MakeType.unenforced tv
            in
            Result.fold
              ~ok:(fun env -> (env, Ok ty2))
              ~error:(fun env -> (env, Error (ty2, tv)))
            @@ Typing_coercion.coerce_type_res
                 ~coerce_for_op:true
                 expr_pos
                 ur
                 env
                 ty2
                 tv'
                 Errors.unify_error
        in
        (env, ty1, Ok ty1, err_res)
      | (r, Tvarray tv) ->
        let (env, tv') = Typing_union.union env tv ty2 in
        let ty = mk (r, Tvarray tv') in
        (env, ty, Ok ty, Ok ty2)
      | (r, Tdynamic)
        when Typing_env_types.(
               TypecheckerOptions.enable_sound_dynamic env.genv.tcopt) ->
        let tv = Typing_make_type.dynamic r in
        let (env, val_err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty2))
            ~error:(fun env -> (env, Error (ty2, tv)))
          @@ Typing_utils.sub_type_res
               env
               ~coerce:(Some Typing_logic.CoerceToDynamic)
               ty2
               tv
               (Errors.unify_error_at expr_pos)
        in
        (env, ty1, Ok ty1, val_err_res)
      | (_, Tdynamic) -> (env, ty1, Ok ty1, Ok ty2)
      | (_, Tunapplied_alias _) ->
        Typing_defs.error_Tunapplied_alias_in_illegal_context ()
      | ( _,
          ( Tnonnull | Tsupportdynamic | Tdarray _ | Tvec_or_dict _
          | Tvarray_or_darray _ | Toption _ | Tprim _ | Tvar _ | Tfun _
          | Tclass _ | Ttuple _ | Tshape _ | Tunion _ | Tintersection _
          | Tgeneric _ | Tnewtype _ | Tdependent _ | Taccess _ | Tneg _ ) ) ->
        let (env, ty) = error_assign_array_append env expr_pos ty1 in
        let ty_nothing = Typing_make_type.nothing Reason.none in
        (env, ty, Error (ty, ty_nothing), Error (ty2, ty_nothing)))

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
      | (_, _, Int _) ->
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

let assign_array_get_with_err
    ~array_pos ~expr_pos ur env ty1 (key : Nast.expr) tkey ty2 =
  let (env, ety1) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"an array or collection"
      env
      (widen_for_assign_array_get ~expr_pos key)
      array_pos
      ty1
  in
  GenericRules.apply_rules_with_array_index_value_errs env ety1 (fun env ety1 ->
      let (r, ety1_) = deref ety1 in
      let arity_error (_, name) =
        Errors.array_get_arity expr_pos name (Reason.to_pos r)
      in
      let type_index env p ty_have ty_expect reason =
        Result.fold
          ~ok:(fun env -> (env, Ok ty_have))
          ~error:(fun env -> (env, Error (ty_have, ty_expect.et_type)))
        @@ Typing_coercion.coerce_type_res
             ~coerce_for_op:true
             p
             reason
             env
             ty_have
             ty_expect
             Errors.index_type_mismatch
      in
      match ety1_ with
      | Tvarray tv ->
        let (_, p, _) = key in
        let tk = MakeType.enforced (MakeType.int (Reason.Ridx (p, r))) in
        let (env, idx_err) =
          type_index env expr_pos tkey tk Reason.index_array
        in
        let (env, tv') = Typing_union.union env tv ty2 in
        let ty = mk (r, Tvarray tv') in
        (env, ty, Ok ty, idx_err, Ok ty2)
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cVector ->
        let (env, tv) =
          match argl with
          | [tv] -> maybe_pessimise_type env tv
          | _ ->
            arity_error id;
            (env, err_witness env expr_pos)
        in
        let (_, p, _) = key in
        let tk = MakeType.enforced (MakeType.int (Reason.Ridx_vector p)) in
        let (env, idx_err) =
          type_index env expr_pos tkey tk (Reason.index_class cn)
        in
        let (env, err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty2))
            ~error:(fun env -> (env, Error (ty2, tv)))
          @@ Typing_ops.sub_type_res expr_pos ur env ty2 tv Errors.unify_error
        in
        (env, ety1, Ok ety1, idx_err, err_res)
      | Tclass (((_, cn) as id), e, argl)
        when String.equal cn SN.Collections.cVec ->
        let (env, tv) =
          match argl with
          | [tv] -> (env, tv)
          | _ ->
            arity_error id;
            (env, err_witness env expr_pos)
        in
        let (_, p, _) = key in
        let tk = MakeType.enforced (MakeType.int (Reason.Ridx_vector p)) in
        let (env, idx_err) =
          type_index env expr_pos tkey tk (Reason.index_class cn)
        in
        let (env, tv') =
          if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
            pessimised_vec_dict_assign expr_pos env tv ty2
          else
            Typing_union.union env tv ty2
        in
        let ty = mk (r, Tclass (id, e, [tv'])) in
        (env, ty, Ok ty, idx_err, Ok ty2)
      | Tclass (((_, cn) as id), _, argl) when cn = SN.Collections.cMap ->
        let (env, idx_err1) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (tk, (env, tv)) =
          match argl with
          | [tk; tv] -> (tk, maybe_pessimise_type env tv)
          | _ ->
            arity_error id;
            let any = err_witness env expr_pos in
            (any, (env, any))
        in
        let (env, tk) =
          let (_, p, _) = key in
          let ak_t = MakeType.arraykey (Reason.Ridx_vector p) in
          let (env, tk) = maybe_pessimise_type env tk in
          if Typing_utils.is_sub_type_for_union env ak_t tk then
            (* hhvm will enforce that the key is an arraykey, so if
               $x : Map<arraykey, t>, then it should be allowed to
               set $x[$d] = e where $d : dynamic. NB above, we don't need
               to check that tk <: arraykey, because the Map ensures that. *)
            (env, MakeType.enforced tk)
          else
            (* It is unsound to allow $x[$d] = e if $x : Map<string, t>
               since the dynamic $d might be an int and hhvm wouldn't
               complain.*)
            (env, MakeType.unenforced tk)
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
        (env, ety1, Ok ety1, idx_err, err_res)
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
        let (env, tk') =
          let dyn_t = MakeType.dynamic Reason.Rnone in
          if
            (* TODO: Remove the test for sound dynamic. It is never ok to put
               dynamic as the key to a dict since the key must be a
               subtype of arraykey. *)
            Typing_env_types.(
              TypecheckerOptions.enable_sound_dynamic env.genv.tcopt)
            &&
            match idx_err with
            | Ok _ -> Typing_utils.is_sub_type_for_union env dyn_t tkey
            | _ -> false
          then
            (* if there weren't any errors with the key then either it is dynamic
               or a subtype of arraykey. If it's also a supertype of dynamic, then
               set the keytype to arraykey, since that the only thing that hhvm won't
               error on.
            *)
            if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
              pessimised_vec_dict_assign expr_pos env tk tkey
            else
              let (_, p, _) = key in
              (env, MakeType.arraykey (Reason.Ridx_dict p))
          else
            Typing_union.union env tk tkey
        in
        let (env, tv') =
          if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
            pessimised_vec_dict_assign expr_pos env tv ty2
          else
            Typing_union.union env tv ty2
        in
        let ty = mk (r, Tclass (id, e, [tk'; tv'])) in
        (env, ty, Ok ty, idx_err, Ok ty2)
      | Tclass ((_, cn), _, _) when String.equal cn SN.Collections.cKeyset ->
        Errors.keyset_set expr_pos (Reason.to_pos r);
        ( env,
          ety1,
          Error (ety1, Typing_make_type.nothing Reason.none),
          Ok tkey,
          Ok ty2 )
      | Tclass ((_, cn), _, tys)
        when String.equal cn SN.Collections.cConstMap
             || String.equal cn SN.Collections.cImmMap ->
        Errors.const_mutation
          expr_pos
          (Reason.to_pos r)
          (Typing_print.error env ety1);
        let ty_expect =
          MakeType.class_type Reason.none SN.Collections.cMap tys
        in
        (env, ety1, Error (ety1, ty_expect), Ok tkey, Ok ty2)
      | Tclass ((_, cn), _, tys)
        when String.equal cn SN.Collections.cConstVector
             || String.equal cn SN.Collections.cImmVector ->
        Errors.const_mutation
          expr_pos
          (Reason.to_pos r)
          (Typing_print.error env ety1);
        let ty_expect =
          MakeType.class_type Reason.none SN.Collections.cVector tys
        in
        (env, ety1, Error (ety1, ty_expect), Ok tkey, Ok ty2)
      | Tclass ((_, cn), _, _)
        when String.equal cn SN.Collections.cKeyedContainer
             || String.equal cn SN.Collections.cAnyArray
             || String.equal cn SN.Collections.cPair ->
        Errors.const_mutation
          expr_pos
          (Reason.to_pos r)
          (Typing_print.error env ety1);
        let ty_expect = MakeType.nothing Reason.none in
        (env, ety1, Error (ety1, ty_expect), Ok tkey, Ok ty2)
      | Tdarray (tk, tv) ->
        let (env, idx_err) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv') = Typing_union.union env tv ty2 in
        let ty = mk (r, Tdarray (tk', tv')) in
        (env, ty, Ok ty, idx_err, Ok ty2)
      | Tvarray_or_darray (tk, tv) ->
        let (env, idx_err) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv') = Typing_union.union env tv ty2 in
        let ty = mk (r, Tvarray_or_darray (tk', tv')) in
        (env, ty, Ok ty, idx_err, Ok ty2)
      | Tvec_or_dict (tk, tv) ->
        let (env, tv) = maybe_pessimise_type env tv in
        let (env, idx_err) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv') = Typing_union.union env tv ty2 in
        let ty = mk (r, Tvec_or_dict (tk', tv')) in
        (env, ty, Ok ty, idx_err, Ok ty2)
      | Terr -> (env, ety1, Ok ety1, Ok tkey, Ok ty2)
      | Tdynamic
        when Typing_env_types.(
               TypecheckerOptions.enable_sound_dynamic env.genv.tcopt) ->
        let tv = Typing_make_type.dynamic r in
        let (env, idx_err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok tkey))
            ~error:(fun env -> (env, Error (tkey, tv)))
          @@ Typing_utils.sub_type_res
               ~coerce:(Some Typing_logic.CoerceToDynamic)
               env
               tkey
               tv
               (Errors.unify_error_at expr_pos)
        in
        let (env, val_err_res) =
          Result.fold
            ~ok:(fun env -> (env, Ok ty2))
            ~error:(fun env -> (env, Error (ty2, tv)))
          @@ Typing_utils.sub_type_res
               ~coerce:(Some Typing_logic.CoerceToDynamic)
               env
               ty2
               tv
               (Errors.unify_error_at expr_pos)
        in
        (env, ety1, Ok ety1, idx_err_res, val_err_res)
      | Tdynamic -> (env, ety1, Ok ety1, Ok tkey, Ok ty2)
      | Tany _ -> (env, ety1, Ok ety1, Ok tkey, Ok ty2)
      | Tprim Tstring ->
        let (_, p, _) = key in
        let tk = MakeType.enforced (MakeType.int (Reason.Ridx (p, r))) in
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
        (env, ety1, Ok ety1, idx_err, err_res)
      | Ttuple tyl ->
        let fail key_err reason =
          let (_, p, _) = key in
          Errors.typing_error p (Reason.string_of_ureason reason);
          (env, ety1, Ok ety1, key_err, Ok ty2)
        in
        begin
          match key with
          | (_, _, Int n) ->
            let idx = int_of_string_opt n in
            (match Option.map ~f:(List.split_n tyl) idx with
            | Some (tyl', _ :: tyl'') ->
              let (env, pess_ty2) =
                if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env)
                then
                  pessimised_tup_assign expr_pos env ty2
                else
                  (env, ty2)
              in
              ( env,
                MakeType.tuple r (tyl' @ pess_ty2 :: tyl''),
                Ok ety1,
                Ok tkey,
                Ok ty2 )
            | _ -> fail (Ok tkey) Reason.index_tuple)
          | _ ->
            fail (Error (tkey, MakeType.int Reason.none)) Reason.URtuple_access
        end
      | Tshape (shape_kind, fdm) ->
        begin
          match TUtils.shape_field_name env key with
          | None -> (env, ety1, Ok ety1, Ok tkey, Ok ty2)
          | Some field ->
            let field = TShapeField.of_ast Pos_or_decl.of_raw_pos field in
            let (env, pess_ty2) =
              if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
                pessimised_tup_assign expr_pos env ty2
              else
                (env, ty2)
            in
            let fdm' =
              TShapeMap.add
                field
                { sft_optional = false; sft_ty = pess_ty2 }
                fdm
            in
            let ty = mk (r, Tshape (shape_kind, fdm')) in
            (env, ty, Ok ty, Ok tkey, Ok ty2)
        end
      | Tunapplied_alias _ ->
        Typing_defs.error_Tunapplied_alias_in_illegal_context ()
      | Toption _
      | Tnonnull
      | Tsupportdynamic
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
        let ty_nothing = Typing_make_type.nothing Reason.none in
        (env, ety1, Error (ety1, ty_nothing), Ok tkey, Error (ty2, ty_nothing)))
