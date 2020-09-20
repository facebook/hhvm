(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val init :
  root:Path.t ->
  shmem_config:SharedMem.config ->
  popt:ParserOptions.t ->
  tcopt:TypecheckerOptions.t ->
  float ->
  Provider_context.t * MultiWorker.worker list * float

val init_with_defaults :
  float -> Provider_context.t * MultiWorker.worker list * float
