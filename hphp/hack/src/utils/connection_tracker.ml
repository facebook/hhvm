(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = string

let create () : t = Random_id.short_string ()

let log_id (t : t) : string = "mc#" ^ t
