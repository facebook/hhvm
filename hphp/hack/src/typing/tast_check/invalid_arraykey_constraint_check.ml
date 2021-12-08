(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Tast_env
module TCO = TypecheckerOptions

let should_enforce env =
  TCO.disallow_invalid_arraykey_constraint (Env.get_tcopt env)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_hint _env (_, hint_) =
      match hint_ with
      | _ -> ()
  end
