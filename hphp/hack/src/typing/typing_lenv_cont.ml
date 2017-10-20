(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

include Typing_env_types

module CMap = Typing_continuations.Map
module LMap = Local_id.Map

(*****************************************************************************)
(* Functions dealing with continuation based flow typing of local variables *)
(*****************************************************************************)
(* TODO: None case *)
(* Right now, we don't distinguish whether we have a continuation or not,
 * we should *)
let check_error = function
  | None -> LMap.empty
  | Some l -> l

let get_cont_option = CMap.get

let get_cont name m =
  check_error @@ get_cont_option name m

(* Add the key, value pair to the continuation named 'name'
 * If the continuation doesn't exist, create it *)
let add_to_cont name key value m =
  let cont = match CMap.get name m with
    | None -> LMap.empty
    | Some c -> c
  in
  let cont = LMap.add key value cont in
  CMap.add name cont m

let remove_from_cont name key m =
  match CMap.get name m with
  | None -> m
  | Some c -> CMap.add name (LMap.remove key c) m

let drop_cont = CMap.remove

let replace_cont key value map = CMap.add key value map
