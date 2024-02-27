(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = { stripped_existential: bool } [@@deriving eq, hash, ord, show]

let create ?(stripped_existential = false) () = { stripped_existential }

let empty = { stripped_existential = false }

let to_json { stripped_existential } =
  Hh_json.JSON_Object
    [("stripped_existential", Hh_json.JSON_Bool stripped_existential)]
