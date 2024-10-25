(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val notebook_to_hack :
  notebook_name:string ->
  header:string ->
  Hh_json.json ->
  (string, string) Result.t
