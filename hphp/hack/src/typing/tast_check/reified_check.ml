(**
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

module Env = Tast_env
module SN = Naming_special_names
module UA = SN.UserAttributes
module Cls = Decl_provider.Class
module Nast = Aast

let tparams_has_reified tparams =
  List.exists tparams ~f:(fun tparam -> tparam.tp_reified <> Nast.Erased)

let valid_newable_hint env tp (pos, hint) =
  match hint with
  | Aast.Happly ((p, h), _) ->
    begin match Env.get_class env h with
    | Some cls ->
      if Cls.kind cls <> Ast_defs.Cnormal then
        Errors.invalid_newable_type_argument tp p
    | None ->
      (* This case should never happen *)
      Errors.invalid_newable_type_argument tp p end
  | Aast.Habstr name ->
    if not @@ Env.get_newable env name then
      Errors.invalid_newable_type_argument tp pos
  | _ ->
    Errors.invalid_newable_type_argument tp pos

let verify_has_consistent_bound env (tparam: Tast.tparam) =
  let upper_bounds = Typing_set.elements (Env.get_upper_bounds env (snd tparam.tp_name)) in
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
let verify_targ_valid env tparam targ =
  (* There is some subtlety here. If a type *parameter* is declared reified,
   * even if it is soft, we require that the argument be concrete or reified, not soft
   * reified or erased *)
  if tparam.tp_reified = Nast.Erased then () else
  begin match tparam.tp_reified with
  | Nast.Reified
  | Nast.SoftReified ->
    let p, _h = targ in
    let ty = Env.hint_to_ty env targ in
    begin match ty with
    | _, Tapply ((pw, h), [])
      when h = SN.Typehints.wildcard && not (Env.get_allow_wildcards env) ->
      Errors.invalid_reified_argument tparam.tp_name pw "a wildcard"
    | _, Tapply ((pw, h), _) when h = SN.Classes.cClassname ->
      Errors.invalid_reified_argument tparam.tp_name pw "a classname"
    | _, Tapply ((pw, h), _) when h = SN.Classes.cTypename ->
      Errors.invalid_reified_argument tparam.tp_name pw "a typename"
    | _, Tgeneric t ->
      begin match (Env.get_reified env t) with
      | Nast.Erased -> Errors.invalid_reified_argument tparam.tp_name p "not reified"
      | Nast.SoftReified -> Errors.invalid_reified_argument tparam.tp_name p "soft reified"
      | Nast.Reified -> () end
    | _, Tarray _
    | _, Tdarray _
    | _, Tvarray _
    | _, Tvarray_or_darray _ ->
      Errors.invalid_reified_argument tparam.tp_name p "an array"
    | _, Tfun { ft_arity = (Fvariadic _ | Fellipsis _); _ } ->
      Errors.invalid_reified_argument tparam.tp_name p "a function type with variadic args"
    | _, Tthis ->
      Errors.invalid_reified_argument tparam.tp_name p "the late static bound this type"
    | _, Taccess _ ->
      let emit_err =
        Errors.invalid_reified_argument_disallow_php_arrays tparam.tp_name
      in
      Type_const_check.php_array_validator#validate_type env ty emit_err
    | _, Tdynamic
    | _, Tfun _
    | _, Tprim _
    | _, Toption _
    | _, Tshape _
    | _, Ttuple _
    | _, Tlike _
    | _, Tapply _
    | _, Tnothing
    | _, Tnonnull
    | _, Tmixed
    | _, Tany
    | _, Terr -> ()
    end;
  | Nast.Erased -> () end;

  begin if Attributes.mem UA.uaEnforceable tparam.tp_user_attributes then
    Enforceable_hint_check.validator#validate_hint env targ
      (Errors.invalid_enforceable_type "parameter" tparam.tp_name) end;

  begin if Attributes.mem UA.uaNewable tparam.tp_user_attributes then
    valid_newable_hint env tparam.tp_name targ end


let verify_call_targs env expr_pos decl_pos tparams targs =
  if tparams_has_reified tparams &&
     List.is_empty targs then
    Errors.require_args_reify decl_pos expr_pos;
  (* Unequal_lengths case handled elsewhere *)
  List.iter2 tparams targs ~f:begin fun tparam targ ->
    verify_targ_valid env tparam targ
  end |> ignore

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env x =
    (* only considering functions where one or more params are reified *)
    match x with
    | (pos, _), Class_get ((_, CI (_, t)), _) ->
      if Env.get_reified env t = Reified then
        Errors.class_get_reified pos
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
          let tparams = Cls.tparams cls in
          let class_pos = Cls.pos cls in
          verify_call_targs env pos class_pos tparams targs
        | None -> () end
    | (pos, _), New ((_, CIstatic), _, _, _, _) ->
      let open Option in
      let t = Env.get_self_id env >>=
        Env.get_class env >>|
        Cls.tparams >>|
        tparams_has_reified in
      Option.iter t ~f:(fun has_reified -> if has_reified then
        Errors.new_static_class_reified pos
      )
    | _ -> ()

  method! at_hint env = function
    | pos, Aast.Happly ((_, class_id), targs) ->
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
            Errors.type_arity pos class_id (string_of_int (tparams_length)) c_pos
          else if tparams_has_reified tparams then
            Errors.require_args_reify c_pos pos
      )
    | _ ->
      ()

  method! at_tparam env tparam =
    (* Can't use Attributes.mem here because of a conflict between Nast.user_attributes and Tast.user_attributes *)
    if List.exists tparam.tp_user_attributes (fun { ua_name; _ } -> UA.uaNewable = snd ua_name) then
      verify_has_consistent_bound env tparam

  method! at_class_ env { c_name = (pos, name); _ } =
    match Env.get_class env name with
    | Some cls ->
      begin match Cls.construct cls with
      | _, Typing_defs.ConsistentConstruct ->
        if List.exists ~f:(fun t -> t.tp_reified <> Nast.Erased) (Cls.tparams cls) then
          Errors.consistent_construct_reified pos;
      | _ -> () end
    | None -> ()

end
