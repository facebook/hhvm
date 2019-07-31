(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Core_kernel
open Typing_defs
module Env = Tast_env

let check_static_const_prop env c (pos, id) =
  let class_ = Env.get_class env c in
    Option.iter class_ ~f:(fun class_ ->
      let tenv = Env.tast_env_as_typing_env env in
      let scprop = Typing_env.get_static_member false tenv class_ id in
      Option.iter scprop ~f:(fun ({ ce_const; _ }) ->
        if (Env.get_val_kind env = Typing_defs.Lval && ce_const) then
          Errors.assigning_to_const pos)
      )

let check_expr env (_, e) =
  match e with
  | Class_get (((_, cty), _), CGstring pid) ->
    begin
    match snd cty with
    | Tclass ((_, c), _, _) -> check_static_const_prop env c pid
    | Tabstract (AKdependent _, Some (_, Tclass ((_, c), _, _))) ->
      check_static_const_prop env c pid
    | Tabstract (AKgeneric name, None) ->
      let upper_bounds = Env.get_upper_bounds env name in
      let check_class bound =
        match snd bound with
        | Tclass ((_, c), _, _) -> check_static_const_prop env c pid
        | _ -> ()
      in
      Typing_set.iter check_class upper_bounds
    | _ -> ()
    end
  | _ -> ()

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env e = check_expr env e

end
