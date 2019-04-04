(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast
open Utils
open Nast_check_env

module SN = Naming_special_names

let is_magic =
  let h = Caml.Hashtbl.create 23 in
  let a x = Caml.Hashtbl.add h x true in
  let _ = SSet.iter (fun m -> if m <> SN.Members.__toString then a m) SN.Members.as_set in
  fun (_, s) ->
    Caml.Hashtbl.mem h s

let handler = object
  inherit Nast_visitor.handler_base

  method! at_expr env (pos, e) =
    match e with
    | Binop ((Ast.Eq None), e1, e2) ->
      begin match e1, e2 with
      | (_, (Lvar (_, x))), (_, Unop (Ast.Uref, _))
       when Local_id.to_string x |> SN.Superglobals.is_superglobal ->
        Errors.illegal_by_ref_expr pos ("Superglobal " ^ Local_id.to_string x) "bound"
      | _ -> ()
      end;
    | Unop (Ast.Uref, e) ->
      let ref_expr ident = Errors.illegal_by_ref_expr pos ident "passed" in
      begin match snd e with
      | Lvar (_, x) when Local_id.to_string x = SN.SpecialIdents.this ->
        ref_expr SN.SpecialIdents.this
      | Lvar (_, x) when Local_id.to_string x = SN.SpecialIdents.dollardollar ->
        ref_expr SN.SpecialIdents.dollardollar
      | _ -> ()
      end
    | Id (pos, const) ->
      let const = add_ns const in
      let ck = env.class_kind in
      if not (SN.PseudoConsts.is_pseudo_const const)
      then ()
      else if const = SN.PseudoConsts.g__CLASS__ && ck = None
      then Errors.illegal_CLASS pos
      else if const = SN.PseudoConsts.g__TRAIT__ && ck <> Some Ast.Ctrait
      then Errors.illegal_TRAIT pos
    | InstanceOf (_, e) ->
      begin match snd e with
      | CIexpr (_, Class_const ((_, CIexpr (_, Id(_, classname))), (p, "class"))) ->
        Errors.classname_const_instanceof (Utils.strip_ns classname) p;
      | _ -> ()
      end;
    | Class_const ((_, CIexpr (_, (Id(_, "parent")))), (_, m_name))
      when env.function_name = Some m_name -> ()
    | Class_const (_, ((_, m_name) as mid))
      when is_magic mid && env.function_name <> Some m_name ->
      Errors.magic mid;
    | Obj_get (_, (_, Id s), _) when is_magic s -> Errors.magic s
    | _ -> ()

  method! at_fun_ _ f =
    let pos, fname = f.f_name in
    let fname_lower = String.lowercase (strip_ns fname) in
    if fname_lower = SN.Members.__construct || fname_lower = "using"
    then Errors.illegal_function_name pos fname

  method! at_method_ env m =
    let pos, name = m.m_name in
    if name = SN.Members.__destruct
      && not (Attributes.mem SN.UserAttributes.uaOptionalDestruct m.m_user_attributes)
    then Errors.illegal_destructor pos;
    begin match env.class_name with
    | Some cname ->
        let p, mname = m.m_name in
        if String.lowercase (strip_ns cname) = String.lowercase mname
            && env.class_kind <> Some Ast.Ctrait
        then Errors.dangerous_method_name p
    | None -> assert false
    end

end
