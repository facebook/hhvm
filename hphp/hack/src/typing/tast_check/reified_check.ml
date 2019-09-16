(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast
open Typing_defs
open Type_validator
module Env = Tast_env
module SN = Naming_special_names
module UA = SN.UserAttributes
module Cls = Decl_provider.Class
module Nast = Aast

let validator =
  object (this)
    inherit type_validator as super

    method! on_tapply acc r (p, h) tyl =
      if h = SN.Classes.cTypename then
        this#invalid acc r "a typename"
      else if h = SN.Classes.cClassname then
        this#invalid acc r "a classname"
      else if
        h = SN.Typehints.wildcard && not (Env.get_allow_wildcards acc.env)
      then
        this#invalid acc r "a wildcard"
      else
        super#on_tapply acc r (p, h) tyl

    method! on_tgeneric acc r t =
      match Env.get_reified acc.env t with
      | Nast.Erased -> this#invalid acc r "not reified"
      | Nast.SoftReified -> this#invalid acc r "soft reified"
      | Nast.Reified -> acc

    method! on_tarray acc r _ _ = this#invalid acc r "an array type"

    method! on_tvarray_or_darray acc r _ = this#invalid acc r "an array type"

    method! on_tfun acc r _ = this#invalid acc r "a function type"

    method! on_typeconst acc is_concrete typeconst =
      match typeconst.ttc_abstract with
      | _ when typeconst.ttc_reifiable <> None || is_concrete ->
        super#on_typeconst acc is_concrete typeconst
      | _ ->
        let r = Reason.Rwitness (fst typeconst.ttc_name) in
        let kind =
          "an abstract type constant without the __Reifiable attribute"
        in
        this#invalid acc r kind

    method! on_tthis acc r =
      this#invalid acc r "the late static bound this type"
  end

let tparams_has_reified tparams =
  List.exists tparams ~f:(fun tparam -> tparam.tp_reified <> Nast.Erased)

let check_explicit expr_pos tparams =
  List.iter tparams ~f:(fun tparam ->
      if Attributes.mem UA.uaExplicit tparam.tp_user_attributes then
        Errors.require_generic_explicit tparam.tp_name expr_pos)

let valid_newable_hint env tp (pos, hint) =
  match hint with
  | Aast.Happly ((p, h), _) ->
    begin
      match Env.get_class env h with
      | Some cls ->
        if Cls.kind cls <> Ast_defs.Cnormal then
          Errors.invalid_newable_type_argument tp p
      | None ->
        (* This case should never happen *)
        Errors.invalid_newable_type_argument tp p
    end
  | Aast.Habstr name ->
    if not @@ Env.get_newable env name then
      Errors.invalid_newable_type_argument tp pos
  | _ -> Errors.invalid_newable_type_argument tp pos

let verify_has_consistent_bound env (tparam : Tast.tparam) =
  let upper_bounds =
    Typing_set.elements (Env.get_upper_bounds env (snd tparam.tp_name))
  in
  let bound_classes =
    List.filter_map upper_bounds ~f:(function
        | (_, Tclass ((_, class_id), _, _)) -> Env.get_class env class_id
        | _ -> None)
  in
  let valid_classes =
    List.filter bound_classes ~f:Tast_utils.valid_newable_class
  in
  if List.length valid_classes <> 1 then
    let cbs = List.map ~f:Cls.name valid_classes in
    Errors.invalid_newable_type_param_constraints tparam.tp_name cbs

(* When passing targs to a reified position, they must either be concrete types
 * or reified type parameters. This prevents the case of
 *
 * class C<reify Tc> {}
 * function f<Tf>(): C<Tf> {}
 *
 * where Tf does not exist at runtime.
 *)
let verify_targ_valid env tparam targ =
  if Attributes.mem UA.uaExplicit tparam.tp_user_attributes then
    match targ with
    | (pos, Aast.Happly ((_, class_id), _targs))
      when class_id = SN.Typehints.wildcard ->
      Errors.require_generic_explicit tparam.tp_name pos
    | _ -> ()
  else
    ();

  (* There is some subtlety here. If a type *parameter* is declared reified,
   * even if it is soft, we require that the argument be concrete or reified, not soft
   * reified or erased *)
  ( if tparam.tp_reified = Nast.Erased then
    ()
  else
    match tparam.tp_reified with
    | Nast.Reified
    | Nast.SoftReified ->
      let emit_error = Errors.invalid_reified_argument tparam.tp_name in
      validator#validate_hint env targ emit_error
    | Nast.Erased -> () );

  if Attributes.mem UA.uaEnforceable tparam.tp_user_attributes then
    Enforceable_hint_check.validator#validate_hint
      env
      targ
      (Errors.invalid_enforceable_type "parameter" tparam.tp_name);

  if Attributes.mem UA.uaNewable tparam.tp_user_attributes then
    valid_newable_hint env tparam.tp_name targ

let verify_call_targs env expr_pos decl_pos tparams targs =
  if tparams_has_reified tparams && List.is_empty targs then
    Errors.require_args_reify decl_pos expr_pos;
  if List.is_empty targs then check_explicit expr_pos tparams;

  (* Unequal_lengths case handled elsewhere *)
  List.iter2 tparams targs ~f:(fun tparam targ ->
      verify_targ_valid env tparam targ)
  |> ignore

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env x =
      (* only considering functions where one or more params are reified *)
      match x with
      | ((pos, _), Class_get ((_, CI (_, t)), _)) ->
        if Env.get_reified env t = Reified then Errors.class_get_reified pos
      | ( (pos, _),
          Call (_, ((_, (_, Tfun { ft_pos; ft_tparams; _ })), _), targs, _, _)
        ) ->
        let tparams = fst ft_tparams in
        verify_call_targs env pos ft_pos tparams targs
      | ((pos, _), New (((_, ty), CI (_, class_id)), targs, _, _, _)) ->
        begin
          match ty with
          | (_, Tabstract (AKgeneric ci, None)) when ci = class_id ->
            if not (Env.get_newable env ci) then
              Errors.new_without_newable pos ci;
            if not (List.is_empty targs) then Errors.tparam_with_tparam pos ci
          | _ ->
            (match Env.get_class env class_id with
            | Some cls ->
              let tparams = Cls.tparams cls in
              let class_pos = Cls.pos cls in
              verify_call_targs env pos class_pos tparams targs
            | None -> ())
        end
      | ((pos, _), New ((_, CIstatic), _, _, _, _)) ->
        Option.(
          let t =
            Env.get_self_id env
            >>= Env.get_class env
            >>| Cls.tparams
            >>| tparams_has_reified
          in
          Option.iter t ~f:(fun has_reified ->
              if has_reified then Errors.new_static_class_reified pos))
      | _ -> ()

    method! at_hint env =
      function
      | (pos, Aast.Happly ((_, class_id), targs)) ->
        let tc = Env.get_class env class_id in
        Option.iter tc ~f:(fun tc ->
            let tparams = Cls.tparams tc in
            ignore (List.iter2 tparams targs ~f:(verify_targ_valid env));

            (* TODO: This check could be unified with the existence check above,
             * but would require some consolidation T38941033. List.iter2 gives
             * a nice Or_unequal_lengths.t result that replaces this if statement *)
            let tparams_length = List.length tparams in
            let targs_length = List.length targs in
            if tparams_length <> targs_length then
              let c_pos = Cls.pos tc in
              if targs_length <> 0 then
                Errors.type_arity
                  pos
                  class_id
                  (string_of_int tparams_length)
                  c_pos
              else if tparams_has_reified tparams then
                Errors.require_args_reify c_pos pos)
      | _ -> ()

    method! at_tparam env tparam =
      (* Can't use Attributes.mem here because of a conflict between Nast.user_attributes and Tast.user_attributes *)
      if
        List.exists tparam.tp_user_attributes (fun { ua_name; _ } ->
            UA.uaNewable = snd ua_name)
      then
        verify_has_consistent_bound env tparam

    method! at_class_ env { c_name = (pos, name); _ } =
      match Env.get_class env name with
      | Some cls ->
        begin
          match Cls.construct cls with
          | (_, Typing_defs.ConsistentConstruct) ->
            if
              List.exists
                ~f:(fun t -> t.tp_reified <> Nast.Erased)
                (Cls.tparams cls)
            then
              Errors.consistent_construct_reified pos
          | _ -> ()
        end
      | None -> ()
  end
