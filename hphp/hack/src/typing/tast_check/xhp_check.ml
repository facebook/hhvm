(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast
open Typing_defs
module Env = Tast_env

let check_xhp_children env pos ty =
  if not @@ Env.is_xhp_child env pos ty then
    let ty_str = Env.print_error_ty ~ignore_dynamic:true env ty in
    let msgl = Reason.to_string ("This is " ^ ty_str) (fst ty) in
    Errors.illegal_xhp_child pos msgl

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, Xml (_, _, tel)) ->
        List.iter tel ~f:(fun ((pos, ty), _) -> check_xhp_children env pos ty)
      | _ -> ()
  end
