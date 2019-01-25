(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Tast
open Typing_defs

module Env = Tast_env

let check_xhp_children env pos ty =
  let _, ty = Env.expand_type env ty in
  let _, ty = Env.fold_unresolved env ty in
  let tys = match ty with
    | _, Tunresolved ts -> ts
    | _ -> [ty] in
  if not @@ List.for_all ~f:(Env.is_xhp_child env pos) tys
  then
    let ty_str = Env.print_error_ty env ty in
    let msgl = Reason.to_string ("This is "^ty_str) (fst ty) in
    Errors.illegal_xhp_child pos msgl

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env = function
    | _, Xml (_, _, tel) ->
      List.iter tel ~f:(fun ((pos, ty), _) -> check_xhp_children env pos ty)
    | _ -> ()
end
