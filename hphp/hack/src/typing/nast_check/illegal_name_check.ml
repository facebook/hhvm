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
open Utils
open Nast_check_env
module SN = Naming_special_names

let is_magic =
  let h = Caml.Hashtbl.create 23 in
  let a x = Caml.Hashtbl.add h x true in
  let _ =
    SSet.iter
      (fun m -> if m <> SN.Members.__toString then a m)
      SN.Members.as_set
  in
  (fun (_, s) -> Caml.Hashtbl.mem h s)

(* Class consts and typeconsts cannot be named "class" *)
let error_if_is_named_class (pos, name) =
  if String.lowercase name = "class" then
    Errors.illegal_member_variable_class pos

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ _ c =
      List.iter c.c_typeconsts ~f:(fun tc ->
          error_if_is_named_class tc.c_tconst_name);
      List.iter c.c_consts ~f:(fun cc -> error_if_is_named_class cc.cc_id)

    method! at_expr env (pos, e) =
      match e with
      | Binop (Ast_defs.Eq None, e1, e2) ->
        begin
          match (e1, e2) with
          | ((_, Lvar (_, x)), (_, Unop (Ast_defs.Uref, _)))
            when Local_id.to_string x |> SN.Superglobals.is_superglobal
                 || SN.Superglobals.globals = Local_id.to_string x ->
            Errors.illegal_by_ref_expr
              pos
              ("Superglobal " ^ Local_id.to_string x)
              "bound"
          | _ -> ()
        end
      | Unop (Ast_defs.Uref, e) ->
        let ref_expr ident = Errors.illegal_by_ref_expr pos ident "passed" in
        begin
          match snd e with
          | Lvar (_, x) when Local_id.to_string x = SN.SpecialIdents.this ->
            ref_expr SN.SpecialIdents.this
          | Lvar (_, x)
            when Local_id.to_string x = SN.SpecialIdents.dollardollar ->
            ref_expr SN.SpecialIdents.dollardollar
          | _ -> ()
        end
      | Id (pos, const) ->
        let const = add_ns const in
        let ck = env.class_kind in
        if not (SN.PseudoConsts.is_pseudo_const const) then
          ()
        else if const = SN.PseudoConsts.g__CLASS__ && ck = None then
          Errors.illegal_CLASS pos
        else if
          const = SN.PseudoConsts.g__TRAIT__ && ck <> Some Ast_defs.Ctrait
        then
          Errors.illegal_TRAIT pos
      | Class_const ((_, CIexpr (_, Id (_, "parent"))), (_, m_name))
        when env.function_name = Some m_name ->
        ()
      | Class_const (_, ((_, m_name) as mid))
        when is_magic mid && env.function_name <> Some m_name ->
        Errors.magic mid
      | Obj_get (_, (_, Id s), _) when is_magic s -> Errors.magic s
      | _ -> ()

    method! at_fun_ _ f =
      let (pos, fname) = f.f_name in
      let fname_lower = String.lowercase (strip_ns fname) in
      if fname_lower = SN.Members.__construct || fname_lower = "using" then
        Errors.illegal_function_name pos fname

    method! at_method_ env m =
      let (pos, name) = m.m_name in
      if name = SN.Members.__destruct then Errors.illegal_destructor pos;
      match env.class_name with
      | Some _ -> ()
      | None -> assert false
  end
