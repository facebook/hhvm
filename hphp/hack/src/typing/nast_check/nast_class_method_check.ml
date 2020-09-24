(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module UA = Naming_special_names.UserAttributes

let is_memoizable user_attributes =
  Naming_attributes.mem2 UA.uaMemoize UA.uaMemoizeLSB user_attributes

let is_overridable user_attributes =
  Naming_attributes.mem UA.uaOverride user_attributes

let error_if_duplicate_method_names methods =
  let _ =
    List.fold_left
      methods
      ~init:SSet.empty
      ~f:(fun seen_methods { m_name = (pos, name); _ } ->
        if SSet.mem name seen_methods then
          Errors.method_name_already_bound pos name;
        SSet.add name seen_methods)
  in
  ()

let error_if_clone_has_arguments method_ =
  match (method_.m_name, method_.m_params) with
  | ((pos, name), _ :: _)
    when String.equal name Naming_special_names.Members.__clone ->
    Errors.clone_too_many_arguments pos
  | _ -> ()

let error_if_abstract_method_is_memoized method_ =
  if method_.m_abstract && is_memoizable method_.m_user_attributes then
    Errors.abstract_method_memoize (fst method_.m_name)

let error_if_private_method_is_overridable class_name method_ =
  if
    Aast.equal_visibility method_.m_visibility Private
    && is_overridable method_.m_user_attributes
  then
    let (pos, id) = method_.m_name in
    Errors.private_override pos class_name id

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ _ class_ =
      error_if_duplicate_method_names class_.c_methods;
      List.iter
        class_.c_methods
        (error_if_private_method_is_overridable (snd class_.c_name));
      ()

    method! at_method_ _ method_ =
      error_if_clone_has_arguments method_;
      error_if_abstract_method_is_memoized method_
  end
