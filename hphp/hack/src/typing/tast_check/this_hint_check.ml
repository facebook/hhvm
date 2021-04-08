(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_hint env (pos, hint) =
      let report_error_if_outside_class env =
        match Tast_env.get_self_id env with
        | Some _ -> ()
        | None -> Errors.this_hint_outside_class pos
      in
      match hint with
      | Aast.Hthis -> report_error_if_outside_class env
      | _ -> ()
  end
