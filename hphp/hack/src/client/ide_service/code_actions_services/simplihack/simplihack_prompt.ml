(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = {
  attribute_pos: Pos.t;
  derive_prompt: unit -> string option;
  edit_span: Pos.t;
}

let derive_prompt ctx ua () =
  let open Option.Let_syntax in
  match ua.Aast.ua_params with
  | (_, _, Aast.Class_const ((_, _p, Aast.CI cid), (_, x))) :: _args
    when String.equal x Naming_special_names.Members.mClass ->
    let* path = Naming_provider.get_class_path ctx (snd cid) in
    let* ast = Ast_provider.find_class_in_file ~full:true ctx path (snd cid) in
    let* meth =
      List.find ast.Aast.c_methods ~f:(fun meth ->
          String.equal (snd meth.Aast.m_name) "onClass")
    in
    let { Aast.fb_ast } = meth.Aast.m_body in
    let* prompt =
      match fb_ast with
      | (_, Aast.Return (Some (_, _, Aast.String prompt))) :: _ -> Some prompt
      | _ -> None
    in
    return prompt
  | _ -> None

let find ctx tast =
  let user_attribute cls_pos acc ua =
    let attribute_name = snd ua.Aast.ua_name in
    if
      String.equal
        attribute_name
        Naming_special_names.UserAttributes.uaSimpliHack
    then
      let attribute_pos = fst ua.Aast.ua_name in
      {
        attribute_pos;
        edit_span = cls_pos;
        derive_prompt = derive_prompt ctx ua;
      }
      :: acc
    else
      acc
  in
  let visitor =
    object
      inherit [_] Tast_visitor.reduce as super

      method zero = []

      method plus = ( @ )

      method! on_class_ env cls =
        let acc = super#on_class_ env cls in
        List.fold
          ~init:acc
          ~f:(user_attribute cls.Aast.c_span)
          cls.Aast.c_user_attributes
    end
  in
  visitor#go ctx tast
