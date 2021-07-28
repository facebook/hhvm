(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type version_components = {
  major: int;
  minor: int;
  build: int;
}

type version =
  | Opaque_version of string option
  | Version_components of version_components

val version_to_string_opt : ?pad:bool -> version -> string option

val parse_version : string option -> version

val compare_versions : version -> version -> int
