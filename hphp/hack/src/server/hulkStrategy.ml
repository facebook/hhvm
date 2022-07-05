(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type hulk_mode =
  | Legacy
  | Lite
  | Heavy

(*
  Parse input config into hulkStrategy
  There are three config options that can affect hulk strategy:
    --config hulk_mode=legacy/lite/heavy
    --config hulk_lite=true/false
    --config hulk_heavy=true/false
  hulk_mode will overwrite hulk_{lite,heavy} and we use the table below to determin hulk strategy:
  ---------------------------------------------------
  hulk_mode   hulk_lite   hulk_heavy    hulkStrategy
  "legacy"        _           _         Legacy
  "lite"          _           _         Lite
  "heavy"         _           _         Heavy
  others        true        false       Lite
  others        false       true        Heavy
  others        others      others      Legacy
*)
let config_to_strategy (mode : string) (hulk_lite : bool) (hulk_heavy : bool) :
    hulk_mode =
  match mode with
  | "legacy" -> Legacy
  | "lite" -> Lite
  | "heavy" -> Heavy
  | _ ->
    (match (hulk_lite, hulk_heavy) with
    | (true, false) -> Lite
    | (false, true) -> Heavy
    | (_, _) -> Legacy)

let is_hulk_lite mode : bool =
  match mode with
  | Lite -> true
  | _ -> false

let is_hulk_heavy mode : bool =
  match mode with
  | Heavy -> true
  | _ -> false

let is_hulk_legacy mode : bool =
  match mode with
  | Legacy -> true
  | _ -> false

let is_hulk_v2 mode : bool =
  match mode with
  | Lite
  | Heavy ->
    true
  | _ -> false

let to_str mode : string =
  match mode with
  | Legacy -> "legacy"
  | Lite -> "lite"
  | Heavy -> "heavy"
