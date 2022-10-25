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

let check_soft_internal_without_internal
    (internal : bool) (attrs : Nast.user_attribute list) =
  match find_attribute SN.UserAttributes.uaSoftInternal attrs with
  | Some { ua_name = (pos, _); _ } when not internal ->
    Errors.add_nast_check_error
    @@ Nast_check_error.Soft_internal_without_internal pos
  | _ -> ()

let check_attribute_arity attrs attr_name arg_spec =
  let attr = Naming_attributes.find attr_name attrs in
  let prim_err_opt =
    match (arg_spec, attr) with
    | (`Range (min_args, _), Some { ua_name = (pos, _); ua_params })
      when List.length ua_params < min_args ->
      Some
        (Typing_error.Primary.Attribute_too_few_arguments
           { pos; name = attr_name; expected = min_args })
      (* Errors.attribute_too_few_arguments pos attr_name min_args *)
    | (`Range (_, max_args), Some { ua_name = (pos, _); ua_params })
      when List.length ua_params > max_args ->
      Some
        (Typing_error.Primary.Attribute_too_many_arguments
           { pos; name = attr_name; expected = max_args })
      (* Errors.attribute_too_many_arguments pos attr_name max_args *)
    | (`Exact expected_args, Some { ua_name = (pos, _); ua_params })
      when List.length ua_params <> expected_args ->
      Some
        (Typing_error.Primary.Attribute_not_exact_number_of_args
           {
             pos;
             name = attr_name;
             expected = expected_args;
             actual = List.length ua_params;
           })
    | _ -> None
  in
  Option.iter
    (fun err -> Errors.add_typing_error @@ Typing_error.primary err)
    prim_err_opt

let attr_pos (attr : ('a, 'b) user_attribute) : Pos.t = fst attr.ua_name

(* Ban methods having both __Memoize and __MemoizeLSB. *)
let check_duplicate_memoize (attrs : Nast.user_attribute list) : unit =
  let memoize = find_attribute SN.UserAttributes.uaMemoize attrs in
  let memoize_lsb = find_attribute SN.UserAttributes.uaMemoizeLSB attrs in
  match (memoize, memoize_lsb) with
  | (Some memoize, Some memoize_lsb) ->
    Errors.add_nast_check_error
      (Nast_check_error.Attribute_conflicting_memoize
         { pos = attr_pos memoize; second_pos = attr_pos memoize_lsb })
  | _ -> ()

let check_deprecated_static attrs =
  let attr = Naming_attributes.find SN.UserAttributes.uaDeprecated attrs in
  match attr with
  | Some { ua_name = _; ua_params = [msg] }
  | Some { ua_name = _; ua_params = [msg; _] } ->
    begin
      match Nast_eval.static_string msg with
      | Error p ->
        Errors.add_typing_error
          Typing_error.(
            primary
            @@ Primary.Attribute_param_type
                 { pos = p; x = "static string literal" })
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

(* TODO: error if both Governed and InferFlows are attributes on a function or method *)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ env f =
      let variadic_param =
        List.find_opt (fun p -> p.param_is_variadic) f.f_params
      in
      (* Ban arguments on functions with the __EntryPoint attribute. *)
      if has_attribute "__EntryPoint" f.f_user_attributes then begin
        (match f.f_params with
        | [] -> ()
        | param :: _ ->
          Errors.add_nast_check_error
          @@ Nast_check_error.Entrypoint_arguments param.param_pos);
        (match variadic_param with
        | Some p ->
          Errors.add_nast_check_error
          @@ Nast_check_error.Entrypoint_arguments p.param_pos
        | None -> ());
        match f.f_tparams with
        | [] -> ()
        | tparam :: _ ->
          Errors.add_nast_check_error
          @@ Nast_check_error.Entrypoint_generics (fst tparam.tp_name)
      end;
      (* Ban variadic arguments on memoized functions. *)
      (if has_attribute "__Memoize" f.f_user_attributes then
        match variadic_param with
        | Some p ->
          Errors.add_nast_check_error
          @@ Nast_check_error.Variadic_memoize p.param_pos
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
        params

    method! at_fun_def _env fd =
      check_soft_internal_without_internal
        fd.fd_internal
        fd.fd_fun.f_user_attributes

    method! at_method_ env m =
      let variadic_param =
        List.find_opt (fun p -> p.param_is_variadic) m.m_params
      in
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
      check_soft_internal_without_internal
        (Aast_defs.equal_visibility m.m_visibility Aast_defs.Internal)
        m.m_user_attributes;
      check_deprecated_static m.m_user_attributes;
      check_duplicate_memoize m.m_user_attributes;
      (* Ban variadic arguments on memoized methods. *)
      if
        has_attribute "__Memoize" m.m_user_attributes
        || has_attribute "__MemoizeLSB" m.m_user_attributes
      then
        match variadic_param with
        | Some p ->
          Errors.add_nast_check_error
          @@ Nast_check_error.Variadic_memoize p.param_pos
        | None -> ()

    method! at_class_ _env c =
      check_soft_internal_without_internal c.c_internal c.c_user_attributes;
      check_attribute_arity
        c.c_user_attributes
        SN.UserAttributes.uaDocs
        (`Exact 1);
      List.iter
        (fun cv ->
          check_soft_internal_without_internal
            (Aast_defs.equal_visibility cv.cv_visibility Aast_defs.Internal)
            cv.cv_user_attributes)
        c.c_vars
  end
