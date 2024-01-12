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
  let (r, e) =
    Typing_solver.is_sub_type env t (MakeType.dynamic Reason.Rnone)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) e;
  r

let is_float env t =
  let (r, e) = Typing_solver.is_sub_type env t (MakeType.float Reason.Rnone) in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) e;
  r

let is_int env t =
  let (r, e) = Typing_solver.is_sub_type env t (MakeType.int Reason.Rnone) in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) e;
  r

(* Checking of numeric operands for arithmetic operators that work only
 * on integers:
 *   (1) Enforce that it coerce to int
 *   (2) Check if it's dynamic
 *)
let check_dynamic_or_enforce_int env p t r err =
  let ty = MakeType.int r in
  let (env, ty_err_opt) =
    Typing_coercion.coerce_type
      ~coerce_for_op:true
      p
      Reason.URnone
      env
      t
      ty
      Enforced
      err
  in
  Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
  let ty_mismatch = Option.map ty_err_opt ~f:Fn.(const (t, ty)) in
  (env, Typing_utils.is_dynamic env t, ty_mismatch)

(** [check_like_num p_exp p env ty] ensures that [ty] is a subtype of num or ~num.
  In the former case, it returns false and the type, in the latter, it returns
  true, and the type with the ~ removed. If it is not a subtype of either, the
  error generated refers to the attempt to make [ty] <: num. If there were errors
  in both cases, we return the errors from the non ~ case. *)
let check_like_num ?err p_exp p env ty =
  let err =
    match err with
    | None -> Typing_error.Callback.math_invalid_argument
    | Some err -> err
  in
  let num_ty = MakeType.num (Reason.Rarith p_exp) in
  Typing_coercion.coerce_type_like_strip p Reason.URnone env ty num_ty err

(** [expand_type_and_narrow_to_numeric ~allow_nothing env p ty] forces the
  solving of [ty] to a numeric type, based on its lower bounds. If allow_nothing
  is set, then it is allowed to return nothing when [ty] doesn't have numeric.
  Otherwise the default type of int will be solved to. *)
let expand_type_and_narrow_to_numeric ~allow_nothing env p ty =
  let widen_for_arithmetic env ty =
    match get_node ty with
    | Tprim Tast.Tnum
    | Tprim Tast.Tfloat
    | Tprim Tast.Tint ->
      ((env, None), Some ty)
    | _ -> ((env, None), None)
  in
  let default = MakeType.int (Reason.Rarith p) in
  Typing_solver.expand_type_and_narrow
    env
    ~default
    ~force_solve:true
    ~allow_nothing
    ~description_of_expected:"a number"
    widen_for_arithmetic
    p
    ty

(** [expand_type_and_try_narrow_to_float env p ty] tries to solve [ty] to
  float, based on its lower bounds. If it isn't solved to float, no narrowing
  is done. *)
let expand_type_and_try_narrow_to_float env p ty =
  let widen_for_arithmetic env ty =
    match get_node ty with
    | Tprim Tast.Tfloat -> ((env, None), Some ty)
    | Tgeneric _ ->
      let float = MakeType.float (get_reason ty) in
      if Typing_subtype.is_sub_type env ty float then
        ((env, None), Some float)
      else
        ((env, None), None)
    | _ -> ((env, None), None)
  in
  Typing_solver.expand_type_and_narrow
    env
    ~force_solve:false
    ~allow_nothing:false
    ~description_of_expected:"a number"
    widen_for_arithmetic
    p
    ty

(** [expand_type_and_try_narrow_to_nothing env p ty] tries to solve [ty] to
  nothing, based on its lower bounds. If it isn't solved to nothing, no narrowing
  is done. *)
let expand_type_and_try_narrow_to_nothing env p ty =
  let widen_for_arithmetic env _ty = ((env, None), None) in
  Typing_solver.expand_type_and_narrow
    env
    ~force_solve:false
    ~allow_nothing:true
    ~description_of_expected:"a number"
    widen_for_arithmetic
    p
    ty

let hole_on_err ((_, pos, _) as expr) err_opt =
  Option.value_map err_opt ~default:expr ~f:(fun (ty_have, ty_expect) ->
      Tast.make_typed_expr pos ty_have
      @@ Aast.(Hole (expr, ty_have, ty_expect, Typing)))

(** [check_for_float_result p env p1 ty1 p2 ty2] Tries to narrow ty1 or ty2 to float,
   and if one can narrow, then the result is float. However, we don't want to do early
   solving otherwise, before we've detected the other cases that don't require full solving. *)
let check_for_float_result p env p1 ty1 p2 ty2 =
  let make_float_type ~use_ty1_reason =
    Some
      (MakeType.float
         (if use_ty1_reason then
           Reason.Rarith_ret_float (p, get_reason ty1, Reason.Afirst)
         else
           Reason.Rarith_ret_float (p, get_reason ty2, Reason.Asecond)))
  in
  let float_no_reason = MakeType.float Reason.none in
  let ((env, ty_err_opt), ty1) =
    expand_type_and_try_narrow_to_float env p1 ty1
  in
  Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
  if Typing_subtype.is_sub_type env ty1 float_no_reason then
    (env, make_float_type ~use_ty1_reason:true)
  else
    let ((env, ty_err_opt), ty2) =
      expand_type_and_try_narrow_to_float env p2 ty2
    in
    Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
    if Typing_subtype.is_sub_type env ty2 float_no_reason then
      (env, make_float_type ~use_ty1_reason:false)
    else
      (env, None)

let binop p env bop p1 te1 ty1 p2 te2 ty2 =
  let make_result ?(is_like = false) env te1 err_opt1 te2 err_opt2 ty =
    let (env, ty) =
      if is_like then
        Typing_union.union env ty (MakeType.dynamic (Reason.Rwitness p))
      else
        (env, ty)
    in
    let hte1 = hole_on_err te1 err_opt1 and hte2 = hole_on_err te2 err_opt2 in
    let (env, te) =
      Typing_utils.make_simplify_typed_expr
        env
        p
        ty
        Aast.(Binop { bop; lhs = hte1; rhs = hte2 })
    in
    (env, te, ty)
  in
  let int_no_reason = MakeType.int Reason.none in
  let num_no_reason = MakeType.num Reason.none in
  let nothing_no_reason = MakeType.nothing Reason.none in
  let make_nothing_type ~use_ty1_reason =
    MakeType.nothing
      (if use_ty1_reason then
        get_reason ty1
      else
        get_reason ty2)
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
    let make_int_type () = MakeType.int (Reason.Rarith_ret_int p) in
    let make_num_type ~use_ty1_reason =
      let (r, apos) =
        if use_ty1_reason then
          (get_reason ty1, Reason.Afirst)
        else
          (get_reason ty2, Reason.Asecond)
      in
      MakeType.num (Reason.Rarith_ret_num (p, r, apos))
    in
    let (env, ty1) = Env.expand_type env ty1 in
    let (env, ty2) = Env.expand_type env ty2 in
    let is_dynamic1 = Typing_defs.is_dynamic ty1 in
    let is_dynamic2 = Typing_defs.is_dynamic ty2 in
    let (env, ty_mismatch1, is_like1, ty1) = check_like_num p p1 env ty1 in
    let (env, ty_mismatch2, is_like2, ty2) = check_like_num p p2 env ty2 in
    (* We'll compute the type ignoring whether one of the arguments was a like-type,
       but we'll add the like type to the result type in the end if one was. *)
    let is_like = is_like1 || is_like2 in
    let (env, result_ty) =
      match check_for_float_result p env p1 ty1 p2 ty2 with
      | (env, Some ty) -> (env, ty)
      | (env, None) ->
        let is_int1 = Typing_subtype.is_sub_type env ty1 int_no_reason in
        let is_int2 = Typing_subtype.is_sub_type env ty2 int_no_reason in
        (* If both of the arguments is definitely subtypes of int,
           then the return is int *)
        if is_int1 && is_int2 then
          (env, make_int_type ())
        else
          let is_num1 = Typing_subtype.is_sub_type env num_no_reason ty1 in
          let is_num2 = Typing_subtype.is_sub_type env num_no_reason ty2 in
          (* If one argument is exactly num, then the return is num. *)
          if is_num1 || is_num2 then
            (env, make_num_type ~use_ty1_reason:is_num1)
          else
            (* Otherwise, at least one of the arguments is unsolved, and we need
               to take care to not solve too eagerly.
               Put the first one to try in ty1,
               and the second, if any in ty2. Keep the result of the previous
               check whether the second type was an int. *)
            let ( (ty1, p1, is_dynamic1),
                  (ty2, p2, is_int2, is_dynamic2),
                  swapped ) =
              if is_int1 then
                ((ty2, p2, is_dynamic2), (ty1, p1, is_int1, is_dynamic1), true)
              else
                ((ty1, p1, is_dynamic1), (ty2, p2, is_int2, is_dynamic2), false)
            in
            let ((env, ty_err_opt), ty1) =
              expand_type_and_narrow_to_numeric
                ~allow_nothing:is_dynamic1
                env
                p1
                ty1
            in
            Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
            (* If the first type solved a subtype of int, and the second was
               already int, then it is sound to return int. *)
            let is_int1 = Typing_subtype.is_sub_type env ty1 int_no_reason in
            if is_int1 && is_int2 then
              (env, make_int_type ())
            else
              (* If the first type solved to exactly num, then we can just return that *)
              let is_num1 = Typing_subtype.is_sub_type env num_no_reason ty1 in
              if is_num1 then
                (env, make_num_type ~use_ty1_reason:swapped)
              else
                (* The second type needs solving too *)
                let ((env, ty_err_opt), ty2) =
                  expand_type_and_narrow_to_numeric
                    ~allow_nothing:is_dynamic2
                    env
                    p2
                    ty2
                in
                Option.iter
                  ty_err_opt
                  ~f:(Typing_error_utils.add_typing_error ~env);
                if
                  Typing_subtype.is_sub_type env ty1 nothing_no_reason
                  && Typing_subtype.is_sub_type env ty2 nothing_no_reason
                then
                  (env, make_nothing_type ~use_ty1_reason:swapped)
                else if
                  is_int1 && Typing_subtype.is_sub_type env ty2 int_no_reason
                then
                  (env, make_int_type ())
                else
                  (env, make_num_type ~use_ty1_reason:(not swapped))
    in
    make_result ~is_like env te1 ty_mismatch1 te2 ty_mismatch2 result_ty
  (* Type of division and exponentiation is essentially
   *    (float,num):float
   *  & (num,float):float
   *  & (num,num):num
   *)
  | Ast_defs.Slash
  | Ast_defs.Starstar
    when not contains_any ->
    let make_num_type () =
      match bop with
      | Ast_defs.Slash -> MakeType.num (Reason.Rret_div p)
      | _ -> MakeType.num (Reason.Rarith_ret p)
    in
    let (env, ty_mismatch1, is_like1, ty1) = check_like_num p p1 env ty1 in
    let (env, ty_mismatch2, is_like2, ty2) = check_like_num p p2 env ty2 in
    (* We'll compute the type ignoring whether one of the arguments was a like-type,
       but we'll add the like type to the result type in the end if one was. *)
    let is_like = is_like1 || is_like2 in
    let (env, result_ty) =
      match check_for_float_result p env p1 ty1 p2 ty2 with
      | (env, Some ty) -> (env, ty)
      | (env, None) ->
        (* We return num, unless both ty1 and ty2 will solve to nothing. This
           would happen for dynamic / dynamic, and in that case we want to
           return nothing, in order to get back dynamic in the end. For that to happen,
           we would have to have detected like types on both arguments. *)
        if not (is_like1 && is_like2) then
          (env, make_num_type ())
        else
          let ((env, ty_err_opt), ty1) =
            expand_type_and_try_narrow_to_nothing env p1 ty1
          in
          Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
          let ((env, ty_err_opt), ty2) =
            expand_type_and_try_narrow_to_nothing env p2 ty2
          in
          Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
          if
            Typing_subtype.is_sub_type env ty1 nothing_no_reason
            && Typing_subtype.is_sub_type env ty2 nothing_no_reason
          then
            (env, make_nothing_type ~use_ty1_reason:true)
          else
            (env, make_num_type ())
    in
    make_result ~is_like env te1 ty_mismatch1 te2 ty_mismatch2 result_ty
  | Ast_defs.Percent
  | Ast_defs.Ltlt
  | Ast_defs.Gtgt
    when not contains_any ->
    let err =
      match bop with
      | Ast_defs.Percent -> Typing_error.Callback.math_invalid_argument
      | _ -> Typing_error.Callback.bitwise_math_invalid_argument
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
        Typing_error.Callback.bitwise_math_invalid_argument
    in
    let (env, is_dynamic2, err_opt2) =
      check_dynamic_or_enforce_int
        env
        p
        ty2
        (Reason.Rbitwise p2)
        Typing_error.Callback.bitwise_math_invalid_argument
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
          List.for_all type_args ~f:strict_allowable_types
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
        && (same_type env ty1 ty2
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
                  ~f:(strict_equatable env)
              with
             | List.Or_unequal_lengths.Ok all_true -> all_true
             | _ -> false)
           | _ -> false)
      in
      if
        TypecheckerOptions.strict_value_equality (Env.get_tcopt env)
        && not (strict_equatable env ty1 ty2)
      then
        let tys1 = lazy (Typing_print.error env ty1)
        and tys2 = lazy (Typing_print.error env ty2) in
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Strict_eq_value_incompatible_types
                 {
                   pos = p;
                   left =
                     Lazy.map tys1 ~f:(fun tys1 ->
                         Reason.to_string ("This is " ^ tys1) (get_reason ty1));
                   right =
                     Lazy.map tys2 ~f:(fun tys2 ->
                         Reason.to_string ("This is " ^ tys2) (get_reason ty2));
                 })
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
    (if
     (not contains_any)
     && not
          (both_sub (MakeType.union (Reason.Rcomp p) [ty_num; ty_dynamic])
          || both_sub (MakeType.union (Reason.Rcomp p) [ty_string; ty_dynamic])
          || both_sub
               (MakeType.union
                  (Reason.Rcomp p)
                  [ty_datetime; ty_datetimeimmutable; ty_dynamic]))
    then
      let ty1 = lazy (Typing_expand.fully_expand env ty1)
      and ty2 = lazy (Typing_expand.fully_expand env ty2) in
      let tys1 = Lazy.map ty1 ~f:(fun ty -> (ty, Typing_print.error env ty))
      and tys2 = Lazy.map ty2 ~f:(fun ty -> (ty, Typing_print.error env ty)) in
      Typing_error_utils.add_typing_error
        ~env
        Typing_error.(
          primary
          @@ Primary.Comparison_invalid_types
               {
                 pos = p;
                 left =
                   Lazy.map tys1 ~f:(fun (ty1, tys1) ->
                       Reason.to_string ("This is " ^ tys1) (get_reason ty1));
                 right =
                   Lazy.map tys2 ~f:(fun (ty2, tys2) ->
                       Reason.to_string ("This is " ^ tys2) (get_reason ty2));
               }));
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
              MakeType.hh_formatstring r formatter_tyvar;
            ]
        in
        let (env, ty_err_opt) =
          Typing_ops.sub_type
            p
            Reason.URstr_concat
            env
            ty
            stringlike
            Typing_error.Callback.strict_str_concat_type_mismatch
        in
        let ty_mismatch =
          Option.map ty_err_opt ~f:(Fn.const (ty, stringlike))
        in
        Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
        (env, ty_mismatch)
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
    let (env, te) =
      Typing_utils.make_simplify_typed_expr
        env
        p
        result_ty
        (Aast.Unop (uop, hte))
    in
    (env, te, result_ty)
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
          Typing_error.Callback.bitwise_math_invalid_argument
      in
      let result_ty =
        if is_dynamic then
          MakeType.dynamic (Reason.Rbitwise_dynamic p)
        else
          MakeType.int (Reason.Rbitwise_ret p)
      in
      make_result env te err_opt result_ty
  | Ast_defs.Uplus
  | Ast_defs.Uminus
  | Ast_defs.Uincr
  | Ast_defs.Upincr
  | Ast_defs.Udecr
  | Ast_defs.Updecr ->
    if is_any ty then
      make_result env te None ty
    else
      let allow_nothing = Typing_defs.is_dynamic ty in
      let (env, ty_mismatch, is_like, ty) =
        check_like_num
          ?err:
            (match uop with
            | Ast_defs.Uincr
            | Ast_defs.Upincr
            | Ast_defs.Udecr
            | Ast_defs.Updecr ->
              Some Typing_error.Callback.inc_dec_invalid_argument
            | _ -> None)
          p
          p
          env
          ty
      in
      let ((env, ty_err_opt), (ty : locl_ty)) =
        expand_type_and_narrow_to_numeric ~allow_nothing env p ty
      in
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
      let result_ty =
        if Typing_subtype.is_sub_type env ty (MakeType.nothing Reason.none) then
          MakeType.nothing (get_reason ty)
        else if is_float env ty then
          MakeType.float
            (Reason.Rarith_ret_float (p, get_reason ty, Reason.Aonly))
        else if is_int env ty then
          MakeType.int (Reason.Rarith_ret_int p)
        else
          MakeType.num (Reason.Rarith_ret_num (p, get_reason ty, Reason.Aonly))
      in
      let (env, result_ty) =
        if is_like then
          Typing_union.union
            env
            result_ty
            (MakeType.dynamic (get_reason result_ty))
        else
          (env, result_ty)
      in
      make_result env te ty_mismatch result_ty
  | Ast_defs.Usilence ->
    (* Silencing does not change the type *)
    (* any check omitted because would return the same anyway *)
    make_result env te None ty
