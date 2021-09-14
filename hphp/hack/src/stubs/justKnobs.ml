(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* NB: This error string is matched against in ServerLocalConfig. Don't change
   it without updating sites where we look for it. *)
let eval ?hash:_ ?switch:_ _ = Error "Not implemented: JustKnobs.eval"

let get ?switch:_ _ = Error "Not implemented: JustKnobs.get"
