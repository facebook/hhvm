(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

val init :
  root:Path.t ->
  shmem_config:SharedMem.config ->
  popt:ParserOptions.t ->
  tcopt:TypecheckerOptions.t ->
  deps_mode:Typing_deps_mode.t ->
  ?gc_control:Gc.control ->
  float ->
  Provider_context.t * MultiWorker.worker list * float

val init_with_defaults :
  float -> Provider_context.t * MultiWorker.worker list * float
