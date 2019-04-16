(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Tast
open Typing_defs

let handler = object
  inherit Tast_visitor.handler_base

  method! at_class_typeconst env { c_tconst_name = (_, name); c_tconst_type; _ } =
    let open Option in
    let t = Tast_env.get_self_id env >>=
      Tast_env.get_class env >>=
      (fun cls -> Typing_classes_heap.get_typeconst cls name) in
    match t with
    | Some { ttc_enforceable = (pos, enforceable); _ } ->
      (* using c_tconst_type here instead of ttc_type because the Type_test_hint_check works on
       * hints instead of decl tys *)
      begin match c_tconst_type with
      | Some h when enforceable ->
        Type_test_hint_check.validate_hint env h
          (Errors.invalid_enforceable_type "constant" (pos, name))
      | _ -> () end
    | _ ->
      ()
  end
