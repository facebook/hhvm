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
open Utils
open Nast_check_env
module SN = Naming_special_names

let is_magic =
  let h = Stdlib.Hashtbl.create 23 in
  let a x = Stdlib.Hashtbl.add h x true in
  let _ =
    SSet.iter
      (fun m -> if String.( <> ) m SN.Members.__toString then a m)
      SN.Members.as_set
  in
  (fun (_, s) -> Stdlib.Hashtbl.mem h s)

(* Class consts and typeconsts cannot be named "class" *)
let error_if_is_named_class (pos, name) custom_err_config =
  if String.equal (String.lowercase name) "class" then
    Diagnostics.add_diagnostic
      (Naming_error_utils.to_user_diagnostic
         (Naming_error.Illegal_member_variable_class pos)
         custom_err_config)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let custom_err_config = Nast_check_env.get_custom_error_config env in
      List.iter c.c_typeconsts ~f:(fun tc ->
          error_if_is_named_class tc.c_tconst_name custom_err_config);
      List.iter c.c_consts ~f:(fun cc ->
          error_if_is_named_class cc.cc_id custom_err_config)

    method! at_expr env (_, _, e) =
      let func_name =
        match env.function_name with
        | None -> None
        | Some sid -> Some (snd sid)
      in
      match e with
      | Id (pos, const) ->
        let custom_err_config = Nast_check_env.get_custom_error_config env in
        let ck = env.classish_kind in
        if not (SN.PseudoConsts.is_pseudo_const const) then
          ()
        else if
          String.equal const SN.PseudoConsts.g__CLASS__ && Option.is_none ck
        then
          Diagnostics.add_diagnostic
            (Naming_error_utils.to_user_diagnostic
               (Naming_error.Illegal_CLASS pos)
               custom_err_config)
        else if
          String.equal const SN.PseudoConsts.g__TRAIT__
          && not
               (Option.equal
                  Ast_defs.equal_classish_kind
                  ck
                  (Some Ast_defs.Ctrait))
        then
          Diagnostics.add_diagnostic
            (Naming_error_utils.to_user_diagnostic
               (Naming_error.Illegal_TRAIT pos)
               custom_err_config)
      | Class_const ((_, _, CIexpr (_, _, Id (_, "parent"))), (_, m_name))
        when Option.equal String.equal func_name (Some m_name) ->
        ()
      | Class_const (_, ((pos, meth_name) as mid))
        when is_magic mid
             && not (Option.equal String.equal func_name (Some meth_name)) ->
        Diagnostics.add_diagnostic
          Nast_check_error.(to_user_diagnostic @@ Magic { pos; meth_name })
      | Obj_get (_, (_, _, Id s), _, _) when is_magic s ->
        let (pos, meth_name) = s in
        Diagnostics.add_diagnostic
          Nast_check_error.(to_user_diagnostic @@ Magic { pos; meth_name })
      | Method_caller (_, meth) when is_magic meth ->
        let (pos, meth_name) = meth in
        Diagnostics.add_diagnostic
          Nast_check_error.(to_user_diagnostic @@ Magic { pos; meth_name })
      | _ -> ()

    method! at_fun_def _ fd =
      let (pos, fname) = fd.fd_name in
      let fname_lower = String.lowercase (strip_ns fname) in
      if
        String.equal fname_lower SN.Members.__construct
        || String.equal fname_lower "using"
      then
        Diagnostics.add_diagnostic
          Nast_check_error.(
            to_user_diagnostic @@ Illegal_function_name { pos; name = fname })

    method! at_method_ env m =
      let (pos, name) = m.m_name in
      if String.equal name SN.Members.__destruct then
        Diagnostics.add_diagnostic
          Nast_check_error.(to_user_diagnostic @@ Illegal_destructor pos);
      match env.class_name with
      | Some _ -> ()
      | None -> assert false
  end
