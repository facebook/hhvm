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
module Env = Tast_env
module SN = Naming_special_names
module UA = SN.UserAttributes
module Cls = Decl_provider.Class
module Nast = Aast

let validator =
  object (this)
    inherit type_validator as super

    method! on_tapply acc r (p, h) tyl =
      if String.equal h SN.Classes.cTypename then
        this#invalid acc r "a typename"
      else if String.equal h SN.Classes.cClassname then
        this#invalid acc r "a classname"
      else if
        String.equal h SN.Typehints.wildcard
        && not (Env.get_allow_wildcards acc.env)
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

    method! on_tvarray_or_darray acc r _ _ = this#invalid acc r "an array type"

    method! on_tfun acc r _ = this#invalid acc r "a function type"

    method! on_typeconst acc is_concrete typeconst =
      match typeconst.ttc_abstract with
      | _ when Option.is_some typeconst.ttc_reifiable || is_concrete ->
        super#on_typeconst acc is_concrete typeconst
      | _ ->
        let r = Reason.Rwitness (fst typeconst.ttc_name) in
        let kind =
          "an abstract type constant without the __Reifiable attribute"
        in
        this#invalid acc r kind

    method! on_taccess acc r (root, ids) =
      let acc =
        match acc.reification with
        | Unresolved -> this#on_type acc root
        | Resolved -> acc
      in
      super#on_taccess acc r (root, ids)

    method! on_tthis acc r =
      this#invalid acc r "the late static bound this type"
  end

let is_reified tparam = not (equal_reify_kind tparam.tp_reified Erased)

let tparams_has_reified tparams = List.exists tparams ~f:is_reified

let valid_newable_hint env tp (pos, hint) =
  match hint with
  | Aast.Happly ((p, h), _) ->
    begin
      match Env.get_class env h with
      | Some cls ->
        if not Ast_defs.(equal_class_kind (Cls.kind cls) Cnormal) then
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
let verify_targ_valid env reification tparam ((_, hint) as targ) =
  if
    Naming_attributes.mem UA.uaExplicit tparam.tp_user_attributes
    && is_wildcard targ
  then
    Errors.require_generic_explicit tparam.tp_name (fst hint);

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

  if Naming_attributes.mem UA.uaEnforceable tparam.tp_user_attributes then
    Enforceable_hint_check.validator#validate_hint
      env
      (snd targ)
      (Errors.invalid_enforceable_type "parameter" tparam.tp_name);

  if Naming_attributes.mem UA.uaNewable tparam.tp_user_attributes then
    valid_newable_hint env tparam.tp_name (snd targ)

let verify_call_targs env expr_pos decl_pos tparams targs =
  ( if tparams_has_reified tparams then
    let check_targ_hints = function
      | (_, (pos, Aast.Happly ((_, class_id), hints))) ->
        let tc = Env.get_class env class_id in
        Option.iter tc ~f:(fun tc ->
            let tparams = Cls.tparams tc in
            let tparams_length = List.length tparams in
            let targs_length = List.length hints in
            if Int.( <> ) tparams_length targs_length then
              let c_pos = Cls.pos tc in
              if Int.( <> ) targs_length 0 then
                Errors.type_arity
                  pos
                  c_pos
                  ~expected:tparams_length
                  ~actual:targs_length
              else
                Errors.require_args_reify c_pos pos)
      | _ -> ()
    in
    List.iter targs ~f:check_targ_hints );
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
      | ((call_pos, _), Class_get ((_, CI (_, t)), _)) ->
        if equal_reify_kind (Env.get_reified env t) Reified then
          Errors.class_get_reified call_pos
      (* Call of the form C::f where f is a static method:
       * error when f uses any C's reified generic *)
      | ( (call_pos, _),
          Call
            ( _,
              ((_, fun_ty), Class_const ((_, CI (_, class_id)), (_, fname))),
              targs,
              _,
              _ ) ) ->
        begin
          match get_node fun_ty with
          | Tfun { ft_tparams; _ } ->
            (match Env.get_class env class_id with
            | Some cls ->
              let tp_names =
                List.filter_map (Cls.tparams cls) (function tp ->
                    if is_reified tp then
                      Some tp.tp_name
                    else
                      None)
              in
              (match Cls.get_smethod cls fname with
              | Some ce_m ->
                (match
                   Env.get_class env ce_m.ce_origin
                   |> Option.bind ~f:(fun cls -> Cls.get_smethod cls fname)
                 with
                | Some m ->
                  let check_type ty =
                    let check_names t_pos t =
                      List.iter tp_names ~f:(function (_, name) ->
                          if String.equal name t then
                            Errors.static_call_with_class_level_reified_generic
                              call_pos
                              t_pos)
                    in
                    match Typing_defs.deref ty with
                    | (r, Tgeneric t) -> check_names (Reason.to_pos r) t
                    | (_, Tapply ((t_pos, t), _)) -> check_names t_pos t
                    | _ -> ()
                  in
                  (match Typing_defs.deref (Lazy.force m.ce_type) with
                  | (_, Tfun ft) ->
                    List.iter ft.ft_params ~f:(fun param ->
                        check_type param.fp_type.et_type);
                    check_type ft.ft_ret.et_type
                  | _ ->
                    failwith
                      "Expected Tfun type in result of Typing_classes_heap.get_smethod")
                | None -> ())
              | _ -> ())
            | None -> ());
            verify_call_targs env call_pos (get_pos fun_ty) ft_tparams targs
          | _ -> ()
        end
      | ((pos, fun_ty), FunctionPointer (_, _targs)) ->
        begin
          match get_node fun_ty with
          | Tfun { ft_tparams; _ } ->
            (* Once we support reified generics in function pointers,
             * we can let this case fall through to the next *)
            if tparams_has_reified ft_tparams then
              Errors.reified_generics_not_allowed pos
          | _ -> ()
        end
      | ((pos, _), Call (_, ((_, fun_ty), _), targs, _, _)) ->
        begin
          match get_node fun_ty with
          | Tfun { ft_tparams; _ } ->
            verify_call_targs env pos (get_pos fun_ty) ft_tparams targs
          | _ -> ()
        end
      | ((pos, _), New (((_, ty), CI (_, class_id)), targs, _, _, _)) ->
        let (env, ty) = Env.expand_type env ty in
        (match get_node ty with
        | Tgeneric ci when String.equal ci class_id ->
          if not (Env.get_newable env ci) then Errors.new_without_newable pos ci;
          if not (List.is_empty targs) then Errors.tparam_with_tparam pos ci
        | _ ->
          (match Env.get_class env class_id with
          | Some cls ->
            let tparams = Cls.tparams cls in
            let class_pos = Cls.pos cls in
            verify_call_targs env pos class_pos tparams targs
          | None -> ()))
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
      | (pos, Aast.Happly ((_, class_id), hints)) ->
        let tc = Env.get_class env class_id in
        Option.iter tc ~f:(fun tc ->
            let tparams = Cls.tparams tc in
            ignore
              (List.iter2 tparams hints ~f:(fun tp hint ->
                   verify_targ_valid env Unresolved tp ((), hint)));

            (* TODO: This check could be unified with the existence check above,
             * but would require some consolidation T38941033. List.iter2 gives
             * a nice Or_unequal_lengths.t result that replaces this if statement *)
            let tparams_length = List.length tparams in
            let targs_length = List.length hints in
            if Int.( <> ) tparams_length targs_length then
              let c_pos = Cls.pos tc in
              if Int.( <> ) targs_length 0 then
                Errors.type_arity
                  pos
                  c_pos
                  ~expected:tparams_length
                  ~actual:targs_length
              else if tparams_has_reified tparams then
                Errors.require_args_reify c_pos pos)
      | _ -> ()

    method! at_tparam env tparam =
      (* Can't use Naming_attributes.mem here because of a conflict between Nast.user_attributes and Tast.user_attributes *)
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
