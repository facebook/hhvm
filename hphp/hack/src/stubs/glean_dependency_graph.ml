(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_json

let convert_deps_to_json ~deps:_ : json option = None

let go_to_implementation ~class_name:_ ~globalrev:_ : string HashSet.t =
  HashSet.create ()
