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
open Nast_check_env
module UA = Naming_special_names.UserAttributes

let is_mutable user_attributes =
  Naming_attributes.mem UA.uaMutable user_attributes

let is_maybe_mutable user_attributes =
  Naming_attributes.mem UA.uaMaybeMutable user_attributes

let is_mutable_return user_attributes =
  Naming_attributes.mem UA.uaMutableReturn user_attributes

let check_mutability pos ua name =
  if is_mutable ua then Errors.mutable_methods_must_be_reactive pos name;
  if is_maybe_mutable ua then
    Errors.maybe_mutable_methods_must_be_reactive pos name

let check_param is_reactive name param =
  let ua = param.param_user_attributes in
  if not is_reactive then check_mutability param.param_pos ua name

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ env f =
      let (pos, fname) = f.f_name in
      let ua = f.f_user_attributes in
      (* Functions can't be mutable, only methods can *)
      if is_mutable ua then Errors.mutable_attribute_on_function pos;
      if is_maybe_mutable ua then Errors.maybe_mutable_attribute_on_function pos;
      if is_mutable_return ua && not env.is_reactive then
        Errors.mutable_return_annotated_decls_must_be_reactive
          "function"
          pos
          fname;
      List.iter f.f_params (check_param env.is_reactive fname)

    method! at_method_ _env m =
      let (pos, name) = m.m_name in
      let ua = m.m_user_attributes in
      let is_reactive = fun_is_reactive ua in
      if not is_reactive then check_mutability pos ua name;
      if is_mutable ua && is_maybe_mutable ua then
        Errors.conflicting_mutable_and_maybe_mutable_attributes pos;

      (* Methods annotated with MutableReturn attribute must be reactive *)
      if is_mutable_return ua && not is_reactive then
        Errors.mutable_return_annotated_decls_must_be_reactive "method" pos name;
      List.iter m.m_params (check_param is_reactive name)
  end
