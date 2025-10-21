(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type dyn_func_kind =
  | Supportdyn_function
  | Like_function

let mk_ty_mismatch_opt ty_have ty_expect = function
  | Some _ -> Some (ty_have, ty_expect)
  | _ -> None

let check_argument_type_against_parameter_type_helper
    ~dynamic_func ~ignore_readonly ~ureason env pos param_ty arg_ty =
  Typing_log.(
    log_with_level env "typing" ~level:2 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos pos)
          env
          [
            Log_head
              ( ("Typing.check_argument_type_against_parameter_type_helper "
                ^
                match dynamic_func with
                | None -> "None"
                | Some Supportdyn_function -> "sd"
                | Some Like_function -> "~"),
                [Log_type ("param_ty", param_ty); Log_type ("arg_ty", arg_ty)]
              );
          ]));
  let param_ty =
    match dynamic_func with
    | Some dyn_func_kind -> begin
      (* It is only sound to add like types to the parameters of supportdyn functions, since
         in this case we are semantically just using the &dynamic part of the type to call them.
         For like functions, they have to check both sides. *)
      match dyn_func_kind with
      | Supportdyn_function ->
        Typing_make_type.locl_like (Typing_defs.get_reason param_ty) param_ty
      | Like_function -> param_ty
    end
    | None -> param_ty
  in
  Typing_coercion.coerce_type
    ~is_dynamic_aware:false
    ~ignore_readonly
    pos
    ureason
    env
    arg_ty
    param_ty
    Enforced
    Typing_error.Callback.unify_error

(*
 * Check a single argument type arg_ty against the function parameter type param_ty.
 *
 * For functions marked SupportDynamicType (if dynamic_func != None)
 * we attempt a static check, and if that fails, we check against a like-type of param_ty.
 *
 * In the case that the argument type is already a like-type,
 * and the parameter type is not, we just skip straight to this dynamic check, in order
 * to avoid the situation where we make poor choices with generic parameters e.g. we
 * attempt ~t <: T to get T:=~t rather than ~t <: ~T to get T := t.
 *)
let check_argument_type_against_parameter_type
    ?(is_single_argument = false)
    ?(ureason = Typing_reason.URparam)
    ~dynamic_func
    ~ignore_readonly
    env
    param_ty
    arg_pos
    arg_ty =
  let (env, opt_e, used_dynamic) =
    if Option.is_none dynamic_func then
      let (env, opt_e) =
        check_argument_type_against_parameter_type_helper
          ~dynamic_func:None
          ~ignore_readonly
          ~ureason
          env
          arg_pos
          param_ty
          arg_ty
      in
      (env, opt_e, false)
    else
      let check_dynamic_only =
        (not is_single_argument)
        && (not (Typing_utils.is_dynamic env param_ty))
        && Option.is_some
             (snd
                (Typing_dynamic_utils.try_strip_dynamic
                   ~accept_intersections:true
                   env
                   arg_ty))
        && Option.is_none
             (snd (Typing_dynamic_utils.try_strip_dynamic env param_ty))
      in
      if check_dynamic_only then
        (* Only try dynamically *)
        let (env, opt_e) =
          check_argument_type_against_parameter_type_helper
            ~dynamic_func
            ~ignore_readonly
            ~ureason
            env
            arg_pos
            param_ty
            arg_ty
        in
        (env, opt_e, true)
      else
        let (env, opt_e, used_dynamic_info) =
          (* First try statically *)
          let (env1, e1opt) =
            check_argument_type_against_parameter_type_helper
              ~dynamic_func:None
              ~ignore_readonly
              ~ureason
              env
              arg_pos
              param_ty
              arg_ty
          in
          match e1opt with
          | None -> (env1, None, false)
          | Some e1 ->
            let (env2, e2opt) =
              check_argument_type_against_parameter_type_helper
                ~dynamic_func
                ~ignore_readonly
                ~ureason
                env
                arg_pos
                param_ty
                arg_ty
            in
            (match e2opt with
            (* We used dynamic calling to get a successful check *)
            | None -> (env2, None, true)
            (* We failed on both, pick the one with fewest errors! (preferring static on a tie) *)
            | Some e2 ->
              if Typing_error.count e1 <= Typing_error.count e2 then
                (env1, Some e1, false)
              else
                (env2, Some e2, true))
        in
        (env, opt_e, used_dynamic_info)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) opt_e;
  (env, mk_ty_mismatch_opt arg_ty param_ty opt_e, used_dynamic)
