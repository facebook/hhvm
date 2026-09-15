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
  shmem_config:Shared_mem.config ->
  popt:Parser_options.t ->
  tcopt:Typechecker_options.t ->
  deps_mode:Typing_deps_mode.t ->
  ?gc_control:Gc.control ->
  float ->
  Provider_context.t * Multi_worker.worker list * float

val init_with_defaults :
  float -> Provider_context.t * Multi_worker.worker list * float
