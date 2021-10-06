(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types
module LMap = Local_id.Map

let init = { constraints = []; lenv = LMap.empty }

let add_constraint env constraint_ =
  { env with constraints = constraint_ :: env.constraints }

let get_local lid env = LMap.find_opt lid env.lenv |> Option.join

let set_local lid entity env = { env with lenv = LMap.add lid entity env.lenv }
