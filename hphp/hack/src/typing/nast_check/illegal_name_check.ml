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

module SN = Naming_special_names

let handler = object
  inherit Nast_visitor.handler_base

  method! at_expr env (pos, e) =
    match e with
    | Unop (Ast.Uref, e) ->
      begin match snd e with
      | Lvar (_, x) when Local_id.to_string x = SN.SpecialIdents.this ->
        Errors.illegal_by_ref_expr pos SN.SpecialIdents.this
      | Lvar (_, x) when Local_id.to_string x = SN.SpecialIdents.dollardollar ->
        Errors.illegal_by_ref_expr pos SN.SpecialIdents.dollardollar
      | _ -> ()
      end
    | Id (pos, const) ->
      let const = add_ns const in
      let ck = env.Nast_visitor.class_kind in
      if not (SN.PseudoConsts.is_pseudo_const const)
      then ()
      else if const = SN.PseudoConsts.g__CLASS__ && ck = None
      then Errors.illegal_CLASS pos
      else if const = SN.PseudoConsts.g__TRAIT__ && ck <> Some Ast.Ctrait
      then Errors.illegal_TRAIT pos
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
    begin match env.Nast_visitor.class_name with
    | Some cname ->
        let p, mname = m.m_name in
        if String.lowercase (strip_ns cname) = String.lowercase mname
            && env.Nast_visitor.class_kind <> Some Ast.Ctrait
        then Errors.dangerous_method_name p
    | None -> assert false
    end

end
