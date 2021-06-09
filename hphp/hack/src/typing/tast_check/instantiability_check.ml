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
module ShapeMap = Aast.ShapeMap
module SN = Naming_special_names
module Cls = Decl_provider.Class

let validate_classname (pos, hint) =
  match hint with
  | Aast.Happly _
  | Aast.Hthis
  | Aast.Hany
  | Aast.Herr
  | Aast.Hmixed
  | Aast.Hnonnull
  | Aast.Habstr _
  | Aast.Haccess _
  | Aast.Hdynamic
  | Aast.Hsoft _
  | Aast.Hlike _
  | Aast.Hnothing ->
    ()
  | Aast.Htuple _
  | Aast.Hunion _
  | Aast.Hintersection _
  | Aast.Hdarray _
  | Aast.Hvarray _
  | Aast.Hvarray_or_darray _
  | Aast.Hvec_or_dict _
  | Aast.Hprim _
  | Aast.Hoption _
  | Aast.Hfun _
  | Aast.Hshape _
  | Aast.Hfun_context _
  | Aast.Hvar _ ->
    Errors.invalid_classname pos

let rec check_hint env (pos, hint) =
  match hint with
  | Aast.Happly ((_, class_id), tal) ->
    begin
      match Tast_env.get_class_or_typedef env class_id with
      | Some (Tast_env.ClassResult cls)
        when let kind = Cls.kind cls in
             let tc_name = Cls.name cls in
             ( Ast_defs.(equal_class_kind kind Ctrait)
             || (Ast_defs.(equal_class_kind kind Cabstract) && Cls.final cls) )
             && String.( <> ) tc_name SN.Collections.cDict
             && String.( <> ) tc_name SN.Collections.cKeyset
             && String.( <> ) tc_name SN.Collections.cVec ->
        let tc_pos = Cls.pos cls in
        let tc_name = Cls.name cls in
        Errors.uninstantiable_class pos tc_pos tc_name None
      | _ -> ()
    end;
    if String.equal class_id SN.Classes.cClassname then
      Option.iter (List.hd tal) ~f:validate_classname
    else
      List.iter tal ~f:(check_hint env)
  | Aast.Hdarray (h1, h2) ->
    check_hint env h1;
    check_hint env h2
  | Aast.Hshape hm -> check_shape env hm
  | Aast.Haccess (h, ids) -> check_access env h ids
  | Aast.Hvec_or_dict (hopt1, h2)
  | Aast.Hvarray_or_darray (hopt1, h2) ->
    Option.iter hopt1 ~f:(check_hint env);
    check_hint env h2
  | Aast.Hvarray h
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

let check_variadic_param env param =
  match param with
  | FVvariadicArg vparam -> check_param env vparam
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_typedef env t =
      check_hint env t.t_kind;
      Option.iter t.t_constraint ~f:(check_hint env)

    method! at_class_ env c =
      let check_class_vars cvar =
        Option.iter (hint_of_type_hint cvar.cv_type) ~f:(check_hint env)
      in
      List.iter c.c_vars ~f:check_class_vars;
      check_tparams env c.c_tparams

    method! at_fun_ env f =
      check_tparams env f.f_tparams;
      List.iter f.f_params ~f:(check_param env);
      check_variadic_param env f.f_variadic;
      Option.iter (hint_of_type_hint f.f_ret) ~f:(check_hint env)

    method! at_method_ env m =
      check_tparams env m.m_tparams;
      List.iter m.m_params ~f:(check_param env);
      check_variadic_param env m.m_variadic;
      Option.iter (hint_of_type_hint m.m_ret) ~f:(check_hint env)

    method! at_hint env (_, h) =
      match h with
      | Aast.Hshape hm -> check_shape env hm
      | _ -> ()

    method! at_gconst env cst = Option.iter cst.cst_type ~f:(check_hint env)

    method! at_expr env (_, e) =
      match e with
      | Is (_, h) -> check_hint env h
      | As (_, h, _) -> check_hint env h
      | _ -> ()
  end
