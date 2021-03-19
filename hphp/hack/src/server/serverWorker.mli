(*
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

val make :
  longlived_workers:bool ->
  nbr_procs:int ->
  Gc.control ->
  SharedMem.handle ->
  logging_init:(unit -> unit) ->
  MultiWorker.worker list
