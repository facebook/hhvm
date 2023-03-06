(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

(** Information about type parameters that are and were in scope. The first boolean flag
  indicates if the variable is still in scope or if it was only in scope earlier
  (true means still in scope).
  The second boolean flag indicates whether the type parameter is higher-kinded *)
type tparam_info = (pos * bool * bool) SMap.t

let error_if_is_this (pos, name) =
  if String.equal (String.lowercase name) "this" then
    Errors.add_naming_error @@ Naming_error.This_reserved pos

let error_if_invalid_tparam_name ~nested ~is_hk (pos, name) =
  match
    ( String.equal name Naming_special_names.Typehints.wildcard,
      nested && not is_hk )
  with
  | (true, false) ->
    Errors.add_naming_error @@ Naming_error.Wildcard_tparam_disallowed pos
  | (true, true) -> ()
  | _ ->
    if String.is_empty name || not (Char.equal name.[0] 'T') then
      Errors.add_naming_error @@ Naming_error.Start_with_T pos

let error_if_reified ~because_nested (pos, name) = function
  | Erased -> ()
  | SoftReified
  | Reified ->
    Errors.add_naming_error
      Naming_error.(
        HKT_unsupported_feature
          { pos; because_nested; var_name = name; feature = Ft_reification })

let error_if_user_attributes ~because_nested (pos, name) attrs =
  if not (List.is_empty attrs) then
    Errors.add_naming_error
      Naming_error.(
        HKT_unsupported_feature
          { pos; because_nested; var_name = name; feature = Ft_user_attrs })

let error_if_not_invariant ~because_nested (pos, name) =
  let open Ast_defs in
  function
  | Invariant -> ()
  | Covariant
  | Contravariant ->
    Errors.add_naming_error
      Naming_error.(
        HKT_unsupported_feature
          { pos; because_nested; var_name = name; feature = Ft_variance })

let error_if_constraints_present ~because_nested (pos, name) constraints =
  if not (List.is_empty constraints) then
    Errors.add_naming_error
      Naming_error.(
        HKT_unsupported_feature
          { pos; because_nested; var_name = name; feature = Ft_constraints })

let rec check_tparam ~nested (seen : tparam_info) tparam =
  let name = tparam.tp_name in
  let is_higher_kinded = not (List.is_empty tparam.tp_parameters) in
  error_if_is_this name;
  error_if_invalid_tparam_name ~nested ~is_hk:is_higher_kinded name;

  if nested then begin
    error_if_constraints_present ~because_nested:true name tparam.tp_constraints;
    error_if_reified ~because_nested:true name tparam.tp_reified;
    error_if_user_attributes ~because_nested:true name tparam.tp_user_attributes;
    error_if_not_invariant ~because_nested:true name tparam.tp_variance
  end;

  if is_higher_kinded then begin
    error_if_constraints_present
      ~because_nested:false
      name
      tparam.tp_constraints;
    error_if_reified ~because_nested:false name tparam.tp_reified;
    error_if_user_attributes
      ~because_nested:false
      name
      tparam.tp_user_attributes;
    error_if_not_invariant ~because_nested:false name tparam.tp_variance
  end;

  check_tparams ~nested:true seen tparam.tp_parameters

(* See not on Naming.type_param about scoping of type parameters *)
and check_tparams ~nested (seen : tparam_info) tparams =
  let bring_into_scope (seen : tparam_info) tparam =
    let is_hk = not (List.is_empty tparam.tp_parameters) in
    let (pos, name) = tparam.tp_name in
    if String.equal name Naming_special_names.Typehints.wildcard then
      seen
    else begin
      (match SMap.find_opt name seen with
      | Some (prev_pos, true, _) ->
        Errors.add_naming_error
        @@ Naming_error.Shadowed_tparam { pos; prev_pos; tparam_name = name }
      | Some (_, false, _) ->
        Errors.add_naming_error
          (Naming_error.Tparam_non_shadowing_reuse { pos; tparam_name = name })
      | None -> ());
      SMap.add name (pos, true, is_hk) seen
    end
  in
  let remove_from_scope (seen : tparam_info) tparam =
    let (pos, name) = tparam.tp_name in
    if String.equal name Naming_special_names.Typehints.wildcard then
      seen
    else
      (* Using a dummy value for the higher-kindedness, we don't care once
         it's out of scope *)
      SMap.add name (pos, false, false) seen
  in

  let seen = List.fold_left tparams ~f:bring_into_scope ~init:seen in
  let seen = List.fold_left tparams ~f:(check_tparam ~nested) ~init:seen in
  if nested then
    List.fold_left tparams ~f:remove_from_scope ~init:seen
  else
    seen

let check_where_constraints (seen : tparam_info) cstrs =
  let visitor =
    object (this)
      inherit [_] Aast.iter as super

      method! on_hint env (pos, h) =
        match h with
        | Aast.Habstr (t, args) ->
          (match SMap.find_opt t seen with
          | Some (_, true, true) ->
            Errors.add_naming_error
              Naming_error.(
                HKT_unsupported_feature
                  {
                    pos;
                    because_nested = false;
                    var_name = t;
                    feature = Ft_where_constraints;
                  })
          | Some _
          | None ->
            ());
          List.iter args ~f:(this#on_hint env)
        | _ -> super#on_hint env (pos, h)
    end
  in
  List.iter cstrs ~f:(fun (h1, _, h2) ->
      visitor#on_hint () h1;
      visitor#on_hint () h2)

let check_class class_ =
  let seen_class_tparams =
    check_tparams ~nested:false SMap.empty class_.c_tparams
  in

  (* Due to ~nested:false above, the class tparams are still marked as in scope *)
  let check_method method_tparams method_where_cstrs =
    let seen = check_tparams ~nested:false seen_class_tparams method_tparams in
    check_where_constraints seen method_where_cstrs
  in
  List.iter class_.c_methods ~f:(fun m ->
      check_method m.m_tparams m.m_where_constraints)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ _ fun_ =
      let seen = check_tparams ~nested:false SMap.empty fun_.f_tparams in
      (* Due to ~nested:false above, the function tparams are still marked as in scope *)
      check_where_constraints seen fun_.f_where_constraints

    method! at_class_ _ = check_class

    method! at_typedef _ typedef =
      check_tparams ~nested:false SMap.empty typedef.t_tparams |> ignore
  end
