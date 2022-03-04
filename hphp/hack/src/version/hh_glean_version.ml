(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This is the version of the Glean "all" schema to be used for
    code indexation *)
let schema_version = 7

(** This is the version of the Glean "hack" schema to be used for
    code indexation *)
let hack_version = "6"

let version : string = string_of_int schema_version

let version_json =
  Hh_json.(JSON_Object [("schema_version", int_ schema_version)])
