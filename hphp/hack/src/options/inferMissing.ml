(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | Deactivated
  | Infer_return
  | Infer_params
  | Infer_global
[@@deriving show]

let can_infer_return t = t = Infer_return

let can_infer_params t = t = Infer_params

let global_inference t = t = Infer_global

let is_on t = t <> Deactivated

let from_string str =
  match str with
  | "return" -> Infer_return
  | "params" -> Infer_params
  | "global" -> Infer_global
  | _ -> Deactivated

let from_string_opt opt =
  from_string
  @@
  match opt with
  | None -> ""
  | Some s -> s
