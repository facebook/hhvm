(*
 * Copyright (c) 2023, Meta, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module Env = Tast_env
module Cls = Folded_class
module SN = Naming_special_names

let check_internal_classname pos ci env =
  let class_result =
    match ci with
    | CI cls -> Tast_env.get_class env (snd cls)
    | CIself
    | CIstatic ->
      Tast_env.get_self_class env
    | CIparent -> Tast_env.get_parent_class env
    | _ -> Decl_entry.DoesNotExist
  in
  match class_result with
  | Decl_entry.Found cls when Cls.internal cls ->
    Lints_diagnostics.internal_classname pos
  | _ -> ()

(* Checks creation of a ::class from an internal class *)
let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, _, expr) =
      match expr with
      | Class_const ((_, p, ci), pstr) when String.equal (snd pstr) "class" ->
        check_internal_classname p ci env
      | _ -> ()
  end
