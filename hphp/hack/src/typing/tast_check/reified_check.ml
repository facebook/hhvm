(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Tast
open Typing_defs

module Env = Tast_env
module UA = Naming_special_names.UserAttributes
module Cls = Typing_classes_heap

let tparams_has_reified tparams =
  List.exists ~f:(fun t -> t.tp_reified) tparams

let valid_newable_hint env tp (pos, hint) =
  match hint with
  | Aast.Happly ((p, h), _) ->
    begin match Tast_env.get_class env h with
    | Some cls ->
      if Cls.kind cls <> Ast.Cnormal then
        Errors.invalid_newable_type_argument tp p
    | None ->
      (* This case should never happen *)
      Errors.invalid_newable_type_argument tp p end
  | Aast.Habstr name ->
    if not @@ Tast_env.get_newable env name then
      Errors.invalid_newable_type_argument tp pos
  | _ ->
    Errors.invalid_newable_type_argument tp pos

let verify_has_consistent_bound env (tparam: Tast.tparam) =
  let upper_bounds = Typing_set.elements (Tast_env.get_upper_bounds env (snd tparam.tp_name)) in
  let bound_classes = List.filter_map upper_bounds ~f:(function
    | _, Tclass ((_, class_id), _, _) ->
      Env.get_class env class_id
    | _ -> None) in
  let valid_classes = List.filter bound_classes ~f:Tast_utils.valid_newable_class in
  if List.length valid_classes <> 1 then
    let cbs = List.map ~f:(Cls.name) valid_classes in
    Errors.invalid_newable_type_param_constraints tparam.tp_name cbs


(* When passing targs to a reified position, they must either be concrete types
 * or reified type parameters. This prevents the case of
 *
 * class C<reify Tc> {}
 * function f<Tf>(): C<Tf> {}
 *
 * where Tf does not exist at runtime.
 *)
let verify_targ_valid_for_reified_tparam env tparam targ =
  begin if tparam.tp_reified then
    let ty = Env.hint_to_ty env targ in
    match Typing_generic.IsGeneric.ty (Tast_env.get_tcopt env) ty with
    | Some resolved_targ when not (Tast_env.get_reified env (snd resolved_targ)) ->
      Errors.erased_generic_passed_to_reified tparam.tp_name resolved_targ
    | _ -> () end;

  begin if Attributes.mem UA.uaEnforceable tparam.tp_user_attributes then
    Type_test_hint_check.validate_hint env targ
      (Errors.invalid_enforceable_type "parameter" tparam.tp_name) end;

  begin if Attributes.mem UA.uaNewable tparam.tp_user_attributes then
    valid_newable_hint env tparam.tp_name targ end


let verify_call_targs env expr_pos decl_pos tparams targs =
  if tparams_has_reified tparams &&
     List.is_empty targs then
    Errors.require_args_reify decl_pos expr_pos;
  (* Unequal_lengths case handled elsewhere *)
  List.iter2 tparams targs ~f:begin fun tparam targ ->
    verify_targ_valid_for_reified_tparam env tparam targ
  end |> ignore

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env x =
    (* only considering functions where one or more params are reified *)
    match x with
    | (pos, _), Call (_, ((_, (_, Tfun { ft_pos; ft_tparams; _ })), _), targs, _, _) ->
      let tparams = fst ft_tparams in
      verify_call_targs env pos ft_pos tparams targs
    | (pos, _), New (((_, ty), CI (_, class_id)), targs, _, _, _) ->
      begin match ty with
      | (_, Tabstract (AKgeneric ci, None)) when ci = class_id ->
        if not (Env.get_newable env ci) then
          Errors.new_without_newable pos ci;
        if not (List.is_empty targs) then
          Errors.tparam_with_tparam pos ci;
      | _ ->
        match Env.get_class env class_id with
        | Some cls ->
          let tparams = Typing_classes_heap.tparams cls in
          let class_pos = Typing_classes_heap.pos cls in
          verify_call_targs env pos class_pos tparams targs
        | None -> () end
    | (pos, _), New ((_, CIstatic), _, _, _, _) ->
      let open Option in
      let t = Tast_env.get_self_id env >>=
        Tast_env.get_class env >>|
        Cls.tparams >>|
        List.exists ~f:(fun tp -> tp.tp_reified) in
      Option.iter t ~f:(fun has_reified -> if has_reified then
        Errors.new_static_class_reified pos
      )
    | _ -> ()

  method! at_hint env = function
    | pos, Aast.Happly ((_, class_id), targs) ->
      let tc = Env.get_class env class_id in
      Option.iter tc ~f:(fun tc ->
        let tparams = Typing_classes_heap.tparams tc in
        ignore (List.iter2 tparams targs ~f:(verify_targ_valid_for_reified_tparam env));

        (* TODO: This check could be unified with the existence check above,
         * but would require some consolidation T38941033. List.iter2 gives
         * a nice Or_unequal_lengths.t result that replaces this if statement *)
        let tparams_length = List.length tparams in
        let targs_length = List.length targs in
        if tparams_length <> targs_length then
          if targs_length <> 0
          then Errors.type_arity pos class_id (string_of_int (tparams_length))
          else if tparams_has_reified tparams then
            Errors.require_args_reify (Typing_classes_heap.pos tc) pos
      )
    | _ ->
      ()

  method! at_tparam env tparam =
    (* Can't use Attributes.mem here because of a conflict between Nast.user_attributes and Tast.user_attributes *)
    if List.exists tparam.tp_user_attributes (fun { ua_name; _ } -> UA.uaNewable = snd ua_name) then
      verify_has_consistent_bound env tparam

  method! at_class_typeconst env { c_tconst_name = (_, name); c_tconst_type; _ } =
    let open Option in
    let t = Tast_env.get_self_id env >>=
      Tast_env.get_class env >>=
      (fun cls -> Typing_classes_heap.get_typeconst cls name) in
    match t with
    | Some { ttc_enforceable = (pos, enforceable); _ } ->
      (* using c_tconst_type here instead of ttc_type because the Type_test_hint_check works on
       * hints instead of decl tys *)
      begin match c_tconst_type with
      | Some h when enforceable ->
        Type_test_hint_check.validate_hint env h
          (Errors.invalid_enforceable_type "constant" (pos, name))
      | _ -> () end
    | _ ->
      ()

end
