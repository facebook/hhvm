(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Saved_state_lookup_common

module Make (Promise : Promise.S) (_ : Utils with type 'a t = 'a Promise.t) =
struct
  let lookup_saved_state
      ~(progress_callback : progress_callback)
      ~(watchman_opts : Watchman_options.t)
      ~(database_shard_name : string option)
      ~(project_name : string)
      ~(project_metadata : string option)
      ~(saved_state_manifold_api_key : string option)
      ~(telemetry : Telemetry.t) :
      (lookup_result, lookup_error * Telemetry.t) result Promise.t =
    ignore
      ( progress_callback,
        watchman_opts,
        database_shard_name,
        project_name,
        project_metadata,
        saved_state_manifold_api_key,
        telemetry );
    failwith "saved-state lookup is not supported in dune builds"
end
