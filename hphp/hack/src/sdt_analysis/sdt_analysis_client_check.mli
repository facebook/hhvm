(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val apply_all :
  get_error_count:(unit -> (int * Telemetry.t) Lwt.t) ->
  get_patches:
    (string ->
    ((ServerRenameTypes.patch list * string list * [ `ClassLike | `Function ])
    * Telemetry.t)
    Lwt.t) ->
  apply_patches:(ServerRenameTypes.patch list -> 'a) ->
  path_to_jsonl:string ->
  strategy:[< `CodemodSdtCumulative | `CodemodSdtIndependent ] ->
  log_remotely:bool ->
  tag:string ->
  (Exit_status.t * Telemetry.t) Lwt.t
