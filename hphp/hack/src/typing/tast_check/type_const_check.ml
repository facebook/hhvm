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
module Cls = Decl_provider.Class

let handler = object
  inherit Tast_visitor.handler_base

  method! at_class_typeconst env { c_tconst_abstract; c_tconst_name = (p, name); _ } =
    let open Option in
    let cls_opt = Tast_env.get_self_id env >>=
      Tast_env.get_class env in
    match cls_opt with
    | Some cls ->
      begin match Cls.kind cls, c_tconst_abstract with
      | Ast.Cnormal, TCAbstract _ ->
        Errors.implement_abstract ~is_final:(Cls.final cls) (Cls.pos cls) p "type constant" name
      | _ -> () end;

      begin match Cls.get_typeconst cls name with
      | Some tc ->
        begin match tc.ttc_abstract, tc.ttc_type, tc.ttc_enforceable with
        | TCAbstract (Some ty), _, (pos, true)
        | (TCPartiallyAbstract | TCConcrete), Some ty, (pos, true) ->
          Type_test_hint_check.validate_type env ty
            (Errors.invalid_enforceable_type "constant" (pos, name))
        | _ -> () end;
      | None -> () end;
    | None -> ()
  end
