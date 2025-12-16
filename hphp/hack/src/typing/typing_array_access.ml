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
module GenericRules = Typing_generic_rules
module SN = Naming_special_names
open String.Replace_polymorphic_compare

let mk_ty_mismatch_res ty_have ty_expect =
  Option.value_map
    ~default:(Ok ty_have)
    ~f:Fn.(const @@ Error (ty_have, ty_expect))

let err_witness env p = Env.fresh_type_error env p

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

(* Check that an index to a map-like collection passes the basic test of
 * being a subtype of arraykey
 *)
let check_arraykey_index error env pos container_ty index_ty =
  let (env, container_ty) = Env.expand_type env container_ty in
  let reason =
    match get_node container_ty with
    | Tclass ((_, cn), _, _) -> Reason.index_class cn
    | _ -> Reason.index_array
  in
  let info_of_type ty = (get_pos ty, lazy (Typing_print.error env ty)) in
  let container_info = info_of_type container_ty in
  let index_info = info_of_type index_ty in
  let ty_arraykey = MakeType.arraykey (Reason.idx_dict pos) in
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
      ty_arraykey
      Enforced
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

let type_index ~read env p ty_have ty_expect enforced reason =
  let log_name =
    if read then
      "array_get/type_index"
    else
      "assign_array_set/type_index"
  in
  Typing_log.(
    log_with_level env "typing" ~level:1 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos p)
          env
          [
            Log_head
              ( log_name,
                [
                  Log_type ("ty_have", ty_have);
                  Log_type ("ty_expect", ty_expect);
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
      enforced
      Typing_error.Callback.index_type_mismatch
  in
  let ty_mismatch = mk_ty_mismatch_res ty_have ty_expect ty_err_opt in
  ((env, ty_err_opt), ty_mismatch)

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
    let ty = mk (r, Tclass (id, nonexact, params)) in
    (env, Some ty)
  | _ -> (env, None)

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
          let r = Reason.key_value_collection_key expr_pos in
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
            let (env, tv', enforced) =
              let ak_t = MakeType.arraykey (Reason.idx_vector expr_pos) in
              let (env, tv) = maybe_pessimise_type env tv in
              if TUtils.is_sub_type_for_union env ak_t tv then
                (* hhvm will enforce that the key is an arraykey, so if
                   $x : Set<arraykey>, then it should be allowed to
                   set $x[] = e where $d : dynamic. *)
                (env, tv, Enforced)
              else
                (* It is unsound to allow $x[] = e if $x : Set<string>
                   since the dynamic $d might be an int and hhvm wouldn't
                   complain.*)
                (env, tv, Unenforced)
            in
            let (env, ty_err_opt) =
              Typing_coercion.coerce_type
                ~coerce_for_op:true
                expr_pos
                ur
                env
                ty2
                tv'
                enforced
                Typing_error.Callback.unify_error
            in
            Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
            let ty_mismatch = mk_ty_mismatch_res ty2 tv ty_err_opt in
            (env, ty_mismatch)
        in
        (env, (ty1, Ok ty1, err_res))
      | (_, Tdynamic) -> got_dynamic ()
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
          | Tlabel _ | Tgeneric _ | Tnewtype _ | Tdependent _ | Taccess _
          | Tneg _ | Tclass_ptr _ ) ) ->
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
  | (_, Tdynamic) -> (env, Some ty)
  | (r, Ttuple { t_required; t_optional; t_extra = Tvariadic t_variadic }) ->
  begin
    (* requires integer literal *)
    match index_expr with
    (* Should freshen type variables *)
    | (_, _, Aast.Int _) ->
      let (env, t_required) =
        List.map_env env t_required ~f:(fun env _ty ->
            Env.fresh_type_invariant env expr_pos)
      in
      let (env, t_optional) =
        List.map_env env t_optional ~f:(fun env _ty ->
            Env.fresh_type_invariant env expr_pos)
      in
      ( env,
        Some
          (mk
             ( r,
               Ttuple { t_required; t_optional; t_extra = Tvariadic t_variadic }
             )) )
    | _ -> (env, None)
  end
  | _ -> (env, None)

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
        let tk = MakeType.int (Reason.idx_vector p) in
        let ((env, ty_err1), idx_err) =
          type_index
            ~read:false
            env
            expr_pos
            tkey
            tk
            Enforced
            (Reason.index_class cn)
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
        let tk = MakeType.int (Reason.idx_vector p) in
        let ((env, ty_err1), idx_err) =
          type_index
            ~read:false
            env
            expr_pos
            tkey
            tk
            Enforced
            (Reason.index_class cn)
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
        let (env, enforced, tk) =
          let (_, p, _) = key in
          let ak_t = MakeType.arraykey (Reason.idx_vector p) in
          let (env, tk) = maybe_pessimise_type env tk in
          if TUtils.is_sub_type_for_union env ak_t tk then
            (* hhvm will enforce that the key is an arraykey, so if
               $x : Map<arraykey, t>, then it should be allowed to
               set $x[$d] = e where $d : dynamic. NB above, we don't need
               to check that tk <: arraykey, because the Map ensures that. *)
            (env, Enforced, tk)
          else
            (* It is unsound to allow $x[$d] = e if $x : Map<string, t>
               since the dynamic $d might be an int and hhvm wouldn't
               complain.*)
            (env, Unenforced, tk)
        in
        let ((env, ty_err1), idx_err2) =
          type_index
            ~read:false
            env
            expr_pos
            tkey
            tk
            enforced
            (Reason.index_class cn)
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
          let ak_t = MakeType.arraykey (Reason.idx_dict p) in
          match idx_err with
          | Ok _ ->
            let (env, tkey_new) =
              Typing_intersection.intersect env ~r:(Reason.idx_dict p) tkey ak_t
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
      | Tdynamic -> got_dynamic ()
      | Tany _ -> (env, (ety1, Ok ety1, Ok tkey, Ok ty2))
      | Tprim Tnull
        when Tast.is_under_dynamic_assumptions env.Typing_env_types.checked ->
        got_dynamic ()
      | Tclass ((_, id), _, _) when String.equal id SN.Classes.cString ->
        let (_, p, _) = key in
        let tk = MakeType.int (Reason.idx (p, r)) in
        let tv = MakeType.string (Reason.witness expr_pos) in
        let ((env, ty_err1), idx_err) =
          type_index
            ~read:false
            env
            expr_pos
            tkey
            tk
            Enforced
            Reason.index_array
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
      (* TODO T201398626 T201398652 *)
      | Ttuple { t_required; t_optional = []; t_extra = Tvariadic _ } ->
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
            (match Option.map ~f:(List.split_n t_required) idx with
            | Some (tyl', _ :: tyl'') ->
              let ty = MakeType.tuple r (tyl' @ (ty2 :: tyl'')) in
              let (env, ty) = maybe_make_supportdyn r env ~supportdyn ty in
              (env, (ty, Ok ety1, Ok tkey, Ok ty2))
            | _ -> fail (Ok tkey) Reason.index_tuple)
          | _ ->
            fail (Error (tkey, MakeType.int Reason.none)) Reason.URtuple_access
        end
      | Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fdm } ->
        Typing_shapes.do_with_field_expr
          env
          key
          ~with_error:
            (let (env, ety1) = maybe_make_supportdyn r env ~supportdyn ety1 in
             (env, (ety1, Ok ety1, Ok tkey, Ok ty2)))
        @@ fun field ->
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
      | Tnewtype (cid, _, _bound) when String.equal cid SN.Classes.cSupportDyn
        ->
        (* We must be under_dynamic_assumptions because
           apply_rules_with_index_value_ty_mismatches otherwise descends into the newtype.
           In this case we just accept the assignment because it's as though
           we applied an implicit upcast to dynamic
        *)
        (env, (ety1, Ok ety1, Ok tkey, Ok ty2))
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
      | Tlabel _
      | Ttuple _
      | Tneg _
      | Tclass_ptr _ ->
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
