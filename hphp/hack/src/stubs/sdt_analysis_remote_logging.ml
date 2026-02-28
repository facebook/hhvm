(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t = unit

let create :
    strategy:[< `CodemodSdtCumulative | `CodemodSdtIndependent ] ->
    log_remotely:bool ->
    tag:string ->
    t =
 (fun ~strategy:_ ~log_remotely:_ ~tag:_ -> ())

let submit_patch_result :
    t ->
    patched_ids:string list ->
    error_count:int ->
    line_index:int ->
    target_kind:[< `ClassLike | `Function ] ->
    unit =
 (fun _ ~patched_ids:_ ~error_count:_ ~line_index:_ ~target_kind:_ -> ())
