(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Common
open Typing_defs
module Env = Typing_env
module MakeType = Typing_make_type
module Reason = Typing_reason
module SN = Naming_special_names

let is_sub_dynamic env t =
  Typing_solver.is_sub_type env t (MakeType.dynamic Reason.Rnone)

let is_float_or_like_float env t =
  let (env, like_float) =
    Typing_union.union
      env
      (MakeType.dynamic Reason.Rnone)
      (MakeType.float Reason.Rnone)
  in
  Typing_solver.is_sub_type env t like_float && not (is_sub_dynamic env t)

let is_int_or_like_int env t =
  let (env, like_int) =
    Typing_union.union
      env
      (MakeType.dynamic Reason.Rnone)
      (MakeType.int Reason.Rnone)
  in
  Typing_solver.is_sub_type env t like_int && not (is_sub_dynamic env t)

let is_float env t =
  Typing_solver.is_sub_type env t (MakeType.float Reason.Rnone)

let is_int env t = Typing_solver.is_sub_type env t (MakeType.int Reason.Rnone)

let is_num env t =
  let is_tyvar_with_num_upper_bound ty =
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Tvar v ->
      Internal_type_set.mem
        (LoclType (MakeType.num Reason.Rnone))
        (Env.get_tyvar_upper_bounds env v)
    | _ -> false
  in
  Typing_solver.is_sub_type env t (MakeType.num Reason.Rnone)
  || is_tyvar_with_num_upper_bound t

let is_super_num env t =
  Typing_subtype.is_sub_type_for_union env (MakeType.num Reason.Rnone) t

(* Checking of numeric operands for arithmetic operators:
 *    (1) Enforce that it coerces to num
 *    (2) Solve to float if it's a type variable with float as lower bound
 *    (3) Check if it's dynamic
 *)
let check_dynamic_or_enforce_num env p t r err =
  let et_type = MakeType.num r in
  let (env, err_opt) =
    Common.Result.fold
      ~ok:(fun env -> (env, None))
      ~error:(fun env -> (env, Some (t, et_type)))
    @@ Typing_coercion.coerce_type_res
         p
         Reason.URnone
         env
         t
         { et_type; et_enforced = Enforced }
         err
  in
  let widen_for_arithmetic env ty =
    match get_node ty with
    | Tprim Tast.Tfloat -> (env, Some ty)
    | _ -> (env, None)
  in
  let default = MakeType.num (Reason.Rarith p) in
  let (env, _) =
    Typing_solver.expand_type_and_narrow
      env
      ~default
      ~description_of_expected:"a number"
      widen_for_arithmetic
      p
      t
  in
  (env, Typing_utils.is_dynamic env t, err_opt)

(* Checking of numeric operands for arithmetic operators that work only
 * on integers:
 *   (1) Enforce that it coerce to int
 *   (2) Check if it's dynamic
 *)
let check_dynamic_or_enforce_int env p t r err =
  let et_type = MakeType.int r in
  let (env, err_opt) =
    Common.Result.fold
      ~ok:(fun env -> (env, None))
      ~error:(fun env -> (env, Some (t, et_type)))
    @@ Typing_coercion.coerce_type_res
         p
         Reason.URnone
         env
         t
         { et_type; et_enforced = Enforced }
         err
  in
  (env, Typing_utils.is_dynamic env t, err_opt)

(* Secondary check of numeric operand for arithmetic operators. We've
 * already enforced num and tried to infer a float. Now let's
 * solve to int if it's a type variable with int as lower bound, or
 * solve to int if it's a type variable with no lower bounds.
 *)
let expand_type_and_narrow_to_int env p ty =
  (* Now narrow for type variables *)
  let widen_for_arithmetic env ty =
    match get_node ty with
    | Tprim Tast.Tint -> (env, Some ty)
    | _ -> (env, None)
  in
  let default = MakeType.int (Reason.Rarith p) in
  Typing_solver.expand_type_and_narrow
    env
    ~default
    ~description_of_expected:"a number"
    widen_for_arithmetic
    p
    ty

let hole_on_err (((pos, _), _) as expr) err_opt =
  Option.value_map err_opt ~default:expr ~f:(fun (ty_have, ty_expect) ->
      Tast.make_typed_expr pos ty_have
      @@ Aast.(Hole (expr, ty_have, ty_expect, Typing)))

let binop p env bop p1 te1 ty1 p2 te2 ty2 =
  let make_result env te1 err_opt1 te2 err_opt2 ty =
    let hte1 = hole_on_err te1 err_opt1 and hte2 = hole_on_err te2 err_opt2 in
    (env, Tast.make_typed_expr p ty (Aast.Binop (bop, hte1, hte2)), ty)
  in

  let is_any = Typing_utils.is_any env in
  let contains_any = is_any ty1 || is_any ty2 in
  match bop with
  (* Type of addition, subtraction and multiplication is essentially
   *    (int,int):int
   *  & (float,num):float
   *  & (num,float):float
   *  & (num,num):num
   *)
  | Ast_defs.Plus
  | Ast_defs.Minus
  | Ast_defs.Star
    when not contains_any ->
    let err =
      if TypecheckerOptions.math_new_code (Env.get_tcopt env) then
        Errors.math_invalid_argument
      else
        Errors.unify_error
    in
    let (env, is_dynamic1, err_opt1) =
      check_dynamic_or_enforce_num env p1 ty1 (Reason.Rarith p) err
    in
    let (env, is_dynamic2, err_opt2) =
      check_dynamic_or_enforce_num env p2 ty2 (Reason.Rarith p) err
    in
    (* TODO: extend this behaviour to other operators. Consider producing dynamic
     * result if *either* operand is dynamic
     *)
    if is_float_or_like_float env ty1 then
      let ty =
        MakeType.float
          (Reason.Rarith_ret_float (p, get_reason ty1, Reason.Afirst))
      in
      let (env, ty) =
        (* If either operand is exactly float. This way defers checking
         * subtyping on the second operand except in the case of ~float + float = float. *)
        if
          (not is_dynamic1)
          || ((not is_dynamic2) && is_float_or_like_float env ty2)
        then
          (env, ty)
        else
          Typing_union.union env (MakeType.dynamic (Reason.Rwitness p)) ty
      in
      make_result env te1 err_opt1 te2 err_opt2 ty
    else if is_float_or_like_float env ty2 then
      let ty =
        MakeType.float
          (Reason.Rarith_ret_float (p, get_reason ty2, Reason.Asecond))
      in
      let (env, ty) =
        (* We know the left operand is not float or ~float, so the result only depends on ty2 *)
        if not is_dynamic2 then
          (env, ty)
        else
          Typing_union.union env (MakeType.dynamic (Reason.Rwitness p)) ty
      in
      make_result env te1 err_opt1 te2 err_opt2 ty
    else
      let num1 = is_super_num env ty1 in
      if
        (* If either operand is num or ~num (and neither is float or ~float), return num *)
        num1 || is_super_num env ty2
      then
        let (r, apos) =
          if num1 then
            (get_reason ty1, Reason.Afirst)
          else
            (get_reason ty2, Reason.Asecond)
        in
        make_result
          env
          te1
          err_opt1
          te2
          err_opt2
          (MakeType.num (Reason.Rarith_ret_num (p, r, apos)))
      (* The only remaining cases return ~int, except for int + int = int and dynamic + dynamic = dynamic *)
      else if is_sub_dynamic env ty1 && is_sub_dynamic env ty2 then
        make_result
          env
          te1
          err_opt1
          te2
          err_opt2
          (MakeType.dynamic (Reason.Rarith_dynamic p))
      else if is_int_or_like_int env ty1 || is_int_or_like_int env ty2 then
        let ty = MakeType.int (Reason.Rarith_ret_int p) in
        let (env, ty) =
          if is_dynamic1 || is_dynamic2 then
            Typing_union.union env (MakeType.dynamic (Reason.Rwitness p)) ty
          else
            (env, ty)
        in
        make_result env te1 err_opt1 te2 err_opt2 ty
      else
        let is_global_inference_mode =
          TypecheckerOptions.global_inference (Env.get_tcopt env)
        in
        (* If ty1 is int, the result has the same type as ty2. *)
        if is_global_inference_mode && is_int env ty1 && is_num env ty2 then
          make_result env te1 err_opt1 te2 err_opt2 ty2
        else if is_global_inference_mode && is_int env ty2 && is_num env ty1
        then
          make_result env te1 err_opt1 te2 err_opt2 ty1
        else
          (* Otherwise try narrowing any type variable, with default int *)
          let (env, ty1) = expand_type_and_narrow_to_int env p ty1 in
          let (env, ty2) = expand_type_and_narrow_to_int env p ty2 in
          if is_int env ty1 && is_int env ty2 then
            make_result
              env
              te1
              err_opt1
              te2
              err_opt2
              (MakeType.int (Reason.Rarith_ret_int p))
          else
            make_result
              env
              te1
              err_opt1
              te2
              err_opt2
              (MakeType.num (Reason.Rarith_ret p))
  (* Type of division and exponentiation is essentially
   *    (float,num):float
   *  & (num,float):float
   *  & (num,num):num
   *)
  | Ast_defs.Slash
  | Ast_defs.Starstar
    when not contains_any ->
    let err =
      if TypecheckerOptions.math_new_code (Env.get_tcopt env) then
        Errors.math_invalid_argument
      else
        Errors.unify_error
    in
    let (env, is_dynamic1, err_opt1) =
      check_dynamic_or_enforce_num env p1 ty1 (Reason.Rarith p) err
    in
    let (env, is_dynamic2, err_opt2) =
      check_dynamic_or_enforce_num env p2 ty2 (Reason.Rarith p) err
    in
    let (env, result_ty) =
      if is_float_or_like_float env ty1 then
        let ty =
          MakeType.float
            (Reason.Rarith_ret_float (p, get_reason ty1, Reason.Afirst))
        in
        if
          (not is_dynamic1)
          || ((not is_dynamic2) && is_float_or_like_float env ty2)
        then
          (env, ty)
        else
          Typing_union.union env (MakeType.dynamic Reason.Rnone) ty
      else if is_float_or_like_float env ty2 then
        let ty =
          MakeType.float
            (Reason.Rarith_ret_float (p, get_reason ty2, Reason.Asecond))
        in
        if not is_dynamic2 then
          (env, ty)
        else
          Typing_union.union env (MakeType.dynamic Reason.Rnone) ty
      else if is_sub_dynamic env ty1 && is_sub_dynamic env ty2 then
        (env, MakeType.dynamic (Reason.Rarith_dynamic p))
      else
        match bop with
        | Ast_defs.Slash -> (env, MakeType.num (Reason.Rret_div p))
        | _ -> (env, MakeType.num (Reason.Rarith_ret p))
    in
    make_result env te1 err_opt1 te2 err_opt2 result_ty
  | Ast_defs.Percent
  | Ast_defs.Ltlt
  | Ast_defs.Gtgt
    when not contains_any ->
    let err =
      match bop with
      | Ast_defs.Percent ->
        if TypecheckerOptions.math_new_code (Env.get_tcopt env) then
          Errors.math_invalid_argument
        else
          Errors.unify_error
      | _ -> Errors.bitwise_math_invalid_argument
    in
    let (env, _, err_opt1) =
      check_dynamic_or_enforce_int env p1 ty1 (Reason.Rarith p) err
    in
    let (env, _, err_opt2) =
      check_dynamic_or_enforce_int env p2 ty2 (Reason.Rarith p) err
    in
    let r =
      match bop with
      | Ast_defs.Percent -> Reason.Rarith_ret_int p
      | _ -> Reason.Rbitwise_ret p
    in
    make_result env te1 err_opt1 te2 err_opt2 (MakeType.int r)
  | Ast_defs.Xor
  | Ast_defs.Amp
  | Ast_defs.Bar
    when not contains_any ->
    let (env, is_dynamic1, err_opt1) =
      check_dynamic_or_enforce_int
        env
        p
        ty1
        (Reason.Rbitwise p1)
        Errors.bitwise_math_invalid_argument
    in
    let (env, is_dynamic2, err_opt2) =
      check_dynamic_or_enforce_int
        env
        p
        ty2
        (Reason.Rbitwise p2)
        Errors.bitwise_math_invalid_argument
    in
    let result_ty =
      if is_dynamic1 && is_dynamic2 then
        MakeType.dynamic (Reason.Rbitwise_dynamic p)
      else
        MakeType.int (Reason.Rbitwise_ret p)
    in
    make_result env te1 err_opt1 te2 err_opt2 result_ty
  | Ast_defs.Eqeq
  | Ast_defs.Diff ->
    begin
      let rec strict_allowable_types ty =
        let open Aast in
        match get_node ty with
        | Tdynamic -> true
        | Tprim prim ->
          (match prim with
          | Tint
          | Tbool
          | Tfloat
          | Tstring ->
            true
          | _ -> false)
        | Tclass ((_, name), _, type_args)
        (* allow vec, keyset, and dict *)
          when String.equal name SN.Collections.cVec
               || String.equal name SN.Collections.cKeyset
               || String.equal name SN.Collections.cDict ->
          List.for_all type_args strict_allowable_types
        | _ -> false
      in
      let same_type env ty1 ty2 =
        Typing_subtype.is_sub_type env ty1 ty2
        && Typing_subtype.is_sub_type env ty2 ty1
        || is_sub_dynamic env ty1
        || is_sub_dynamic env ty2
      in
      (* Allow value-equating types A, B such that:
      - [(A <: B and B <: A) or (A and/or B is dynamic)]
      - AND (A and B are dynamic/primitive)
      We also allow the basic value container types `vec`, `keyset`, and `dict` to be compared. *)
      let rec strict_equatable env ty1 ty2 =
        strict_allowable_types ty1
        && strict_allowable_types ty2
        && ( same_type env ty1 ty2
           ||
           (* Cover the case where the containers have a dynamic parameter,
           since `same_type` would evaluate to false.
           E.g. `vec<dynamic> == vec<int>` should be allowed. *)
           match (get_node ty1, get_node ty2) with
           | ( Tclass ((_, name1), _, container_tys1),
               Tclass ((_, name2), _, container_tys2) )
             when String.equal name1 name2 ->
             (match
                List.for_all2
                  container_tys1
                  container_tys2
                  (strict_equatable env)
              with
             | List.Or_unequal_lengths.Ok all_true -> all_true
             | _ -> false)
           | _ -> false )
      in
      if
        TypecheckerOptions.strict_value_equality (Env.get_tcopt env)
        && not (strict_equatable env ty1 ty2)
      then
        let tys1 = Typing_print.error env ty1 in
        let tys2 = Typing_print.error env ty2 in
        Errors.strict_eq_value_incompatible_types
          p
          (Reason.to_string ("This is " ^ tys1) (get_reason ty1))
          (Reason.to_string ("This is " ^ tys2) (get_reason ty2))
    end;
    make_result env te1 None te2 None (MakeType.bool (Reason.Rcomp p))
  | Ast_defs.Eqeqeq
  | Ast_defs.Diff2 ->
    make_result env te1 None te2 None (MakeType.bool (Reason.Rcomp p))
  | Ast_defs.Lt
  | Ast_defs.Lte
  | Ast_defs.Gt
  | Ast_defs.Gte
  | Ast_defs.Cmp ->
    let ty_num = MakeType.num (Reason.Rcomp p) in
    let ty_int = MakeType.int (Reason.Rcomp p) in
    let ty_bool = MakeType.bool (Reason.Rcomp p) in
    let ty_result =
      match bop with
      | Ast_defs.Cmp -> ty_int
      | _ -> ty_bool
    in
    let ty_string = MakeType.string (Reason.Rcomp p) in
    let ty_datetime = MakeType.datetime (Reason.Rcomp p) in
    let ty_datetimeimmutable = MakeType.datetime_immutable (Reason.Rcomp p) in
    let ty_dynamic = MakeType.dynamic (Reason.Rcomp p) in
    let both_sub ty =
      Typing_subtype.can_sub_type env ty1 ty
      && Typing_subtype.can_sub_type env ty2 ty
    in
    (*
     * Comparison here is allowed when both args are num, both string, or both
     * DateTime | DateTimeImmutable. Alternatively, either or both args can be
     * dynamic. We use both_sub to check that both arguments subtype a type.
     *
     * This actually does not properly handle union types. For instance,
     * DateTime | DateTimeImmutable is neither a subtype of DateTime nor
     * DateTimeImmutable, but it will be the type of an element coming out
     * of a vector containing both. Further, dynamic could be comparable to
     * num | string | DateTime | DateTimeImmutable | dynamic. Better union
     * handling would be an improvement.
     *)
    ( if
      (not contains_any)
      && not
           ( both_sub
               (Typing_make_type.union (Reason.Rcomp p) [ty_num; ty_dynamic])
           || both_sub
                (Typing_make_type.union
                   (Reason.Rcomp p)
                   [ty_string; ty_dynamic])
           || both_sub
                (Typing_make_type.union
                   (Reason.Rcomp p)
                   [ty_datetime; ty_datetimeimmutable; ty_dynamic]) )
    then
      let ty1 = Typing_expand.fully_expand env ty1 in
      let ty2 = Typing_expand.fully_expand env ty2 in
      let tys1 = Typing_print.error env ty1 in
      let tys2 = Typing_print.error env ty2 in
      Errors.comparison_invalid_types
        p
        (Reason.to_string ("This is " ^ tys1) (get_reason ty1))
        (Reason.to_string ("This is " ^ tys2) (get_reason ty2)) );
    make_result env te1 None te2 None ty_result
  | Ast_defs.Dot ->
    (* A bit weird, this one:
     *   function(Stringish | string, Stringish | string) : string)
     *)
    if TypecheckerOptions.enable_strict_string_concat_interp (Env.get_tcopt env)
    then
      let sub_arraykey env p ty =
        let r = Reason.Rconcat_operand p in
        let (env, formatter_tyvar) = Env.fresh_type_invariant env p in
        let stringlike =
          MakeType.union
            r
            [
              MakeType.arraykey r;
              MakeType.dynamic r;
              MakeType.new_type r SN.Classes.cHHFormatString [formatter_tyvar];
            ]
        in
        Result.fold
          ~ok:(fun env -> (env, None))
          ~error:(fun env -> (env, Some (ty, stringlike)))
        @@ Typing_ops.sub_type_res
             p
             Reason.URstr_concat
             env
             ty
             stringlike
             Errors.strict_str_concat_type_mismatch
      in
      let (env, err_opt1) = sub_arraykey env p1 ty1 in
      let (env, err_opt2) = sub_arraykey env p2 ty2 in
      let ty = MakeType.string (Reason.Rconcat_ret p) in
      make_result env te1 err_opt1 te2 err_opt2 ty
    else
      let (env, err_opt1) = Typing_substring.sub_string_err p1 env ty1 in
      let (env, err_opt2) = Typing_substring.sub_string_err p2 env ty2 in
      let ty = MakeType.string (Reason.Rconcat_ret p) in
      make_result env te1 err_opt1 te2 err_opt2 ty
  | Ast_defs.Barbar
  | Ast_defs.Ampamp ->
    make_result env te1 None te2 None (MakeType.bool (Reason.Rlogic_ret p))
  | Ast_defs.QuestionQuestion
  | Ast_defs.Eq _
    when not contains_any ->
    assert false
  | _ ->
    assert contains_any;
    if is_any ty1 then
      make_result env te1 None te2 None ty1
    else
      make_result env te1 None te2 None ty2

let unop p env uop te ty =
  let make_result env te err_opt result_ty =
    let hte = hole_on_err te err_opt in
    (env, Tast.make_typed_expr p result_ty (Aast.Unop (uop, hte)), result_ty)
  in
  let is_any = Typing_utils.is_any env in
  match uop with
  | Ast_defs.Unot ->
    if is_any ty then
      make_result env te None ty
    else
      (* args isn't any or a variant thereof so can actually do stuff *)
      (* !$x (logical not) works with any type, so we just return Tbool *)
      make_result env te None (MakeType.bool (Reason.Rlogic_ret p))
  | Ast_defs.Utild ->
    if is_any ty then
      make_result env te None ty
    else
      (* args isn't any or a variant thereof so can actually do stuff *)
      let (env, is_dynamic, err_opt) =
        check_dynamic_or_enforce_int
          env
          p
          ty
          (Reason.Rbitwise p)
          Errors.bitwise_math_invalid_argument
      in
      let result_ty =
        if is_dynamic then
          MakeType.dynamic (Reason.Rbitwise_dynamic p)
        else
          MakeType.int (Reason.Rbitwise_ret p)
      in
      make_result env te err_opt result_ty
  | Ast_defs.Uincr
  | Ast_defs.Upincr
  | Ast_defs.Updecr
  | Ast_defs.Udecr ->
    (* increment and decrement operators modify the value,
     * check for immutability violation here *)
    if is_any ty then
      make_result env te None ty
    else
      (* args isn't any or a variant thereof so can actually do stuff *)
      let (env, is_dynamic, err_opt) =
        check_dynamic_or_enforce_num
          env
          p
          ty
          (Reason.Rarith p)
          Errors.inc_dec_invalid_argument
      in
      let result_ty =
        if is_dynamic then
          MakeType.dynamic (Reason.Rincdec_dynamic p)
        else if is_float env ty then
          MakeType.float
            (Reason.Rarith_ret_float (p, get_reason ty, Reason.Aonly))
        else if is_int env ty then
          MakeType.int (Reason.Rarith_ret_int p)
        else
          MakeType.num (Reason.Rarith_ret_num (p, get_reason ty, Reason.Aonly))
      in
      make_result env te err_opt result_ty
  | Ast_defs.Uplus
  | Ast_defs.Uminus ->
    if is_any ty then
      make_result env te None ty
    else
      (* args isn't any or a variant thereof so can actually do stuff *)
      let (env, _, err_opt) =
        check_dynamic_or_enforce_num
          env
          p
          ty
          (Reason.Rarith p)
          Errors.unify_error
      in
      let (env, ty) = expand_type_and_narrow_to_int env p ty in
      let result_ty =
        if is_float env ty then
          MakeType.float
            (Reason.Rarith_ret_float (p, get_reason ty, Reason.Aonly))
        else if is_int env ty then
          MakeType.int (Reason.Rarith_ret_int p)
        else
          MakeType.num (Reason.Rarith_ret_num (p, get_reason ty, Reason.Aonly))
      in
      make_result env te err_opt result_ty
  | Ast_defs.Usilence ->
    (* Silencing does not change the type *)
    (* any check omitted because would return the same anyway *)
    make_result env te None ty
