(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let handler =
  object
    inherit Tast_visitor.handler_base

    (* Require that methods named __bool should return a bool. *)
    method! at_method_ _env m =
      let (_, name) = m.m_name in
      if String.equal name "__bool" then
        let (_, hint) = m.m_ret in
        match hint with
        | Some (_, Hprim Tbool) -> ()
        | Some (p, _) -> Lints_diagnostics.bad_virtualized_method p
        | None -> ()
  end
