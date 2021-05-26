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
open Type_validator
open Typing_reified_check (* validator *)

module Env = Tast_env
module SN = Naming_special_names
module UA = SN.UserAttributes
module Cls = Decl_provider.Class
module Nast = Aast

let is_reified tparam = not (equal_reify_kind tparam.tp_reified Erased)

let tparams_has_reified tparams = List.exists tparams ~f:is_reified

let valid_newable_hint env tp (pos, hint) =
  match hint with
  | Aast.Happly ((p, h), _) ->
    begin
      match Env.get_class_or_typedef env h with
      | Some (Env.ClassResult cls) ->
        if not Ast_defs.(equal_class_kind (Cls.kind cls) Cnormal) then
          Errors.invalid_newable_type_argument tp p
      | _ ->
        (* This case should never happen *)
        Errors.invalid_newable_type_argument tp p
    end
  | Aast.Habstr (name, []) ->
    if not @@ Env.get_newable env name then
      Errors.invalid_newable_type_argument tp pos
  | _ -> Errors.invalid_newable_type_argument tp pos

let verify_has_consistent_bound env (tparam : Tast.tparam) =
  let upper_bounds =
    (* a newable type parameter cannot be higher-kinded/require arguments *)
    Typing_set.elements (Env.get_upper_bounds env (snd tparam.tp_name) [])
  in
  let bound_classes =
    List.filter_map upper_bounds ~f:(fun ty ->
        match get_node ty with
        | Tclass ((_, class_id), _, _) -> Env.get_class env class_id
        | _ -> None)
  in
  let valid_classes =
    List.filter bound_classes ~f:Tast_utils.valid_newable_class
  in
  if Int.( <> ) 1 (List.length valid_classes) then
    let cbs = List.map ~f:Cls.name valid_classes in
    Errors.invalid_newable_type_param_constraints tparam.tp_name cbs

let is_wildcard (_, hint) =
  match hint with
  | (_, Aast.Happly ((_, class_id), _))
    when String.equal class_id SN.Typehints.wildcard ->
    true
  | _ -> false

(* When passing targs to a reified position, they must either be concrete types
 * or reified type parameters. This prevents the case of
 *
 * class C<reify Tc> {}
 * function f<Tf>(): C<Tf> {}
 *
 * where Tf does not exist at runtime.
 *)
let verify_targ_valid env reification tparam targ =
  (* There is some subtlety here. If a type *parameter* is declared reified,
   * even if it is soft, we require that the argument be concrete or reified, not soft
   * reified or erased *)
  ( if is_reified tparam then
    match tparam.tp_reified with
    | Nast.Reified
    | Nast.SoftReified ->
      let emit_error = Errors.invalid_reified_argument tparam.tp_name in
      validator#validate_hint env (snd targ) ~reification emit_error
    | Nast.Erased -> () );

  if Attributes.mem UA.uaEnforceable tparam.tp_user_attributes then
    Typing_enforceable_hint.validate_hint
      env
      (snd targ)
      (Errors.invalid_enforceable_type "parameter" tparam.tp_name);

  if Attributes.mem UA.uaNewable tparam.tp_user_attributes then
    valid_newable_hint env tparam.tp_name (snd targ)

let verify_call_targs env expr_pos decl_pos tparams targs =
  ( if tparams_has_reified tparams then
    let tparams_length = List.length tparams in
    let targs_length = List.length targs in
    if Int.( <> ) tparams_length targs_length then
      if Int.( = ) targs_length 0 then
        Errors.require_args_reify decl_pos expr_pos
      else
        (* mismatches with targs_length > 0 are not specific to reification and handled
                  elsewhere *)
        () );
  let all_wildcards = List.for_all ~f:is_wildcard targs in
  if all_wildcards && tparams_has_reified tparams then
    Errors.require_args_reify decl_pos expr_pos
  else
    (* Unequal_lengths case handled elsewhere *)
    List.iter2 tparams targs ~f:(verify_targ_valid env Resolved) |> ignore

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env x =
      (* only considering functions where one or more params are reified *)
      match x with
      | ((call_pos, _), Class_get ((_, CI (_, t)), _, _)) ->
        if equal_reify_kind (Env.get_reified env t) Reified then
          Errors.class_get_reified call_pos
      | ((pos, fun_ty), Method_caller _)
      | ((pos, fun_ty), Fun_id _)
      | ((pos, fun_ty), Method_id _)
      | ((pos, fun_ty), Smethod_id _) ->
        begin
          match get_node fun_ty with
          | Tfun { ft_tparams; _ } ->
            if tparams_has_reified ft_tparams then
              Errors.reified_function_reference pos
          | _ -> ()
        end
      | ( (pos, _),
          FunctionPointer (FP_class_const (((_, ty), CI (_, class_id)), _), _)
        )
        when Env.is_in_expr_tree env ->
        let (_env, ty) = Env.expand_type env ty in
        begin
          match get_node ty with
          (* If we get Tgeneric here, the underlying type was reified *)
          | Tgeneric (ci, _) when String.equal ci class_id ->
            Errors.reified_static_method_in_expr_tree pos
          | _ -> ()
        end
      | ((pos, fun_ty), FunctionPointer (_, targs)) ->
        begin
          match get_node fun_ty with
          | Tfun { ft_tparams; _ } ->
            verify_call_targs env pos (get_pos fun_ty) ft_tparams targs
          | _ -> ()
        end
      | ((pos, _), Call (((_, fun_ty), _), targs, _, _)) ->
        let (env, efun_ty) = Env.expand_type env fun_ty in
        (match get_node efun_ty with
        | Tfun ({ ft_tparams; _ } as ty)
          when not @@ get_ft_is_function_pointer ty ->
          verify_call_targs env pos (get_pos efun_ty) ft_tparams targs
        | _ -> ())
      | ((pos, _), New (((_, ty), CI (_, class_id)), targs, _, _, _)) ->
        let (env, ty) = Env.expand_type env ty in
        (match get_node ty with
        | Tgeneric (ci, _tyargs) when String.equal ci class_id ->
          (* ignoring type arguments here: If we get a Tgeneric here, the underlying type
             parameter must have been newable and reified, neither of which his allowed for
             higher-kinded type-parameters *)
          if not (Env.get_newable env ci) then Errors.new_without_newable pos ci
        (* No need to report a separate error here if targs is non-empty:
             If targs is not empty then there are two cases:
             - ci is indeed higher-kinded, in which case it is not allowed to be newable
               (yielding an error above)
             - ci is not higher-kinded. Typing_phase.localize_targs_* is called
               on the the type arguments, reporting the arity mismatch *)
        | _ ->
          (match Env.get_class env class_id with
          | Some cls ->
            let tparams = Cls.tparams cls in
            let class_pos = Cls.pos cls in
            verify_call_targs env pos class_pos tparams targs
          | None -> ()))
      | ( (pos, _),
          New ((_, ((CIstatic | CIself | CIparent) as cid)), _, _, _, _) ) ->
        Option.(
          let t =
            Env.get_self_id env
            >>= Env.get_class env
            >>| Cls.tparams
            >>| tparams_has_reified
          in
          Option.iter t ~f:(fun has_reified ->
              if has_reified then
                let (class_type, suggested_class) =
                  match cid with
                  | CIstatic -> ("static", None)
                  | CIself -> ("self", Env.get_self_id env)
                  | CIparent -> ("parent", Env.get_parent_id env)
                  | _ -> failwith "Unexpected match"
                in
                Errors.new_class_reified pos class_type suggested_class))
      | ((pos, _), New (((_, ty), _), targs, _, _, _)) ->
        let (env, ty) = Env.expand_type env ty in
        begin
          match get_node ty with
          | Tclass ((_, cid), _, _) ->
            begin
              match Env.get_class env cid with
              | Some cls ->
                let tparams = Cls.tparams cls in
                let class_pos = Cls.pos cls in
                verify_call_targs env pos class_pos tparams targs
              | _ -> ()
            end
          | _ -> ()
        end
      | _ -> ()

    method! at_hint env =
      function
      | (_pos, Aast.Happly ((_, class_id), hints)) ->
        let tc = Env.get_class_or_typedef env class_id in
        begin
          match tc with
          | Some (Env.ClassResult tc) ->
            let tparams = Cls.tparams tc in
            ignore
              (List.iter2 tparams hints ~f:(fun tp hint ->
                   verify_targ_valid env Unresolved tp ((), hint)))
          | _ -> ()
        end
      | _ -> ()

    method! at_tparam env tparam =
      (* Can't use Attributes.mem here because of a conflict between Nast.user_attributes and Tast.user_attributes *)
      if
        List.exists tparam.tp_user_attributes (fun { ua_name; _ } ->
            String.equal UA.uaNewable (snd ua_name))
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
                ~f:(fun t -> not (equal_reify_kind t.tp_reified Erased))
                (Cls.tparams cls)
            then
              Errors.consistent_construct_reified pos
          | _ -> ()
        end
      | None -> ()
  end
