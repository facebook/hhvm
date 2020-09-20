(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
module SN = Naming_special_names

let has_attribute (name : string) (attrs : Nast.user_attribute list) : bool =
  let matches_name { ua_name = (_, attr_name); _ } =
    String.equal attr_name name
  in
  List.exists matches_name attrs

let variadic_pos v : pos option =
  match v with
  | FVvariadicArg param -> Some param.param_pos
  | FVellipsis p -> Some p
  | FVnonVariadic -> None

let check_deprecated_arity attrs =
  let attr = Naming_attributes.find SN.UserAttributes.uaDeprecated attrs in
  match attr with
  | Some { ua_name = _; ua_params = [msg] }
  | Some { ua_name = _; ua_params = [msg; _] } ->
    begin
      match Nast_eval.static_string msg with
      | Error p -> Errors.attribute_param_type p "static string literal"
      | _ -> ()
    end
  | Some { ua_name = (pos, _); ua_params = [] } ->
    Errors.attribute_too_few_arguments pos SN.UserAttributes.uaDeprecated 1
  | Some { ua_name = (pos, _); ua_params = _ } ->
    Errors.attribute_too_many_arguments pos SN.UserAttributes.uaDeprecated 2
  | None -> ()

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ _ f =
      (* Ban arguments on functions with the __EntryPoint attribute. *)
      if has_attribute "__EntryPoint" f.f_user_attributes then begin
        (match f.f_params with
        | [] -> ()
        | param :: _ -> Errors.entrypoint_arguments param.param_pos);
        match variadic_pos f.f_variadic with
        | Some p -> Errors.entrypoint_arguments p
        | None -> ()
      end;
      (* Ban variadic arguments on memoized functions. *)
      ( if has_attribute "__Memoize" f.f_user_attributes then
        match variadic_pos f.f_variadic with
        | Some p -> Errors.variadic_memoize p
        | None -> () );
      check_deprecated_arity f.f_user_attributes

    method! at_method_ _ m =
      check_deprecated_arity m.m_user_attributes;
      (* Ban variadic arguments on memoized methods. *)
      if
        has_attribute "__Memoize" m.m_user_attributes
        || has_attribute "__MemoizeLSB" m.m_user_attributes
      then
        match variadic_pos m.m_variadic with
        | Some p -> Errors.variadic_memoize p
        | None -> ()
  end
