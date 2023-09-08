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
open Typing_reified_check (* validator *)
module Env = Tast_env
module SN = Naming_special_names
module UA = SN.UserAttributes
module Cls = Decl_provider.Class
module Nast = Aast

let is_reified tparam = not (equal_reify_kind tparam.tp_reified Erased)

let tparams_has_reified tparams = List.exists tparams ~f:is_reified

let valid_newable_hint env (tp_pos, tp_name) (pos, hint) =
  let err_opt =
    let open Typing_error.Primary in
    match hint with
    | Aast.Happly ((p, h), _) -> begin
      match Env.get_class_or_typedef env h with
      | Some (Env.ClassResult cls) ->
        if not Ast_defs.(is_c_normal (Cls.kind cls)) then
          Some (Invalid_newable_type_argument { tp_pos; tp_name; pos = p })
        else
          None
      | _ ->
        (* This case should never happen *)
        Some (Invalid_newable_type_argument { tp_pos; tp_name; pos = p })
    end
    | Aast.Habstr (name, []) ->
      if not @@ Env.get_newable env name then
        Some (Invalid_newable_type_argument { tp_pos; tp_name; pos })
      else
        None
    | _ -> Some (Invalid_newable_type_argument { tp_pos; tp_name; pos })
  in
  Option.iter err_opt ~f:(fun err ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
      @@ Typing_error.primary err)

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
  let valid_classes = List.filter bound_classes ~f:Cls.valid_newable_class in
  if Int.( <> ) 1 (List.length valid_classes) then
    let constraints = List.map ~f:Cls.name valid_classes in
    let (pos, tp_name) = tparam.tp_name in
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(
        primary
        @@ Primary.Invalid_newable_typaram_constraints
             { pos; tp_name; constraints })

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
  (if is_reified tparam then
    match tparam.tp_reified with
    | Nast.Reified
    | Nast.SoftReified ->
      let (decl_pos, param_name) = tparam.tp_name in
      let emit_error pos arg_info =
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            primary
            @@ Primary.Invalid_reified_arg
                 { pos; param_name; decl_pos; arg_info })
      in
      validator#validate_hint
        (Tast_env.tast_env_as_typing_env env)
        (snd targ)
        ~reification
        emit_error
    | Nast.Erased -> ());

  if Attributes.mem UA.uaEnforceable tparam.tp_user_attributes then
    Typing_enforceable_hint.validate_hint
      (Tast_env.tast_env_as_typing_env env)
      (snd targ)
      (fun pos ty_info ->
        let (tp_pos, tp_name) = tparam.tp_name in
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            primary
            @@ Primary.Invalid_enforceable_type
                 { kind = `param; pos; ty_info; tp_pos; tp_name }));

  if Attributes.mem UA.uaNewable tparam.tp_user_attributes then
    valid_newable_hint env tparam.tp_name (snd targ)

let verify_call_targs env expr_pos decl_pos tparams targs =
  (if tparams_has_reified tparams then
    let tparams_length = List.length tparams in
    let targs_length = List.length targs in
    if Int.( <> ) tparams_length targs_length then
      if Int.( = ) targs_length 0 then
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          Typing_error.(
            primary @@ Primary.Require_args_reify { decl_pos; pos = expr_pos })
      else
        (* mismatches with targs_length > 0 are not specific to reification and handled
                  elsewhere *)
        ());
  let all_wildcards =
    List.for_all ~f:(fun (_, h) -> Aast_defs.is_wildcard_hint h) targs
  in
  if all_wildcards && tparams_has_reified tparams then
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(
        primary @@ Primary.Require_args_reify { decl_pos; pos = expr_pos })
  else
    (* Unequal_lengths case handled elsewhere *)
    List.iter2 tparams targs ~f:(verify_targ_valid env Type_validator.Resolved)
    |> ignore

let rec get_ft_tparams env fun_ty =
  let fun_ty = Tast_env.strip_dynamic env fun_ty in
  match get_node fun_ty with
  | Tnewtype (name, _, ty1) when String.equal name SN.Classes.cSupportDyn ->
    get_ft_tparams env ty1
  | Tfun ({ ft_tparams; _ } as fun_ty) -> Some (ft_tparams, fun_ty)
  | _ -> None

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env x =
      (* only considering functions where one or more params are reified *)
      match x with
      | (_, call_pos, Class_get ((_, _, CI (_, t)), _, _)) ->
        if equal_reify_kind (Env.get_reified env t) Reified then
          Typing_error_utils.add_typing_error
            ~env:(Tast_env.tast_env_as_typing_env env)
            Typing_error.(primary @@ Primary.Class_get_reified call_pos)
      | (fun_ty, pos, Method_caller _) ->
        (match get_ft_tparams env fun_ty with
        | Some (ft_tparams, _) ->
          if tparams_has_reified ft_tparams then
            Typing_error_utils.add_typing_error
              ~env:(Tast_env.tast_env_as_typing_env env)
              Typing_error.(primary @@ Primary.Reified_function_reference pos)
        | None -> ())
      | ( _,
          pos,
          FunctionPointer (FP_class_const ((ty, _, CI (_, class_id)), _), _) )
        when Env.is_in_expr_tree env ->
        let (_env, ty) = Env.expand_type env ty in
        begin
          match get_node ty with
          (* If we get Tgeneric here, the underlying type was reified *)
          | Tgeneric (ci, _) when String.equal ci class_id ->
            Typing_error_utils.add_typing_error
              ~env:(Tast_env.tast_env_as_typing_env env)
              Typing_error.(
                expr_tree
                @@ Primary.Expr_tree.Reified_static_method_in_expr_tree pos)
          | _ -> ()
        end
      | (fun_ty, pos, FunctionPointer (_, targs)) -> begin
        match get_ft_tparams env fun_ty with
        | Some (ft_tparams, _) ->
          verify_call_targs env pos (get_pos fun_ty) ft_tparams targs
        | None -> ()
      end
      | (_, pos, Call { func = (fun_ty, _, _); targs; _ }) ->
        let (env, efun_ty) = Env.expand_type env fun_ty in
        (match get_ft_tparams env efun_ty with
        | Some (ft_tparams, ty) when not @@ get_ft_is_function_pointer ty ->
          verify_call_targs env pos (get_pos efun_ty) ft_tparams targs
        | _ -> ())
      | (_, pos, New ((ty, _, CI (_, class_id)), targs, _, _, _)) ->
        let (env, ty) = Env.expand_type env ty in
        (match get_node ty with
        | Tgeneric (ci, _tyargs) when String.equal ci class_id ->
          (* ignoring type arguments here: If we get a Tgeneric here, the underlying type
             parameter must have been newable and reified, neither of which his allowed for
             higher-kinded type-parameters *)
          if not (Env.get_newable env ci) then
            Typing_error_utils.add_typing_error
              ~env:(Tast_env.tast_env_as_typing_env env)
              Typing_error.(
                primary @@ Primary.New_without_newable { pos; name = ci })
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
      | ( _,
          pos,
          New ((_, _, ((CIstatic | CIself | CIparent) as cid)), _, _, _, _) ) ->
        Option.(
          let t =
            Env.get_self_id env
            >>= Env.get_class env
            >>| Cls.tparams
            >>| tparams_has_reified
          in
          Option.iter t ~f:(fun has_reified ->
              if has_reified then
                let (class_kind, suggested_class_name) =
                  match cid with
                  | CIstatic -> ("static", None)
                  | CIself -> ("self", Env.get_self_id env)
                  | CIparent -> ("parent", Env.get_parent_id env)
                  | _ -> failwith "Unexpected match"
                in
                Typing_error_utils.add_typing_error
                  ~env:(Tast_env.tast_env_as_typing_env env)
                  Typing_error.(
                    primary
                    @@ Primary.New_class_reified
                         { pos; class_kind; suggested_class_name })))
      | (_, pos, New ((ty, _, _), targs, _, _, _)) ->
        let (env, ty) = Env.expand_type env ty in
        begin
          match get_node ty with
          | Tclass ((_, cid), _, _) -> begin
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
                   verify_targ_valid env Type_validator.Unresolved tp ((), hint)))
          | _ -> ()
        end
      | _ -> ()

    method! at_tparam env tparam =
      (* Can't use Attributes.mem here because of a conflict between Nast.user_attributes and Tast.user_attributes *)
      if
        List.exists tparam.tp_user_attributes ~f:(fun { ua_name; _ } ->
            String.equal UA.uaNewable (snd ua_name))
      then
        verify_has_consistent_bound env tparam

    method! at_class_ env { c_name = (pos, name); _ } =
      match Env.get_class env name with
      | Some cls -> begin
        match Cls.construct cls with
        | (_, Typing_defs.ConsistentConstruct) ->
          if
            List.exists
              ~f:(fun t -> not (equal_reify_kind t.tp_reified Erased))
              (Cls.tparams cls)
          then
            Typing_error_utils.add_typing_error
              ~env:(Tast_env.tast_env_as_typing_env env)
              Typing_error.(primary @@ Primary.Consistent_construct_reified pos)
        | _ -> ()
      end
      | None -> ()
  end
