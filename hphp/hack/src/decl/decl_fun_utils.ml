(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module SN = Naming_special_names

let has_accept_disposable_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaAcceptDisposable user_attributes

let has_ignore_readonly_error_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaIgnoreReadonlyError user_attributes

let has_return_disposable_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaReturnDisposable user_attributes

let has_memoize_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaMemoize user_attributes
  || Naming_attributes.mem SN.UserAttributes.uaMemoizeLSB user_attributes

let hint_to_type_opt env hint = Option.map hint ~f:(Decl_hint.hint env)

let hint_to_type ~default env hint =
  Option.value (hint_to_type_opt env hint) ~default

let make_wildcard_ty pos =
  mk (Reason.witness_from_decl pos, Typing_defs.Twildcard)

let make_param_ty env param =
  let param_pos = Decl_env.make_decl_pos env param.param_pos in
  let ty =
    hint_to_type
      ~default:(make_wildcard_ty param_pos)
      env
      (hint_of_type_hint param.param_type_hint)
  in
  let ty =
    if Aast_utils.is_param_variadic param then
      (* When checking a call f($a, $b) to a function f(C ...$args),
       * both $a and $b must be of type C *)
      with_reason ty (Reason.var_param_from_decl param_pos)
    else if Aast_utils.is_param_splat param then
      with_reason ty (Reason.tuple_from_splat param_pos)
    else
      ty
  in
  let mode = get_param_mode param.param_callconv in
  {
    fp_pos = param_pos;
    fp_name =
      (* The parser uses ... for variadic parameters that lack a name *)
      (if String.equal param.param_name "..." then
        None
      else
        Some param.param_name);
    fp_type = ty;
    fp_flags =
      make_fp_flags
        ~mode
        ~accept_disposable:
          (has_accept_disposable_attribute param.param_user_attributes)
        ~is_optional:(Option.is_some (Aast_utils.get_param_default param))
        ~readonly:(Option.is_some param.param_readonly)
        ~ignore_readonly_error:
          (has_ignore_readonly_error_attribute param.param_user_attributes)
        ~splat:(Option.is_some param.param_splat);
    fp_def_value = None;
  }

let ret_from_fun_kind ?(is_constructor = false) env (pos : pos) kind hint =
  let pos = Decl_env.make_decl_pos env pos in
  let ret_ty () =
    if is_constructor then
      mk (Reason.witness_from_decl pos, Tprim Tvoid)
    else
      hint_to_type ~default:(make_wildcard_ty pos) env hint
  in
  match hint with
  | None ->
    (match kind with
    | Ast_defs.FGenerator ->
      let r = Reason.ret_fun_kind_from_decl (pos, kind) in
      mk
        ( r,
          Tapply
            ((pos, SN.Classes.cGenerator), [ret_ty (); ret_ty (); ret_ty ()]) )
    | Ast_defs.FAsyncGenerator ->
      let r = Reason.ret_fun_kind_from_decl (pos, kind) in
      mk
        ( r,
          Tapply
            ( (pos, SN.Classes.cAsyncGenerator),
              [ret_ty (); ret_ty (); ret_ty ()] ) )
    | Ast_defs.FAsync ->
      let r = Reason.ret_fun_kind_from_decl (pos, kind) in
      mk (r, Tapply ((pos, SN.Classes.cAwaitable), [ret_ty ()]))
    | Ast_defs.FSync -> ret_ty ())
  | Some _ -> ret_ty ()

let type_param = Decl_hint.aast_tparam_to_decl_tparam

let where_constraint env (ty1, ck, ty2) =
  (Decl_hint.hint env ty1, ck, Decl_hint.hint env ty2)

(* Functions building the types for the parameters of a function *)
(* It's not completely trivial because of optional arguments  *)

let check_params ~from_abstract_method env paraml =
  (* We wish to give an error on the first non-default parameter
     after a default parameter. That is:
     function foo(int $x, ?int $y = null, int $z)
     is an error on $z. *)
  (* TODO: This check doesn't need to be done at type checking time; it is
     entirely syntactic. When we switch over to the FFP, remove this code. *)
  let rec loop seen_default (acc : Typing_error.t list) paraml =
    match paraml with
    | [] -> acc
    | param :: rl ->
      let is_optional_not_default =
        Aast_utils.is_param_optional param
        && Option.is_none (Aast_utils.get_param_default param)
      in
      let acc =
        if is_optional_not_default then
          if
            not
              (TypecheckerOptions.enable_abstract_method_optional_parameters
                 (Decl_env.tcopt env))
          then
            Typing_error.(
              primary
              @@ Primary.Optional_parameter_not_supported param.param_pos)
            :: acc
          else if not from_abstract_method then
            Typing_error.(
              primary @@ Primary.Optional_parameter_not_abstract param.param_pos)
            :: acc
          else
            acc
        else
          acc
      in
      (* Assume that a variadic parameter is the last one we need
            to check. We've already given a parse error if the variadic
            parameter is not last. *)
      if Aast_utils.is_param_variadic param then
        acc
      else
        let is_optional_or_default =
          Option.is_some (Aast_utils.get_param_default param)
          || is_optional_not_default
        in
        let acc =
          if seen_default && not is_optional_or_default then
            Typing_error.(
              primary
              @@
              if from_abstract_method then
                Primary.Previous_default_or_optional param.param_pos
              else
                Primary.Previous_default param.param_pos)
            :: acc
          else
            acc
        in
        (* We've seen at least one required parameter, and there's an
            optional parameter after it.  Given an error, and then stop looking
            for more errors in this parameter list. *)
        loop is_optional_or_default acc rl
  in
  loop false [] paraml

let make_params env paraml = List.map paraml ~f:(make_param_ty env)
