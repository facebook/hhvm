(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast

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

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ _ class_ =
      error_if_duplicate_method_names class_.c_methods;
      ()
  end
