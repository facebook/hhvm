(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Nast
open Tast

module Env = Tast_env
module TCO = TypecheckerOptions

let should_enforce env =
  TCO.disallow_stringish_magic (Env.get_tcopt env)

(** Produce an error on (string) casts of objects. Currently it is allowed in HHVM to
    cast an object if it is Stringish (i.e., has a __toString() method), but all
    (string) casts of objects will be banned in the future. Eventually,
    __toString/(string) casts of objects will be removed from HHVM entirely. *)
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
end
