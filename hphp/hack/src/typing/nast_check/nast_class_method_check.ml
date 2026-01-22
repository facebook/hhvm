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

let error_if_duplicate_method_names methods custom_err_config =
  let _ =
    List.fold_left
      methods
      ~init:SSet.empty
      ~f:(fun seen_methods { m_name = (pos, meth_name); _ } ->
        if SSet.mem meth_name seen_methods then
          Diagnostics.add_diagnostic
            (Naming_error_utils.to_user_diagnostic
               (Naming_error.Method_name_already_bound { pos; meth_name })
               custom_err_config);
        SSet.add meth_name seen_methods)
  in
  ()

let error_if_abstract_method_is_memoized method_ =
  if method_.m_abstract && is_memoizable method_.m_user_attributes then
    Diagnostics.add_diagnostic
      Nast_check_error.(
        to_user_diagnostic @@ Abstract_method_memoize (fst method_.m_name))

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env class_ =
      let custom_err_config = Nast_check_env.get_custom_error_config env in
      error_if_duplicate_method_names class_.c_methods custom_err_config

    method! at_method_ _ method_ = error_if_abstract_method_is_memoized method_
  end
