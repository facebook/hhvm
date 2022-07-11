(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Refactor_sd_types
module LMap = Local_id.Map
module Cont = Typing_continuations

let var_counter : int ref = ref 0

let fresh_var () : entity_ =
  var_counter := !var_counter + 1;
  Variable !var_counter

module LEnv = struct
  type t = lenv

  let init bindings : t = Cont.Map.add Cont.Next bindings Cont.Map.empty

  let get_local_in_continuation lenv cont lid : entity =
    let open Option.Monad_infix in
    lenv |> Cont.Map.find_opt cont >>= LMap.find_opt lid |> Option.join

  let get_local lenv : LMap.key -> entity =
    get_local_in_continuation lenv Cont.Next

  let set_local_in_continuation lenv cont lid entity : t =
    let update_cont = function
      | None -> None
      | Some lenv_per_cont -> Some (LMap.add lid entity lenv_per_cont)
    in
    Cont.Map.update cont update_cont lenv

  let set_local lenv lid entity : t =
    set_local_in_continuation lenv Cont.Next lid entity
end

let init tast_env constraints bindings =
  { constraints; lenv = LEnv.init bindings; tast_env }

let add_constraint env constraint_ =
  { env with constraints = constraint_ :: env.constraints }

let reset_constraints env = { env with constraints = [] }

let get_local env = LEnv.get_local env.lenv

let set_local env lid entity =
  let lenv = LEnv.set_local env.lenv lid entity in
  { env with lenv }
