(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Refactor_sd_types
module LMap = Local_id.Map
module Cont = Typing_continuations

module LEnv = struct
  type t = lenv

  let init bindings : t = Cont.Map.add Cont.Next bindings Cont.Map.empty
end

let init tast_env constraints bindings =
  { constraints; lenv = LEnv.init bindings; tast_env }
