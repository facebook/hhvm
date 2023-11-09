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

let mk_ty_mismatch_res ty_have ty_expect =
  Option.value_map
    ~default:(Ok ty_have)
    ~f:Fn.(const @@ Error (ty_have, ty_expect))

let err_witness env p = Env.fresh_type_error env p

let error_array env p ty =
  Typing_error_utils.add_typing_error
    ~env
    Typing_error.(
      with_code ~code:Error_code.IndexTypeMismatch
      @@ primary
      @@ Primary.Array_access
           {
             ctxt = `read;
             pos = p;
             decl_pos = get_pos ty;
             ty_name = lazy (Typing_print.error env ty);
           })

let error_const_mutation env p ty =
  Typing_error_utils.add_typing_error
    ~env
    Typing_error.(
      primary
      @@ Primary.Const_mutation
           {
             pos = p;
             decl_pos = get_pos ty;
             ty_name = lazy (Typing_print.error env ty);
           });
  err_witness env p

let error_assign_array_append env p ty =
  if not (TUtils.is_tyvar_error env ty) then
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Array_append
             {
               pos = p;
               decl_pos = get_pos ty;
               ty_name = lazy (Typing_print.error env ty);
             });
  (env, ty)

let maybe_make_supportdyn r env ~supportdyn ty =
  if supportdyn then
    Typing_utils.make_supportdyn r env ty
  else
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
  | (_, Tprim Tnull) when lhs_of_null_coalesce -> ((env, None), Some ty)
  (* dynamic is valid for array get *)
  | (_, Tdynamic) -> ((env, None), Some ty)
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
    ((env, None), Some ty)
  (* The same is true of PHP arrays *)
  | (r, Tvec_or_dict _) ->
    let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
    let (env, index_ty) = Env.fresh_type_invariant env expr_pos in
    let ty = MakeType.keyed_container r index_ty element_ty in
    ((env, None), Some ty)
  (* For tuples, we just freshen the element types *)
  | (r, Ttuple tyl) -> begin
    (* requires integer literal *)
    match index_expr with
    (* Should freshen type variables *)
    | (_, _, Aast.Int _) ->
      let (env, params) =
        List.map_env env tyl ~f:(fun env _ty ->
            Env.fresh_type_invariant env expr_pos)
      in
      ((env, None), Some (MakeType.tuple r params))
    | _ -> ((env, None), None)
  end
  (* Whatever the lower bound, construct an open, singleton shape type. *)
  | (r, Tshape { s_fields = fdm; _ }) -> begin
    let (fld_opt, ty_err_opt) =
      TUtils.shape_field_name_with_ty_err env index_expr
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    match fld_opt with
    | None -> ((env, None), None)
    | Some field ->
      let field = TShapeField.of_ast Pos_or_decl.of_raw_pos field in
      (match TShapeMap.find_opt field fdm with
      (* If field is in the lower bound but is optional, then no upper bound makes sense
       * unless this is a null-coalesce access *)
      | Some { sft_optional = true; _ } when not lhs_of_null_coalesce ->
        ((env, None), None)
      | _ ->
        let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
        let (env, rest_ty) = Env.fresh_type_invariant env expr_pos in
        let upper_fdm =
          TShapeMap.add
            field
            { sft_optional = lhs_of_null_coalesce; sft_ty = element_ty }
            TShapeMap.empty
        in
        let upper_shape_ty = MakeType.shape r rest_ty upper_fdm in
        ((env, None), Some upper_shape_ty))
  end
  | _ -> ((env, None), None)

(* Check that an index to a map-like collection passes the basic test of
 * being a subtype of arraykey
 *)
let check_arraykey_index error env pos container_ty index_ty =
  if TypecheckerOptions.disallow_invalid_arraykey (Env.get_tcopt env) then (
    let (env, container_ty) = Env.expand_type env container_ty in
    let reason =
      match get_node container_ty with
      | Tclass ((_, cn), _, _) -> Reason.index_class cn
      | _ -> Reason.index_array
    in
    let info_of_type ty = (get_pos ty, lazy (Typing_print.error env ty)) in
    let container_info = info_of_type container_ty in
    let index_info = info_of_type index_ty in
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
    let base_error = error pos container_info index_info in
    let (ty_actual, is_option) =
      match deref index_ty with
      | (_, Toption inner_ty) -> (inner_ty, true)
      | _ -> (index_ty, false)
    in
    let (env, e1) =
      Typing_coercion.coerce_type
        ~coerce_for_op:true
        pos
        reason
        env
        ty_actual
        ty_expected
      @@ Typing_error.Callback.always base_error
    in
    let (ty_mismatch, e2) =
      match e1 with
      | None when is_option ->
        (Error (index_ty, ty_actual), Some (Typing_error.primary base_error))
      | None -> (Ok index_ty, None)
      | Some _ -> (Error (index_ty, ty_arraykey), None)
    in
    Option.(
      iter ~f:(Typing_error_utils.add_typing_error ~env)
      @@ merge e1 e2 ~f:Typing_error.both);
    (env, ty_mismatch)
  ) else
    (env, Ok index_ty)

let check_arraykey_index_read =
  let mk_err pos (container_pos, container_ty_name) (key_pos, key_ty_name) =
    Typing_error.Primary.Invalid_arraykey
      {
        pos;
        ctxt = `read;
        container_pos;
        container_ty_name;
        key_pos;
        key_ty_name;
      }
  in
  check_arraykey_index mk_err

let check_arraykey_index_write =
  let mk_err pos (container_pos, container_ty_name) (key_pos, key_ty_name) =
    Typing_error.Primary.Invalid_arraykey
      {
        pos;
        ctxt = `write;
        container_pos;
        container_ty_name;
        key_pos;
        key_ty_name;
      }
  in
  check_arraykey_index mk_err

let check_keyset_value =
  let mk_err pos (container_pos, container_ty_name) (value_pos, value_ty_name) =
    Typing_error.Primary.Invalid_keyset_value
      { pos; container_pos; container_ty_name; value_pos; value_ty_name }
  in
  check_arraykey_index mk_err

let check_set_value =
  let mk_err pos (container_pos, container_ty_name) (value_pos, value_ty_name) =
    Typing_error.Primary.Invalid_set_value
      { pos; container_pos; container_ty_name; value_pos; value_ty_name }
  in
  check_arraykey_index mk_err

let pessimise_type env ty =
  Typing_union.union env ty (MakeType.dynamic (get_reason ty))

let maybe_pessimise_type env ty =
  if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
    pessimise_type env ty
  else
    (env, ty)

let pessimised_tup_assign p env arg_ty =
  let env = Env.open_tyvars env p in
  let (env, ty) = Env.fresh_type env p in
  let (env, pess_ty) = pessimise_type env ty in
  let env = Env.set_tyvar_variance env pess_ty in
  (* There can't be an error since the type variable is fresh *)
  let cb = Typing_error.Reasons_callback.unify_error_at p in
  let (env, ty_err_opt) = SubType.sub_type env arg_ty pess_ty (Some cb) in
  (* Enforce the invariant - this call should never give us an error *)
  if Option.is_some ty_err_opt then
    Errors.internal_error p "Subtype of fresh type variable";
  let (env, ty_err_opt) = Typing_solver.close_tyvars_and_solve env in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, ty)

(* Typing of array-get like expressions; [ty1] is the type of the expression
   into which we are indexing (the 'collection'), [e2] is the index expression
   and [ty2] is the type of that expression.

   We return:
   1) the (modified) typing environment,
   2) the type of the resulting expression (i.e. the type of the element we are 'getting')
   3) the actual and expected type of the indexed expression, indicating a type mismatch (if any)
   4) the actual and expected type of the indexing expression, indicating a type mismatch (if any)
   and an optional type mismatch giving the actual vs expected type of the

   The function has an error side-effect
*)
let rec array_get
    ~array_pos
    ~expr_pos
    ~expr_ty
    ?(lhs_of_null_coalesce = false)
    ?(ignore_error = false)
    is_lvalue
    env
    ty1
    e2
    ty2 =
  let ((env, ty_err1), ty1) =
    Typing_solver.expand_type_and_narrow
      env
      ~description_of_expected:"an array or collection"
      (widen_for_array_get ~lhs_of_null_coalesce ~expr_pos e2)
      array_pos
      ty1
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err1;
  GenericRules.apply_rules_with_index_value_ty_mismatches
    ~ignore_type_structure:true
    ~preserve_supportdyn:true
    env
    ty1
    (fun env ~supportdyn:_ ty1 ->
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
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Array_get_arity
                 { pos = expr_pos; name; decl_pos = Reason.to_pos r })
      in

      let nullable_container_get env ty =
        if
          (not lhs_of_null_coalesce)
          && not
               (Tast.is_under_dynamic_assumptions env.Typing_env_types.checked)
        then
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Null_container
                   {
                     pos = expr_pos;
                     null_witness =
                       lazy
                         (Reason.to_string
                            "This is what makes me believe it can be `null`."
                            r);
                   });
        array_get
          ~expr_ty
          ~array_pos
          ~expr_pos
          ~lhs_of_null_coalesce
          ~ignore_error:(not lhs_of_null_coalesce)
          is_lvalue
          env
          ty
          e2
          ty2
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
        let (env, ty_err_opt) =
          Typing_coercion.coerce_type
            ~coerce_for_op:true
            p
            reason
            env
            ty_have
            ty_expect
            Typing_error.Callback.index_type_mismatch
        in
        let ty_mismatch =
          mk_ty_mismatch_res ty_have ty_expect.et_type ty_err_opt
        in
        ((env, ty_err_opt), ty_mismatch)
      in
      let got_dynamic () =
        let tv = MakeType.dynamic r in
        let (env, idx_ty_err_opt) =
          TUtils.supports_dynamic env ty2
          @@ Some (Typing_error.Reasons_callback.unify_error_at expr_pos)
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) idx_ty_err_opt;
        let idx_err_res = mk_ty_mismatch_res ty2 tv idx_ty_err_opt in
        (env, (tv, dflt_arr_res, idx_err_res))
      in
      match ety1_ with
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cVector
             || String.equal cn SN.Collections.cVec ->
        let (env, ty) =
          match argl with
          | [ty] ->
            if String.equal cn SN.Collections.cVector then
              maybe_pessimise_type env ty
            else
              (env, ty)
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let (_, p2, _) = e2 in
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx_vector p2)) in
        let ((env, ty_err_opt), idx_err_res) =
          type_index env expr_pos ty2 ty1 (Reason.index_class cn)
        in
        Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
        (env, (ty, dflt_arr_res, idx_err_res))
      | Tclass (((_, cn) as id), _, argl)
        when cn = SN.Collections.cMap
             || cn = SN.Collections.cDict
             || cn = SN.Collections.cKeyset ->
        if cn = SN.Collections.cKeyset && is_lvalue then (
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Keyset_set
                   { pos = expr_pos; decl_pos = Reason.to_pos r });
          let (env, ty) = err_witness env expr_pos in
          (env, (ty, Ok ty2, dflt_arr_res))
        ) else
          let (k, (env, v)) =
            match argl with
            | [t] when String.equal cn SN.Collections.cKeyset -> (t, (env, t))
            | [k; v] when String.equal cn SN.Collections.cDict -> (k, (env, v))
            | [k; v] when String.( <> ) cn SN.Collections.cKeyset ->
              (k, maybe_pessimise_type env v)
            | _ ->
              arity_error id;
              let (env, ty) = err_witness env expr_pos in
              (ty, (env, ty))
          in
          (* dict and keyset are covariant in the key type, so subsumption
           * lets you upcast the key type beyond ty2 to arraykey.
           * e.g. consider $d: dict<string,int> and $i:int
           * and $d[$i] should actually type check because
           * dict<string,int> <: dict<arraykey,int>
           *)
          let ((env, ty_err_opt), idx_err_res) =
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
              let (env, res) = check_arraykey_index_read env expr_pos ty1 ty2 in
              ((env, None), res)
          in
          Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
          (env, (v, dflt_arr_res, idx_err_res))
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
          let ty_nothing = MakeType.nothing Reason.none in
          (env, (ty1, Error (ty1, ty_nothing), Ok ty2))
        else
          let (_k, (env, v)) =
            match argl with
            | [k; v] -> (k, maybe_pessimise_type env v)
            | _ ->
              arity_error id;
              let (env, ty) = err_witness env expr_pos in
              (ty, (env, ty))
          in
          let (env, idx_err_res) =
            check_arraykey_index_read env expr_pos ty1 ty2
          in
          (env, (v, dflt_arr_res, idx_err_res))
      | Tclass (((_, cn) as id), _, argl)
        when (not is_lvalue)
             && (String.equal cn SN.Collections.cConstVector
                || String.equal cn SN.Collections.cImmVector) ->
        let (env, ty) =
          match argl with
          | [ty] -> maybe_pessimise_type env ty
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let (_, p2, _) = e2 in
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx (p2, r))) in
        let ((env, ty_err1), idx_err_res) =
          type_index env expr_pos ty2 ty1 (Reason.index_class cn)
        in
        Option.iter ty_err1 ~f:(Typing_error_utils.add_typing_error ~env);
        (env, (ty, dflt_arr_res, idx_err_res))
      | Tclass ((_, cn), _, tys)
        when is_lvalue
             && (String.equal cn SN.Collections.cConstVector
                || String.equal cn SN.Collections.cImmVector) ->
        let (env, ty1) = error_const_mutation env expr_pos ty1 in
        let ty_vector =
          MakeType.class_type Reason.none SN.Collections.cVector tys
        in
        (env, (ty1, Error (ty1, ty_vector), Ok ty2))
      | Tvec_or_dict (_k, v) ->
        let (env, idx_err_res) =
          check_arraykey_index_read env expr_pos ty1 ty2
        in
        let (env, tv) = maybe_pessimise_type env v in
        (env, (tv, dflt_arr_res, idx_err_res))
      | Tvar _ when TUtils.is_tyvar_error env ty1 ->
        let (env, ty) = err_witness env expr_pos in
        (env, (ty, dflt_arr_res, Ok ty2))
      | Tdynamic
        when Typing_env_types.(
               TypecheckerOptions.enable_sound_dynamic env.genv.tcopt) ->
        got_dynamic ()
      | Tdynamic
      | Tany _ ->
        (env, (ty1, dflt_arr_res, Ok ty2))
      | Tprim Tstring ->
        let ty = MakeType.string (Reason.Rwitness expr_pos) in
        let (_, p2, _) = e2 in
        let ty1 = MakeType.enforced (MakeType.int (Reason.Ridx (p2, r))) in
        let ((env, ty_err1), idx_err_res) =
          type_index env expr_pos ty2 ty1 Reason.index_array
        in
        Option.iter ty_err1 ~f:(Typing_error_utils.add_typing_error ~env);
        (env, (ty, dflt_arr_res, idx_err_res))
      | Ttuple tyl ->
        (* requires integer literal *)
        (match e2 with
        | (_, p, Int n) ->
          let idx = int_of_string_opt n in
          (match Option.bind idx ~f:(List.nth tyl) with
          | Some nth -> (env, (nth, dflt_arr_res, Ok ty2))
          | None ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(
                primary
                @@ Primary.Generic_unify
                     {
                       pos = p;
                       msg = Reason.string_of_ureason Reason.index_tuple;
                     });
            let (env, ty) = err_witness env p in
            (env, (ty, dflt_arr_res, Ok ty2)))
        | (_, p, _) ->
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Generic_unify
                   {
                     pos = p;
                     msg = Reason.string_of_ureason Reason.URtuple_access;
                   });
          let (env, ty) = err_witness env expr_pos in
          (env, (ty, dflt_arr_res, Error (ty2, MakeType.int Reason.none))))
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cPair ->
        let (ty_fst, ty_snd) =
          match argl with
          | [ty_fst; ty_snd] -> (ty_fst, ty_snd)
          | _ ->
            arity_error id;
            let (_env, ty) = err_witness env expr_pos in
            (ty, ty)
        in
        (* requires integer literal *)
        (match e2 with
        | (_, p, Int n) ->
          let idx = int_of_string_opt n in
          (match Option.bind ~f:(List.nth [ty_fst; ty_snd]) idx with
          | Some nth -> (env, (nth, dflt_arr_res, Ok ty2))
          | None ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(
                primary
                @@ Primary.Generic_unify
                     {
                       pos = p;
                       msg = Reason.string_of_ureason (Reason.index_class cn);
                     });
            let (env, ty) = err_witness env p in
            (env, (ty, dflt_arr_res, Ok ty2)))
        | (_, p, _) ->
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Generic_unify
                   {
                     pos = p;
                     msg = Reason.string_of_ureason Reason.URpair_access;
                   });
          let (env, ty) = err_witness env p in
          (env, (ty, dflt_arr_res, Error (ty2, MakeType.int Reason.none))))
      | Tshape { s_fields = fdm; _ } ->
        let (_, p, _) = e2 in
        begin
          let (fld_opt, ty_err_opt) =
            TUtils.shape_field_name_with_ty_err env e2
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          match fld_opt with
          | None ->
            (* there was already an error in shape_field name,
               don't report another one for a missing field *)
            let (env, ty) = err_witness env p in
            (env, (ty, dflt_arr_res, Ok ty2))
          | Some field ->
            let field = TShapeField.of_ast Pos_or_decl.of_raw_pos field in
            if is_lvalue || lhs_of_null_coalesce then
              (* The expression $s['x'] ?? $y is semantically equivalent to
                 Shapes::idx ($s, 'x') ?? $y.  I.e., if $s['x'] occurs on
                 the left of a coalesce operator, then for type checking it
                 can be treated as if it evaluated to null instead of
                 throwing an exception if the field 'x' doesn't exist in $s.
              *)
              let (env, ty) =
                Typing_shapes.idx_without_default
                  env
                  ty1
                  field
                  ~expr_pos
                  ~shape_pos:array_pos
              in
              (env, (ty, dflt_arr_res, Ok ty2))
            else begin
              match TShapeMap.find_opt field fdm with
              | None ->
                Typing_error_utils.add_typing_error
                  ~env
                  Typing_error.(
                    primary
                    @@ Primary.Undefined_field
                         {
                           pos = p;
                           name = TUtils.get_printable_shape_field_name field;
                           decl_pos = Reason.to_pos r;
                         });
                let (env, ty) = err_witness env p in
                (env, (ty, dflt_arr_res, Ok ty2))
              | Some { sft_optional; sft_ty } ->
                if sft_optional then (
                  let declared_field =
                    List.find_exn
                      ~f:(fun x -> TShapeField.equal field x)
                      (TShapeMap.keys fdm)
                  in
                  Typing_error_utils.add_typing_error
                    ~env
                    Typing_error.(
                      primary
                      @@ Primary.Array_get_with_optional_field
                           {
                             recv_pos = array_pos;
                             field_pos = p;
                             field_name =
                               TUtils.get_printable_shape_field_name field;
                             decl_pos =
                               Typing_defs.TShapeField.pos declared_field;
                           });
                  let (env, ty) = err_witness env p in
                  (env, (ty, dflt_arr_res, Ok ty2))
                ) else
                  (env, (sft_ty, dflt_arr_res, Ok ty2))
            end
        end
      | Toption ty ->
        let (env, (ty, err_opt_arr, err_opt_idx)) =
          nullable_container_get env ty
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
        (env, (ty, err_res_arr, err_res_idx))
      | Tprim Tnull ->
        if Tast.is_under_dynamic_assumptions env.Typing_env_types.checked then
          got_dynamic ()
        else
          let ty = MakeType.nothing Reason.Rnone in
          let (env, (ty, err_opt_arr, err_opt_idx)) =
            nullable_container_get env ty
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
          (env, (ty, err_res_arr, err_res_idx))
      | Tnewtype (cid, _, _bound) when String.equal cid SN.Classes.cSupportDyn
        ->
        (* We must be under_dynamic_assumptions because
           apply_rules_with_index_value_ty_mismatches otherwise descends into the newtype *)
        got_dynamic ()
      | Tnewtype (ts, [ty], bound) -> begin
        match deref bound with
        | ( r,
            Tshape
              { s_origin = _; s_unknown_value = shape_kind; s_fields = fields }
          )
          when String.equal ts SN.FB.cTypeStructure ->
          let (env, fields) =
            Typing_structure.transform_shapemap env array_pos ty fields
          in
          let ty =
            mk
              ( r,
                Tshape
                  {
                    s_origin = Missing_origin;
                    s_unknown_value = shape_kind;
                    s_fields = fields;
                  } )
          in
          let (env, (ty, err_opt_arr, err_opt_idx)) =
            array_get
              ~array_pos
              ~expr_pos
              ~expr_ty
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
          (env, (ty, err_res_arr, err_res_idx))
        | _ ->
          if not ignore_error then error_array env expr_pos expr_ty;
          let (env, res_ty) = err_witness env expr_pos in
          let ty_nothing = MakeType.nothing Reason.none in
          let (env, ty_key) =
            MakeType.arraykey Reason.none
            |> Typing_intersection.intersect ~r:Reason.Rnone env ty2
          in
          let ty_keyedcontainer =
            MakeType.(keyed_container Reason.none ty_key ty_nothing)
          in
          (env, (res_ty, Error (ty1, ty_keyedcontainer), Ok ty2))
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
        if not ignore_error then error_array env expr_pos expr_ty;
        let (env, res_ty) = err_witness env expr_pos in
        let ty_nothing = MakeType.nothing Reason.none in
        let (env, ty_key) =
          MakeType.arraykey Reason.none
          |> Typing_intersection.intersect ~r:Reason.Rnone env ty2
        in
        let ty_keyedcontainer =
          MakeType.(keyed_container Reason.none ty_key ty_nothing)
        in
        (env, (res_ty, Error (ty1, ty_keyedcontainer), Ok ty2))
      (* Type-check array access as though it is the method
       * array_get<Tk,Tv>(KeyedContainer<Tk,Tv> $array, Tk $key): Tv
       * (We can already force Tk to be the type of the key argument because
       * Tk does not appear in the result of the call)
       *)
      | Tvar _ ->
        let (env, value) = Env.fresh_type env expr_pos in
        let (env, ty_key) =
          MakeType.arraykey Reason.none
          |> Typing_intersection.intersect ~r:Reason.Rnone env ty2
        in
        let keyed_container = MakeType.keyed_container r ty_key value in
        let (env, arr_ty_err_opt) =
          SubType.sub_type env ty1 keyed_container
          @@ Some
               (Typing_error.Reasons_callback.index_type_mismatch_at expr_pos)
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) arr_ty_err_opt;
        let arr_res = mk_ty_mismatch_res ty1 keyed_container arr_ty_err_opt in
        (env, (value, arr_res, Ok ty2)))

(* Given a type `ty` known to be a lower bound on the type of the array operand
 * to an array append operation, compute the largest upper bound on that type
 * that validates the get operation. For example, if `vec<string>` is a lower
 * bound, then `vec<#1>` is a suitable upper bound.`
 *)
let widen_for_assign_array_append ~expr_pos env ty =
  match deref ty with
  (* dynamic is valid for array append *)
  | (_, Tdynamic) -> ((env, None), Some ty)
  | (r, Tclass (((_, cn) as id), _, tyl))
    when String.equal cn SN.Collections.cVec
         || String.equal cn SN.Collections.cKeyset
         || String.equal cn SN.Collections.cVector
         || String.equal cn SN.Collections.cMap ->
    let (env, params) =
      List.map_env env tyl ~f:(fun env _ty ->
          Env.fresh_type_invariant env expr_pos)
    in
    let ty = mk (r, Tclass (id, nonexact, params)) in
    ((env, None), Some ty)
  | _ -> ((env, None), None)

let assign_array_append ~array_pos ~expr_pos ur env ty1 ty2 =
  let ((env, ty_err1), ty1) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"an array or collection"
      env
      (widen_for_assign_array_append ~expr_pos)
      array_pos
      ty1
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err1;
  GenericRules.apply_rules_with_index_value_ty_mismatches
    ~preserve_supportdyn:false
    env
    ty1
    (fun env ~supportdyn ty1 ->
      (* In dynamic mode strip off nullable because we can always upcast to dynamic *)
      let ty1 =
        match get_node ty1 with
        | Toption ty
          when Tast.is_under_dynamic_assumptions env.Typing_env_types.checked ->
          ty
        | _ -> ty1
      in
      let got_dynamic () =
        let tv = MakeType.dynamic (get_reason ty1) in
        let (env, val_ty_err_opt) =
          TUtils.supports_dynamic env ty2
          @@ Some (Typing_error.Reasons_callback.unify_error_at expr_pos)
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) val_ty_err_opt;
        let val_err_res = mk_ty_mismatch_res ty2 tv val_ty_err_opt in
        (env, (tv, Ok tv, val_err_res))
      in
      match deref ty1 with
      | (_, Tany _) -> (env, (ty1, Ok ty1, Ok ty2))
      | (r, Tclass ((_, n), _, [tv])) when String.equal n SN.Collections.cVector
        ->
        let (env, tv) = maybe_make_supportdyn r env ~supportdyn tv in
        let (env, tv) = maybe_pessimise_type env tv in
        let (env, val_ty_err_opt) =
          Typing_ops.sub_type
            expr_pos
            ur
            env
            ty2
            tv
            Typing_error.Callback.unify_error
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) val_ty_err_opt;
        let val_err_res = mk_ty_mismatch_res ty2 tv val_ty_err_opt in
        (env, (ty1, Ok ty1, val_err_res))
      (* Handle the case where Vector or Set was used as a typehint
         without type parameters *)
      | (_, Tclass ((_, n), _, [])) when String.equal n SN.Collections.cVector
        ->
        (env, (ty1, Ok ty1, Ok ty2))
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
        (env, (ty, Ok ty, val_err_res))
      | (r, Tclass (((_, n) as id), e, [tv]))
        when String.equal n SN.Collections.cVec ->
        let (env, tv) = maybe_make_supportdyn r env ~supportdyn tv in
        let (env, tv') = Typing_union.union env tv ty2 in
        let ty = mk (r, Tclass (id, e, [tv'])) in
        (env, (ty, Ok ty, Ok ty2))
      | (r, Tclass (((_, n) as id), e, [tv]))
        when String.equal n SN.Collections.cKeyset ->
        let (env, err_res) = check_keyset_value env expr_pos ty1 ty2 in
        let (env, tk') =
          let r = Reason.Rkey_value_collection_key expr_pos in
          let ak_t = MakeType.arraykey r in
          let (env, ty2) = Typing_intersection.intersect env ~r ak_t ty2 in
          Typing_union.union env tv ty2
        in
        let ty = mk (r, Tclass (id, e, [tk'])) in
        (env, (ty, Ok ty, err_res))
      | (_, Tclass ((_, n), _, [tv])) when String.equal n SN.Collections.cSet ->
        let (env, err_res) =
          match check_set_value env expr_pos ty1 ty2 with
          | (_, Error _) as err_res -> err_res
          | (env, _) ->
            let (env, tv') =
              let ak_t = MakeType.arraykey (Reason.Ridx_vector expr_pos) in
              let (env, tv) = maybe_pessimise_type env tv in
              if TUtils.is_sub_type_for_union env ak_t tv then
                (* hhvm will enforce that the key is an arraykey, so if
                   $x : Set<arraykey>, then it should be allowed to
                   set $x[] = e where $d : dynamic. *)
                (env, MakeType.enforced tv)
              else
                (* It is unsound to allow $x[] = e if $x : Set<string>
                   since the dynamic $d might be an int and hhvm wouldn't
                   complain.*)
                (env, MakeType.unenforced tv)
            in
            let (env, ty_err_opt) =
              Typing_coercion.coerce_type
                ~coerce_for_op:true
                expr_pos
                ur
                env
                ty2
                tv'
                Typing_error.Callback.unify_error
            in
            Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
            let ty_mismatch = mk_ty_mismatch_res ty2 tv ty_err_opt in
            (env, ty_mismatch)
        in
        (env, (ty1, Ok ty1, err_res))
      | (_, Tdynamic)
        when Typing_env_types.(
               TypecheckerOptions.enable_sound_dynamic env.genv.tcopt) ->
        got_dynamic ()
      | (_, Tdynamic) -> (env, (ty1, Ok ty1, Ok ty2))
      | (_, Tunapplied_alias _) ->
        Typing_defs.error_Tunapplied_alias_in_illegal_context ()
      | (_, Tprim Tnull)
        when Tast.is_under_dynamic_assumptions env.Typing_env_types.checked ->
        got_dynamic ()
      | (_, Tnewtype (cid, _, _bound))
        when String.equal cid SN.Classes.cSupportDyn ->
        (* We must be under_dynamic_assumptions because
           apply_rules_with_index_value_ty_mismatches otherwise descends into the newtype.
           In this case we just accept the assignment because it's as though
           we applied an implicit upcast to dynamic
        *)
        got_dynamic ()
      | ( r,
          ( Tnonnull | Tvec_or_dict _ | Toption _ | Tprim _ | Tvar _ | Tfun _
          | Tclass _ | Ttuple _ | Tshape _ | Tunion _ | Tintersection _
          | Tgeneric _ | Tnewtype _ | Tdependent _ | Taccess _ | Tneg _ ) ) ->
        let (env, ty) = error_assign_array_append env expr_pos ty1 in
        let (env, ty) = maybe_make_supportdyn r env ~supportdyn ty in
        let ty_nothing = MakeType.nothing Reason.none in
        (env, (ty, Error (ty, ty_nothing), Error (ty2, ty_nothing))))

let widen_for_assign_array_get ~expr_pos index_expr env ty =
  Typing_log.(
    log_with_level env "typing" ~level:1 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos expr_pos)
          env
          [Log_head ("widen_for_assign_array_get", [Log_type ("ty", ty)])]));
  match deref ty with
  (* dynamic is valid for assign array get *)
  | (_, Tdynamic) -> ((env, None), Some ty)
  | (r, Ttuple tyl) -> begin
    (* requires integer literal *)
    match index_expr with
    (* Should freshen type variables *)
    | (_, _, Aast.Int _) ->
      let (env, params) =
        List.map_env env tyl ~f:(fun env _ty ->
            Env.fresh_type_invariant env expr_pos)
      in
      ((env, None), Some (mk (r, Ttuple params)))
    | _ -> ((env, None), None)
  end
  | _ -> ((env, None), None)

(* Used for typing an assignment e1[key] = e2
 * where e1 has type ty1, key has type tkey and e2 has type ty2.
 * Return the new array type
 *)

let assign_array_get ~array_pos ~expr_pos ur env ty1 (key : Nast.expr) tkey ty2
    =
  Typing_log.(
    log_with_level env "typing" ~level:1 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos expr_pos)
          env
          [
            Log_head
              ( "assign_array_get",
                [Log_type ("ty1", ty1); Log_type ("ty2", ty2)] );
          ]));
  let ((env, ty_err1), ety1) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"an array or collection"
      env
      (widen_for_assign_array_get ~expr_pos key)
      array_pos
      ty1
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err1;
  GenericRules.apply_rules_with_array_index_value_ty_mismatches
    ~preserve_supportdyn:false
    env
    ety1
    (fun env ~supportdyn ety1 ->
      let (r, ety1_) = deref ety1 in
      let arity_error (_, name) =
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Array_get_arity
                 { pos = expr_pos; name; decl_pos = Reason.to_pos r })
      in
      let type_index env p ty_have ty_expect reason =
        let (env, ty_err_opt) =
          Typing_coercion.coerce_type
            ~coerce_for_op:true
            p
            reason
            env
            ty_have
            ty_expect
            Typing_error.Callback.index_type_mismatch
        in
        let ty_mismatch =
          mk_ty_mismatch_res ty_have ty_expect.et_type ty_err_opt
        in
        ((env, ty_err_opt), ty_mismatch)
      in
      let got_dynamic () =
        let tv = MakeType.dynamic r in
        let (env, ty_err1) =
          TUtils.supports_dynamic env tkey
          @@ Some (Typing_error.Reasons_callback.unify_error_at expr_pos)
        in
        let idx_err_res = mk_ty_mismatch_res tkey tv ty_err1 in
        let (env, ty_err2) =
          TUtils.supports_dynamic env ty2
          @@ Some (Typing_error.Reasons_callback.unify_error_at expr_pos)
        in
        let val_err_res = mk_ty_mismatch_res tkey tv ty_err2 in
        Option.(
          iter ~f:(Typing_error_utils.add_typing_error ~env)
          @@ merge ty_err1 ty_err2 ~f:Typing_error.both);
        (env, (tv, Ok tv, idx_err_res, val_err_res))
      in
      match ety1_ with
      | Tclass (((_, cn) as id), _, argl)
        when String.equal cn SN.Collections.cVector ->
        let (env, tv) =
          match argl with
          | [tv] -> maybe_pessimise_type env tv
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let (env, tv) = maybe_make_supportdyn r env ~supportdyn tv in
        let (_, p, _) = key in
        let tk = MakeType.enforced (MakeType.int (Reason.Ridx_vector p)) in
        let ((env, ty_err1), idx_err) =
          type_index env expr_pos tkey tk (Reason.index_class cn)
        in
        let (env, ty_err2) =
          Typing_ops.sub_type
            expr_pos
            ur
            env
            ty2
            tv
            Typing_error.Callback.unify_error
        in
        let err_res = mk_ty_mismatch_res ty2 tv ty_err2 in
        Option.(
          iter ~f:(Typing_error_utils.add_typing_error ~env)
          @@ merge ty_err1 ty_err2 ~f:Typing_error.both);
        (env, (ety1, Ok ety1, idx_err, err_res))
      | Tclass (((_, cn) as id), e, argl)
        when String.equal cn SN.Collections.cVec ->
        let (env, tv) =
          match argl with
          | [tv] -> (env, tv)
          | _ ->
            arity_error id;
            err_witness env expr_pos
        in
        let (_, p, _) = key in
        let tk = MakeType.enforced (MakeType.int (Reason.Ridx_vector p)) in
        let ((env, ty_err1), idx_err) =
          type_index env expr_pos tkey tk (Reason.index_class cn)
        in
        let (env, tv) = maybe_make_supportdyn r env ~supportdyn tv in
        let (env, tv') = Typing_union.union env tv ty2 in
        let ty = mk (r, Tclass (id, e, [tv'])) in
        Option.iter ty_err1 ~f:(Typing_error_utils.add_typing_error ~env);
        (env, (ty, Ok ty, idx_err, Ok ty2))
      | Tclass (((_, cn) as id), _, argl) when cn = SN.Collections.cMap ->
        let (env, idx_err1) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (tk, (env, tv)) =
          match argl with
          | [tk; tv] -> (tk, maybe_pessimise_type env tv)
          | _ ->
            arity_error id;
            let (env, ty) = err_witness env expr_pos in
            (ty, (env, ty))
        in
        let (env, tv) = maybe_make_supportdyn r env ~supportdyn tv in
        let (env, tk) =
          let (_, p, _) = key in
          let ak_t = MakeType.arraykey (Reason.Ridx_vector p) in
          let (env, tk) = maybe_pessimise_type env tk in
          if TUtils.is_sub_type_for_union env ak_t tk then
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
        let ((env, ty_err1), idx_err2) =
          type_index env expr_pos tkey tk (Reason.index_class cn)
        in
        let idx_err =
          match (idx_err1, idx_err2) with
          | (Error _, _) -> idx_err1
          | _ -> idx_err2
        in
        let (env, ty_err2) =
          Typing_ops.sub_type
            expr_pos
            ur
            env
            ty2
            tv
            Typing_error.Callback.unify_error
        in
        let err_res = mk_ty_mismatch_res ty2 tv ty_err2 in
        Option.(
          iter ~f:(Typing_error_utils.add_typing_error ~env)
          @@ merge ty_err1 ty_err2 ~f:Typing_error.both);
        (env, (ety1, Ok ety1, idx_err, err_res))
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
            let (_env, ty) = err_witness env expr_pos in
            (ty, ty)
        in
        let (env, tv) = maybe_make_supportdyn r env ~supportdyn tv in
        let (env, tk') =
          let (_, p, _) = key in
          let ak_t = MakeType.arraykey (Reason.Ridx_dict p) in
          match idx_err with
          | Ok _ ->
            let (env, tkey_new) =
              Typing_intersection.intersect
                env
                ~r:(Reason.Ridx_dict p)
                tkey
                ak_t
            in
            Typing_union.union env tk tkey_new
          | _ -> Typing_union.union env tk tkey
        in
        let (env, tv') = Typing_union.union env tv ty2 in
        let ty = mk (r, Tclass (id, e, [tk'; tv'])) in
        (env, (ty, Ok ty, idx_err, Ok ty2))
      | Tclass ((_, cn), _, _) when String.equal cn SN.Collections.cKeyset ->
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Keyset_set { pos = expr_pos; decl_pos = Reason.to_pos r });
        ( env,
          (ety1, Error (ety1, MakeType.nothing Reason.none), Ok tkey, Ok ty2) )
      | Tclass ((_, cn), _, tys)
        when String.equal cn SN.Collections.cConstMap
             || String.equal cn SN.Collections.cImmMap ->
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Const_mutation
                 {
                   pos = expr_pos;
                   decl_pos = Reason.to_pos r;
                   ty_name = lazy (Typing_print.error env ety1);
                 });
        let ty_expect =
          MakeType.class_type Reason.none SN.Collections.cMap tys
        in
        (env, (ety1, Error (ety1, ty_expect), Ok tkey, Ok ty2))
      | Tclass ((_, cn), _, tys)
        when String.equal cn SN.Collections.cConstVector
             || String.equal cn SN.Collections.cImmVector ->
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Const_mutation
                 {
                   pos = expr_pos;
                   decl_pos = Reason.to_pos r;
                   ty_name = lazy (Typing_print.error env ety1);
                 });
        let ty_expect =
          MakeType.class_type Reason.none SN.Collections.cVector tys
        in
        let (env, ety1) = maybe_make_supportdyn r env ~supportdyn ety1 in
        (env, (ety1, Error (ety1, ty_expect), Ok tkey, Ok ty2))
      | Tclass ((_, cn), _, _)
        when String.equal cn SN.Collections.cAnyArray
             && Tast.is_under_dynamic_assumptions env.Typing_env_types.checked
        ->
        got_dynamic ()
      | Tclass ((_, cn), _, _)
        when String.equal cn SN.Collections.cKeyedContainer
             || String.equal cn SN.Collections.cAnyArray
             || String.equal cn SN.Collections.cPair ->
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Const_mutation
                 {
                   pos = expr_pos;
                   decl_pos = Reason.to_pos r;
                   ty_name = lazy (Typing_print.error env ety1);
                 });
        let ty_expect = MakeType.nothing Reason.none in
        let (env, ety1) = maybe_make_supportdyn r env ~supportdyn ety1 in
        (env, (ety1, Error (ety1, ty_expect), Ok tkey, Ok ty2))
      | Tvec_or_dict (tk, tv) ->
        let (env, idx_err) =
          check_arraykey_index_write env expr_pos ety1 tkey
        in
        let (env, tk') = Typing_union.union env tk tkey in
        let (env, tv) = maybe_make_supportdyn r env ~supportdyn tv in
        let (env, tv') = Typing_union.union env tv ty2 in
        let ty = mk (r, Tvec_or_dict (tk', tv')) in
        (env, (ty, Ok ty, idx_err, Ok ty2))
      | Tvar _ when TUtils.is_tyvar_error env ety1 ->
        let (env, ty) = Env.fresh_type_error env expr_pos in
        let (env, ty) = maybe_make_supportdyn r env ~supportdyn ty in
        (env, (ty, Ok ty, Ok tkey, Ok ty2))
      | Tdynamic
        when Typing_env_types.(
               TypecheckerOptions.enable_sound_dynamic env.genv.tcopt) ->
        got_dynamic ()
      | Tdynamic -> (env, (ety1, Ok ety1, Ok tkey, Ok ty2))
      | Tany _ -> (env, (ety1, Ok ety1, Ok tkey, Ok ty2))
      | Tprim Tnull
        when Tast.is_under_dynamic_assumptions env.Typing_env_types.checked ->
        got_dynamic ()
      | Tprim Tstring ->
        let (_, p, _) = key in
        let tk = MakeType.enforced (MakeType.int (Reason.Ridx (p, r))) in
        let tv = MakeType.string (Reason.Rwitness expr_pos) in
        let ((env, ty_err1), idx_err) =
          type_index env expr_pos tkey tk Reason.index_array
        in
        let (env, tv') = maybe_pessimise_type env tv in
        let (env, ty_err2) =
          Typing_ops.sub_type
            expr_pos
            ur
            env
            ty2
            tv'
            Typing_error.Callback.unify_error
        in
        let err_res = mk_ty_mismatch_res ty2 tv ty_err2 in
        Option.(
          iter ~f:(Typing_error_utils.add_typing_error ~env)
          @@ merge ty_err1 ty_err2 ~f:Typing_error.both);
        (env, (ety1, Ok ety1, idx_err, err_res))
      | Ttuple tyl ->
        let fail key_err reason =
          let (_, p, _) = key in
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Generic_unify
                   { pos = p; msg = Reason.string_of_ureason reason });
          (env, (ety1, Ok ety1, key_err, Ok ty2))
        in
        begin
          match key with
          | (_, _, Int n) ->
            let idx = int_of_string_opt n in
            (match Option.map ~f:(List.split_n tyl) idx with
            | Some (tyl', _ :: tyl'') ->
              let ty = MakeType.tuple r (tyl' @ (ty2 :: tyl'')) in
              let (env, ty) = maybe_make_supportdyn r env ~supportdyn ty in
              (env, (ty, Ok ety1, Ok tkey, Ok ty2))
            | _ -> fail (Ok tkey) Reason.index_tuple)
          | _ ->
            fail (Error (tkey, MakeType.int Reason.none)) Reason.URtuple_access
        end
      | Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fdm } ->
      begin
        let (fld_opt, ty_err_opt) =
          TUtils.shape_field_name_with_ty_err env key
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        match fld_opt with
        | None ->
          let (env, ety1) = maybe_make_supportdyn r env ~supportdyn ety1 in
          (env, (ety1, Ok ety1, Ok tkey, Ok ty2))
        | Some field ->
          let field = TShapeField.of_ast Pos_or_decl.of_raw_pos field in
          let (env, fdm) =
            if supportdyn then
              let f env _name { sft_optional; sft_ty } =
                let (env, sft_ty) = TUtils.make_supportdyn r env sft_ty in
                (env, { sft_optional; sft_ty })
              in
              TShapeMap.map_env f env fdm
            else
              (env, fdm)
          in
          let fdm' =
            TShapeMap.add field { sft_optional = false; sft_ty = ty2 } fdm
          in
          let ty =
            mk
              ( r,
                Tshape
                  {
                    s_origin = Missing_origin;
                    s_unknown_value = shape_kind;
                    s_fields = fdm';
                  } )
          in
          (env, (ty, Ok ty, Ok tkey, Ok ty2))
      end
      | Tnewtype (cid, _, _bound) when String.equal cid SN.Classes.cSupportDyn
        ->
        (* We must be under_dynamic_assumptions because
           apply_rules_with_index_value_ty_mismatches otherwise descends into the newtype.
           In this case we just accept the assignment because it's as though
           we applied an implicit upcast to dynamic
        *)
        (env, (ety1, Ok ety1, Ok tkey, Ok ty2))
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
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Array_access
                 {
                   ctxt = `write;
                   pos = expr_pos;
                   decl_pos = Reason.to_pos r;
                   ty_name = lazy (Typing_print.error env ety1);
                 });
        let ty_nothing = MakeType.nothing Reason.none in
        let (env, ety1) = maybe_make_supportdyn r env ~supportdyn ety1 in
        (env, (ety1, Error (ety1, ty_nothing), Ok tkey, Error (ty2, ty_nothing))))
