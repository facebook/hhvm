(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module Env = Tast_env
module Reason = Typing_reason
module MakeType = Typing_make_type
module Typing = Typing_defs
module Cls = Decl_provider.Class

let ensure_valid_switch_case_value_types env scrutinee_ty casel =
  let is_subtype ty_sub ty_super = Env.can_subtype env ty_sub ty_super in
  let ty_num = MakeType.num Reason.Rnone in
  let ty_arraykey = MakeType.arraykey Reason.Rnone in
  let ty_mixed = MakeType.mixed Reason.Rnone in
  let ty_traversable = MakeType.traversable Typing_reason.Rnone ty_mixed in
  let compatible_types ty1 ty2 =
    (is_subtype ty1 ty_num && is_subtype ty2 ty_num)
    || (is_subtype ty1 ty_arraykey && is_subtype ty2 ty_arraykey)
    || is_subtype ty1 ty_traversable
       && is_subtype ty2 ty_traversable
       && (is_subtype ty1 ty2 || is_subtype ty2 ty1)
    || (is_subtype ty1 ty2 && is_subtype ty2 ty1)
  in
  let ensure_valid_switch_case_value_type = function
    | Default _ -> ()
    | Case ((case_value_ty, case_value_p, _), _) ->
      if not (compatible_types case_value_ty scrutinee_ty) then
        Lints_errors.invalid_switch_case_value_type
          case_value_p
          (lazy (Env.print_ty env case_value_ty))
          (lazy (Env.print_ty env scrutinee_ty))
  in
  List.iter casel ~f:ensure_valid_switch_case_value_type

let check_exhaustiveness_lint env pos ty caselist =
  let rec has_infinite_values ty =
    match Typing.get_node ty with
    | Typing.Tunion tyl -> List.exists tyl ~f:has_infinite_values
    | Typing.Tintersection tyl -> List.for_all tyl ~f:has_infinite_values
    | Typing.Tprim (Tstring | Tint | Tfloat | Tarraykey | Tnum) -> true
    | Typing.Tclass ((_, name), _, _) ->
      begin
        match Decl_provider.get_class (Env.get_ctx env) name with
        | Some class_decl
          when not
                 (Cls.final class_decl
                 || Option.is_some (Cls.sealed_whitelist class_decl)) ->
          true
        | _ -> false
      end
    | _ -> false
  in
  let has_default cases =
    List.exists
      ~f:(function
        | Default _ -> true
        | _ -> false)
      cases
  in
  if has_infinite_values ty && not (has_default caselist) then
    Lints_errors.switch_nonexhaustive pos

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_stmt env x =
      match snd x with
      | Switch ((scrutinee_ty, scrutinee_pos, _), casel) ->
        ensure_valid_switch_case_value_types env scrutinee_ty casel;
        check_exhaustiveness_lint env scrutinee_pos scrutinee_ty casel
      | _ -> ()
  end
