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
        ~splat:(Option.is_some param.param_splat)
        ~named:(Option.is_some param.param_named);
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

(** Check for `optional` keyword where it isn't allowed. Example: `function foo(optional int $x): void {}` (should be a default arg instead) *)
let check_params_for_bad_optional_keyword ~from_abstract_method env paraml :
    Typing_error.t list =
  List.filter_map paraml ~f:(fun param ->
      let is_optional_not_default =
        Aast_utils.is_param_optional param
        && Option.is_none (Aast_utils.get_param_default param)
      in
      if is_optional_not_default then
        if
          not
            (TypecheckerOptions.enable_abstract_method_optional_parameters
               (Decl_env.tcopt env))
        then
          Some
            Typing_error.(
              primary
              @@ Primary.Optional_parameter_not_supported param.param_pos)
        else if not from_abstract_method then
          Some
            Typing_error.(
              primary @@ Primary.Optional_parameter_not_abstract param.param_pos)
        else
          None
      else
        None)

(** Required positional parameters must not come before optional positional parameters.
    For example: function foo(int $x, ?int $y = null, int $z)
    produces an error for parameter $z *)
let check_params_for_required_after_optional ~from_abstract_method paraml :
    Typing_error.t option =
  let open struct
    type ('a, 'b) t =
      | Init
      | Seen_optional_or_default
      | Found_required_after_optional_or_default of ('a, 'b) Aast_defs.fun_param
  end in
  let positional_params =
    List.filter paraml ~f:(fun param -> Option.is_none param.param_named)
  in
  let res =
    List.fold positional_params ~init:Init ~f:(fun state param : _ t ->
        let is_optional_or_default =
          Option.is_some (Aast_utils.get_param_default param)
          || Aast_utils.is_param_optional param
        in
        match state with
        | Init when is_optional_or_default -> Seen_optional_or_default
        | Init -> Init
        | Seen_optional_or_default
          when is_optional_or_default || Aast_utils.is_param_variadic param ->
          Seen_optional_or_default
        | Seen_optional_or_default ->
          Found_required_after_optional_or_default param
        | Found_required_after_optional_or_default _ -> state)
  in
  match res with
  | Init
  | Seen_optional_or_default ->
    None
  | Found_required_after_optional_or_default param when from_abstract_method ->
    Some
      Typing_error.(
        primary @@ Primary.Previous_default_or_optional param.param_pos)
  | Found_required_after_optional_or_default param ->
    Some Typing_error.(primary @@ Primary.Previous_default param.param_pos)

let check_params ~from_abstract_method env paraml : Typing_error.t list =
  let err =
    check_params_for_required_after_optional ~from_abstract_method paraml
  in
  Option.to_list err
  @ check_params_for_bad_optional_keyword ~from_abstract_method env paraml

let make_params env paraml = List.map paraml ~f:(make_param_ty env)
