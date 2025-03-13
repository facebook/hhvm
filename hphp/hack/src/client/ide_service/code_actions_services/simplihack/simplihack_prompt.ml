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

let derive_prompt ctx ua method_name () =
  Simplihack_interpreter.eval ctx ua method_name

let find ctx tast =
  let user_attribute edit_span method_name acc ua =
    let attribute_name = snd ua.Aast.ua_name in
    if
      String.equal
        attribute_name
        Naming_special_names.UserAttributes.uaSimpliHack
    then
      let attribute_pos = fst ua.Aast.ua_name in
      {
        attribute_pos;
        edit_span;
        derive_prompt = derive_prompt ctx ua method_name;
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
          ~f:(user_attribute cls.Aast.c_span "onClass")
          cls.Aast.c_user_attributes

      method! on_method_ env meth =
        let acc = super#on_method_ env meth in
        List.fold
          ~init:acc
          ~f:(user_attribute meth.Aast.m_span "onMethod")
          meth.Aast.m_user_attributes

      method! on_fun_ env func =
        let acc = super#on_fun_ env func in
        List.fold
          ~init:acc
          ~f:(user_attribute func.Aast.f_span "onFunction")
          func.Aast.f_user_attributes

      method! on_class_var env cv =
        let acc = super#on_class_var env cv in
        List.fold
          ~init:acc
          ~f:(user_attribute cv.Aast.cv_span "onField")
          cv.Aast.cv_user_attributes

      method! on_typedef env x =
        let acc = super#on_typedef env x in
        List.fold
          ~init:acc
          ~f:(user_attribute x.Aast.t_span "onTypeDef")
          x.Aast.t_user_attributes

      method! on_class_const env x =
        let acc = super#on_class_const env x in
        List.fold
          ~init:acc
          ~f:(user_attribute x.Aast.cc_span "onClassConstant")
          x.Aast.cc_user_attributes

      method! on_fun_param env x =
        let acc = super#on_fun_param env x in
        match x.Aast.param_visibility with
        | None -> acc
        | Some _ ->
          List.fold
            ~init:acc
            ~f:(user_attribute x.Aast.param_pos "onParameter")
            x.Aast.param_user_attributes

      method! on_file_attribute env x =
        let acc = super#on_file_attribute env x in
        List.fold
          ~init:acc
          ~f:(user_attribute Pos.none "onFile")
          x.Aast.fa_user_attributes

      method! on_tparam env x =
        let acc = super#on_tparam env x in
        List.fold
          ~init:acc
          ~f:(user_attribute (fst x.Aast.tp_name) "onGeneric")
          x.Aast.tp_user_attributes

      method! on_class_typeconst_def env x =
        let acc = super#on_class_typeconst_def env x in
        List.fold
          ~init:acc
          ~f:(user_attribute x.Aast.c_tconst_span "onTypeConstant")
          x.Aast.c_tconst_user_attributes

      method! on_module_def env x =
        let acc = super#on_module_def env x in
        List.fold
          ~init:acc
          ~f:(user_attribute x.Aast.md_span "onModule")
          x.Aast.md_user_attributes
    end
  in
  visitor#go ctx tast
