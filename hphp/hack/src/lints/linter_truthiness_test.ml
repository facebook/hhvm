(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ast_defs
open Aast
open Typing_defs
module Env = Tast_env
module SN = Naming_special_names
module MakeType = Typing_make_type

let truthiness_test env (ty, p, _e) =
  Tast_utils.(
    let prim_to_string prim =
      Env.print_error_ty env (MakeType.prim_type (get_reason ty) prim)
    in
    List.iter (find_sketchy_types env ty) ~f:(function
        | String ->
          let tystr = prim_to_string Aast_defs.Tstring in
          Lints_errors.sketchy_truthiness_test p tystr `String
        | Arraykey ->
          let tystr = prim_to_string Aast_defs.Tarraykey in
          Lints_errors.sketchy_truthiness_test p tystr `Arraykey
        | Stringish ->
          let tystr = Utils.strip_ns SN.Classes.cStringish in
          Lints_errors.sketchy_truthiness_test p tystr `Stringish
        | XHPChild ->
          let tystr = Utils.strip_ns SN.Classes.cXHPChild in
          Lints_errors.sketchy_truthiness_test p tystr `XHPChild
        | Traversable_interface tystr ->
          Lints_errors.sketchy_truthiness_test p tystr `Traversable);
    match truthiness env ty with
    | Always_truthy ->
      Lints_errors.invalid_truthiness_test p (Env.print_ty env ty)
    | Always_falsy ->
      Lints_errors.invalid_truthiness_test_falsy p (Env.print_ty env ty)
    | Possibly_falsy
    | Unknown ->
      ())

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, _, x) =
      match x with
      | Unop (Unot, e)
      | Eif (e, _, _) ->
        truthiness_test env e
      | Binop { bop = Ampamp | Barbar; lhs = e1; rhs = e2 } ->
        truthiness_test env e1;
        truthiness_test env e2
      | _ -> ()

    method! at_stmt env x =
      match snd x with
      | If (e, _, _)
      | Do (_, e)
      | While (e, _)
      | For (_, Some e, _, _) ->
        truthiness_test env e
      | _ -> ()
  end
