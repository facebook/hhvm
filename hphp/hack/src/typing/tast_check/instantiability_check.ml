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
module SN = Naming_special_names
module Cls = Decl_provider.Class

(* This TAST check raises an error when an abstract final
   class or a trait appears outside of classname<_>. *)

let rec validate_classname env (pos, hint) =
  match hint with
  | Aast.Happly _
  | Aast.Hthis
  | Aast.Hany
  | Aast.Herr
  | Aast.Hmixed
  | Aast.Hwildcard
  | Aast.Hnonnull
  | Aast.Habstr _
  | Aast.Haccess _
  | Aast.Hdynamic
  | Aast.Hsoft _
  | Aast.Hlike _
  | Aast.Hnothing ->
    ()
  | Aast.Hrefinement (h, _) -> validate_classname env h
  | Aast.Hclass_args _ (* TODO: future Hclass will be valid *)
  | Aast.Htuple _
  | Aast.Hunion _
  | Aast.Hintersection _
  | Aast.Hvec_or_dict _
  | Aast.Hprim _
  | Aast.Hoption _
  | Aast.Hfun _
  | Aast.Hshape _
  | Aast.Hfun_context _
  | Aast.Hvar _ ->
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(primary @@ Primary.Invalid_classname pos)

let rec check_hint env (pos, hint) =
  match hint with
  | Aast.Happly ((_, class_id), tal) ->
    begin
      match Tast_env.get_class_or_typedef env class_id with
      | Some (Tast_env.ClassResult cls)
        when let kind = Cls.kind cls in
             let tc_name = Cls.name cls in
             (Ast_defs.is_c_trait kind
             || (Ast_defs.is_c_abstract kind && Cls.final cls))
             && String.( <> ) tc_name SN.Collections.cDict
             && String.( <> ) tc_name SN.Collections.cKeyset
             && String.( <> ) tc_name SN.Collections.cVec ->
        let tc_pos = Cls.pos cls in
        let tc_name = Cls.name cls in
        let err =
          Typing_error.(
            primary
            @@ Primary.Uninstantiable_class
                 {
                   pos;
                   class_name = tc_name;
                   decl_pos = tc_pos;
                   reason_ty_opt = None;
                 })
        in
        Typing_error_utils.add_typing_error
          ~env:(Tast_env.tast_env_as_typing_env env)
          err
      | _ -> ()
    end;
    if String.equal class_id SN.Classes.cClassname then
      Option.iter
        (List.hd tal)
        ~f:(validate_classname (Tast_env.tast_env_as_typing_env env))
    else
      List.iter tal ~f:(check_hint env)
  | Aast.Hrefinement (h, members) ->
    let check_bounds (lower, upper) =
      List.iter lower ~f:(check_hint env);
      List.iter upper ~f:(check_hint env)
    in
    let check_member = function
      | Aast.Rtype (_, ref) ->
        (match ref with
        | Aast.TRexact h -> check_hint env h
        | Aast.TRloose { Aast.tr_lower; tr_upper } ->
          check_bounds (tr_lower, tr_upper))
      | Aast.Rctx (_, ref) ->
        (match ref with
        | Aast.CRexact h -> check_hint env h
        | Aast.CRloose { Aast.cr_lower; cr_upper } ->
          check_bounds (Option.to_list cr_lower, Option.to_list cr_upper))
    in
    List.iter members ~f:check_member;
    check_hint env h
  | Aast.Hshape hm -> check_shape env hm
  | Aast.Haccess (h, ids) -> check_access env h ids
  | Aast.Hvec_or_dict (hopt1, h2) ->
    Option.iter hopt1 ~f:(check_hint env);
    check_hint env h2
  | Aast.Hclass_args h ->
    check_hint env h;
    validate_classname (Tast_env.tast_env_as_typing_env env) h
  | Aast.Hoption h
  | Aast.Hlike h
  | Aast.Hsoft h ->
    check_hint env h
  | Aast.Habstr _
  | Aast.Hprim _
  | Aast.Hany
  | Aast.Herr
  | Aast.Hdynamic
  | Aast.Hnonnull
  | Aast.Hmixed
  | Aast.Hwildcard
  | Aast.Hthis
  | Aast.Hnothing
  | Aast.Hfun_context _
  | Aast.Hvar _ ->
    ()
  | Aast.Hfun
      Aast.
        {
          hf_is_readonly = _;
          hf_param_tys = hl;
          hf_param_info = _;
          (* TODO: shouldn't we be checking this hint as well? *)
          hf_variadic_ty = _;
          hf_ctxs = _;
          hf_return_ty = h;
          hf_is_readonly_return = _;
        } ->
    List.iter hl ~f:(check_hint env);
    check_hint env h
  | Aast.Htuple hl
  | Aast.Hunion hl
  | Aast.Hintersection hl ->
    List.iter hl ~f:(check_hint env)

and check_shape env Aast.{ nsi_allows_unknown_fields = _; nsi_field_map } =
  List.iter ~f:(fun v -> check_hint env v.Aast.sfi_hint) nsi_field_map

(* Need to skip the root of the Haccess element *)
and check_access env h _ =
  match h with
  | (_, Aast.Happly (_, hl)) -> List.iter hl ~f:(check_hint env)
  | _ -> check_hint env h

let check_tparams env tparams =
  List.iter tparams ~f:(fun t ->
      List.iter t.tp_constraints ~f:(fun (_ck, h) -> check_hint env h))

let check_param env param =
  Option.iter (hint_of_type_hint param.param_type_hint) ~f:(check_hint env)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_typedef env t =
      check_hint env t.t_kind;
      Option.iter t.t_as_constraint ~f:(check_hint env);
      Option.iter t.t_super_constraint ~f:(check_hint env)

    method! at_class_ env c =
      let check_class_vars cvar =
        Option.iter (hint_of_type_hint cvar.cv_type) ~f:(check_hint env)
      in
      List.iter c.c_vars ~f:check_class_vars;
      check_tparams env c.c_tparams

    method! at_fun_def env fd = check_tparams env fd.fd_tparams

    method! at_fun_ env f =
      List.iter f.f_params ~f:(check_param env);
      Option.iter (hint_of_type_hint f.f_ret) ~f:(check_hint env)

    method! at_method_ env m =
      check_tparams env m.m_tparams;
      List.iter m.m_params ~f:(check_param env);
      Option.iter (hint_of_type_hint m.m_ret) ~f:(check_hint env)

    method! at_hint env (_, h) =
      match h with
      | Aast.Hshape hm -> check_shape env hm
      | _ -> ()

    method! at_gconst env cst = Option.iter cst.cst_type ~f:(check_hint env)

    method! at_expr env (_, _, e) =
      match e with
      | Is (_, h) -> check_hint env h
      | As { hint = h; _ } -> check_hint env h
      | _ -> ()
  end
