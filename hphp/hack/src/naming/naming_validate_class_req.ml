(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let on_class_ on_error =
  let handler
        : 'a 'b.
          _ * ('a, 'b) Aast_defs.class_ ->
          (_ * ('a, 'b) Aast_defs.class_, _) result =
   fun (env, (Aast.{ c_reqs; c_kind; _ } as c)) ->
    let (c_req_extends, c_req_implements, c_req_class) =
      Aast.split_reqs c_reqs
    in
    let is_trait = Ast_defs.is_c_trait c_kind
    and is_interface = Ast_defs.is_c_interface c_kind in
    let err_req_impl =
      match c_req_implements with
      | (pos, _) :: _ when not is_trait ->
        Some
          (Naming_phase_error.naming
          @@ Naming_error.Invalid_require_implements pos)
      | _ -> None
    and err_req_class =
      match c_req_class with
      | (pos, _) :: _ when not is_trait ->
        Some
          (Naming_phase_error.naming @@ Naming_error.Invalid_require_class pos)
      | _ -> None
    and err_req_extends =
      match c_req_extends with
      | (pos, _) :: _ when not (is_trait || is_interface) ->
        Some
          (Naming_phase_error.naming @@ Naming_error.Invalid_require_extends pos)
      | _ -> None
    in
    Option.iter ~f:on_error err_req_impl;
    Option.iter ~f:on_error err_req_class;
    Option.iter ~f:on_error err_req_extends;
    Ok (env, c)
  in
  handler

let pass on_error =
  Naming_phase_pass.(
    top_down
      Ast_transform.{ identity with on_class_ = Some (on_class_ on_error) })
