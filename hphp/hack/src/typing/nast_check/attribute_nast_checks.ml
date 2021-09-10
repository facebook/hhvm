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

let find_attribute (name : string) (attrs : Nast.user_attribute list) :
    Nast.user_attribute option =
  let matches_name { ua_name = (_, attr_name); _ } =
    String.equal attr_name name
  in
  List.find_opt matches_name attrs

let has_attribute (name : string) (attrs : Nast.user_attribute list) : bool =
  match find_attribute name attrs with
  | Some _ -> true
  | None -> false

let variadic_pos v : pos option =
  match v with
  | FVvariadicArg param -> Some param.param_pos
  | FVellipsis p -> Some p
  | FVnonVariadic -> None

let check_attribute_arity attrs attr_name arg_spec =
  let attr = Naming_attributes.find attr_name attrs in
  match (arg_spec, attr) with
  | (`Range (min_args, _), Some { ua_name = (pos, _); ua_params })
    when List.length ua_params < min_args ->
    Errors.attribute_too_few_arguments pos attr_name min_args
  | (`Range (_, max_args), Some { ua_name = (pos, _); ua_params })
    when List.length ua_params > max_args ->
    Errors.attribute_too_many_arguments pos attr_name max_args
  | (`Exact expected_args, Some { ua_name = (pos, _); ua_params })
    when List.length ua_params <> expected_args ->
    Errors.attribute_not_exact_number_of_args
      pos
      ~attr_name
      ~expected_args
      ~actual_args:(List.length ua_params)
  | _ -> ()

let check_deprecated_static attrs =
  let attr = Naming_attributes.find SN.UserAttributes.uaDeprecated attrs in
  match attr with
  | Some { ua_name = _; ua_params = [msg] }
  | Some { ua_name = _; ua_params = [msg; _] } ->
    begin
      match Nast_eval.static_string msg with
      | Error p -> Errors.attribute_param_type p "static string literal"
      | _ -> ()
    end
  | _ -> ()

let check_ifc_enabled tcopt attrs =
  let inferflows_opt = find_attribute SN.UserAttributes.uaInferFlows attrs in
  match
    ( inferflows_opt,
      TypecheckerOptions.experimental_feature_enabled
        tcopt
        TypecheckerOptions.experimental_infer_flows )
  with
  | (Some { ua_name = (pos, _); _ }, false) ->
    Errors.experimental_feature pos "IFC InferFlows"
  | _ -> ()

let check_internal_method_visibility m =
  let attr =
    Naming_attributes.find SN.UserAttributes.uaInternal m.m_user_attributes
  in
  match (attr, m.m_visibility) with
  | (Some { ua_name = (pos, _); _ }, Aast.Private)
  | (Some { ua_name = (pos, _); _ }, Aast.Protected) ->
    Errors.internal_method_with_invalid_visibility
      ~attr_pos:pos
      ~visibility:m.m_visibility
  | (_, _) -> ()

(* TODO: error if both Governed and InferFlows are attributes on a function or method *)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ env f =
      (* Ban arguments on functions with the __EntryPoint attribute. *)
      if has_attribute "__EntryPoint" f.f_user_attributes then begin
        (match f.f_params with
        | [] -> ()
        | param :: _ -> Errors.entrypoint_arguments param.param_pos);
        (match variadic_pos f.f_variadic with
        | Some p -> Errors.entrypoint_arguments p
        | None -> ());
        match f.f_tparams with
        | [] -> ()
        | tparam :: _ -> Errors.entrypoint_generics (fst tparam.tp_name)
      end;
      (* Ban variadic arguments on memoized functions. *)
      (if has_attribute "__Memoize" f.f_user_attributes then
        match variadic_pos f.f_variadic with
        | Some p -> Errors.variadic_memoize p
        | None -> ());
      check_attribute_arity
        f.f_user_attributes
        SN.UserAttributes.uaPolicied
        (`Range (0, 1));
      check_attribute_arity
        f.f_user_attributes
        SN.UserAttributes.uaInferFlows
        (`Exact 0);
      check_attribute_arity
        f.f_user_attributes
        SN.UserAttributes.uaDeprecated
        (`Range (1, 2));
      check_deprecated_static f.f_user_attributes;
      check_ifc_enabled (Nast_check_env.get_tcopt env) f.f_user_attributes;
      let params = f.f_params in
      List.iter
        (fun fp ->
          check_attribute_arity
            fp.param_user_attributes
            SN.UserAttributes.uaExternal
            (`Exact 0);
          check_attribute_arity
            fp.param_user_attributes
            SN.UserAttributes.uaCanCall
            (`Exact 0))
        params;
      check_attribute_arity
        f.f_user_attributes
        SN.UserAttributes.uaModule
        (`Exact 1);
      check_attribute_arity
        f.f_user_attributes
        SN.UserAttributes.uaInternal
        (`Exact 0)

    method! at_method_ env m =
      check_attribute_arity
        m.m_user_attributes
        SN.UserAttributes.uaPolicied
        (`Range (0, 1));
      check_attribute_arity
        m.m_user_attributes
        SN.UserAttributes.uaInferFlows
        (`Exact 0);
      check_ifc_enabled (Nast_check_env.get_tcopt env) m.m_user_attributes;
      check_attribute_arity
        m.m_user_attributes
        SN.UserAttributes.uaDeprecated
        (`Range (1, 2));
      check_deprecated_static m.m_user_attributes;
      (* Ban variadic arguments on memoized methods. *)
      (if
       has_attribute "__Memoize" m.m_user_attributes
       || has_attribute "__MemoizeLSB" m.m_user_attributes
      then
        match variadic_pos m.m_variadic with
        | Some p -> Errors.variadic_memoize p
        | None -> ());
      check_attribute_arity
        m.m_user_attributes
        SN.UserAttributes.uaInternal
        (`Exact 0);
      check_internal_method_visibility m

    method! at_class_ _env c =
      check_attribute_arity
        c.c_user_attributes
        SN.UserAttributes.uaModule
        (`Exact 1)
  end
