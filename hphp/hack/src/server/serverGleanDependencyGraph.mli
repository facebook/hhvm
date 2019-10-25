(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_json
open Typing_deps

(** Convert all of dependencies into a glean-friendly json format.
 * Returns None if an empty set of dependencies was passed in *)
val convert_deps_to_json :
  (Dep.dependency Dep.variant * Dep.dependent Dep.variant) HashSet.t ->
  json option
