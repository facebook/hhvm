(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Refactor_sd_types
module T = Tast

exception Refactor_sd_exn = Refactor_sd_exn

let contains_upcast = function
  | Exists_Upcast -> true
  | _ -> false
