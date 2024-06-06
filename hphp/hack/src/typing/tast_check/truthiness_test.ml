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

let warning_kind = Typing_warning.Truthiness_test

let error_code = Typing_warning_utils.code warning_kind

let add_warning env ~as_lint pos kind ty =
  Typing_warning_utils.add_for_migration
    (Env.get_tcopt env)
    ~as_lint:
      (if as_lint then
        Some None
      else
        None)
    ( pos,
      Typing_warning.Truthiness_test,
      { Typing_warning.TruthinessTest.kind; ty } )

let truthiness_test env ~as_lint (ty, p, _e) =
  let module TruthinessTest = Typing_warning.TruthinessTest in
  let prim_to_string prim =
    Env.print_error_ty env (MakeType.prim_type (get_reason ty) prim)
  in
  List.iter (Tast_utils.find_sketchy_types env ty) ~f:(function
      | Tast_utils.String ->
        let tystr = prim_to_string Aast_defs.Tstring in
        add_warning env ~as_lint p TruthinessTest.(Sketchy String) tystr
      | Tast_utils.Arraykey ->
        let tystr = prim_to_string Aast_defs.Tarraykey in
        add_warning env ~as_lint p TruthinessTest.(Sketchy Arraykey) tystr
      | Tast_utils.Stringish ->
        let tystr = Utils.strip_ns SN.Classes.cStringish in
        add_warning env ~as_lint p TruthinessTest.(Sketchy Stringish) tystr
      | Tast_utils.XHPChild ->
        let tystr = Utils.strip_ns SN.Classes.cXHPChild in
        add_warning env ~as_lint p TruthinessTest.(Sketchy Xhp_child) tystr
      | Tast_utils.Traversable_interface tystr ->
        add_warning env ~as_lint p TruthinessTest.(Sketchy Traversable) tystr);
  match Tast_utils.truthiness env ty with
  | Tast_utils.Always_truthy ->
    add_warning
      env
      ~as_lint
      p
      TruthinessTest.(Invalid { truthy = true })
      (Env.print_ty env ty)
  | Tast_utils.Always_falsy ->
    add_warning
      env
      ~as_lint
      p
      TruthinessTest.(Invalid { truthy = false })
      (Env.print_ty env ty)
  | Tast_utils.Possibly_falsy
  | Tast_utils.Unknown ->
    ()

let handler ~as_lint =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, _, x) =
      match x with
      | Unop (Unot, e)
      | Eif (e, _, _) ->
        truthiness_test env ~as_lint e
      | Binop { bop = Ampamp | Barbar; lhs = e1; rhs = e2 } ->
        truthiness_test env ~as_lint e1;
        truthiness_test env ~as_lint e2
      | _ -> ()

    method! at_stmt env x =
      match snd x with
      | If (e, _, _)
      | Do (_, e)
      | While (e, _)
      | For (_, Some e, _, _) ->
        truthiness_test env ~as_lint e
      | _ -> ()
  end
