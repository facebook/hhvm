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

val can_infer_return : t -> bool

val can_infer_params : t -> bool

val global_inference : t -> bool

val is_on : t -> bool

val from_string : string -> t

val from_string_opt : string option -> t
