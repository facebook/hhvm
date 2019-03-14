(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Tast

module Env = Tast_env
module TCO = TypecheckerOptions
module SN = Naming_special_names

let should_enforce env =
  TCO.disallow_stringish_magic (Env.get_tcopt env)

(** Produce an error on (string) casts of objects. Currently it is allowed in HHVM to
    cast an object if it is Stringish (i.e., has a __toString() method), but all
    (string) casts of objects will be banned in the future. Eventually,
    __toString/(string) casts of objects will be removed from HHVM entirely. *)

let check__toString m is_static =
  let (pos, name) = m.m_name in
  if name = SN.Members.__toString
  then begin
    if m.m_visibility <> Public || is_static
    then Errors.toString_visibility pos;
    match m.m_ret with
    | Some (_, Hprim Tstring) -> ()
    | Some (p, _) -> Errors.toString_returns_string p
    | None -> ()
  end

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env ((p, _), expr) =
    match expr with
    | Cast ((_, Hprim Tstring), te) when should_enforce env ->
      let ((_, ty), _) = te in
      (* Whitelist mixed/nonnull *)
      if not (Env.is_stringish env ty ~allow_mixed:true)
      then Errors.string_cast p (Env.print_ty env ty)
    | _ -> ()

  method! at_static_method _ m = check__toString m true

  method! at_method_ _ m = check__toString m false

  method! at_constructor _ m = check__toString m true
end
