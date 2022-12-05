(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env : sig
  type t

  val empty : t
end = struct
  type t = unit

  let empty = ()
end

let on_class_ (env, (Aast.{ c_reqs; c_kind; _ } as c), err_acc) =
  let (c_req_extends, c_req_implements, c_req_class) = Aast.split_reqs c_reqs in
  let is_trait = Ast_defs.is_c_trait c_kind
  and is_interface = Ast_defs.is_c_interface c_kind in
  let err_req_impl =
    match c_req_implements with
    | (pos, _) :: _ when not is_trait ->
      Some (Err.naming @@ Naming_error.Invalid_require_implements pos)
    | _ -> None
  and err_req_class =
    match c_req_class with
    | (pos, _) :: _ when not is_trait ->
      Some (Err.naming @@ Naming_error.Invalid_require_class pos)
    | _ -> None
  and err_req_extends =
    match c_req_extends with
    | (pos, _) :: _ when not (is_trait || is_interface) ->
      Some (Err.naming @@ Naming_error.Invalid_require_extends pos)
    | _ -> None
  in
  let err =
    List.fold_right ~init:err_acc ~f:Err.Free_monoid.plus
    @@ List.filter_map ~f:Fn.id [err_req_impl; err_req_class; err_req_extends]
  in
  Naming_phase_pass.Cont.next (env, c, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_class_ = Some on_class_ })

let visitor = Naming_phase_pass.mk_visitor [pass]

let validate f ?init ?(env = Env.empty) elem =
  Err.from_monoid ?init @@ snd @@ f env elem

let validate_program ?init ?env elem =
  validate visitor#on_program ?init ?env elem

let validate_module_def ?init ?env elem =
  validate visitor#on_module_def ?init ?env elem

let validate_class ?init ?env elem = validate visitor#on_class_ ?init ?env elem

let validate_typedef ?init ?env elem =
  validate visitor#on_typedef ?init ?env elem

let validate_fun_def ?init ?env elem =
  validate visitor#on_fun_def ?init ?env elem

let validate_gconst ?init ?env elem = validate visitor#on_gconst ?init ?env elem
