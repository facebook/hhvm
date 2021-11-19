(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let config = Cas.{ install_dir = Path.make "/tmp"; version = "STABLE" }

let go errors ctx =
  let session_id = Random_id.short_string () in
  let dir = Filename.concat Sys_utils.temp_dir_name session_id in
  Sys_utils.mkdir_p dir;
  let state_filename = "dep_edges.bin" |> Filename.concat dir in
  let dep_table_edges_added =
    Typing_deps.save_discovered_edges
      (Provider_context.get_deps_mode ctx)
      ~dest:state_filename
      ~reset_state_after_saving:true
  in
  Hh_logger.log
    "Saved partial dependency graph (%d edges) at %s"
    dep_table_edges_added
    state_filename;
  let dep_edges =
    if dep_table_edges_added > 0 then (
      let result = Cas.upload_directory ~config ~timeout:3600 (Path.make dir) in
      match result with
      | Ok { Cas.digest; _ } -> Cas.to_string digest
      | Error (Cas.Parse_failure error)
      | Error (Cas.Process_failure error) ->
        Hh_logger.log "Unexpected error: %s" error;
        failwith "Uploading dep edges failed"
    ) else
      ""
  in
  let errors = Base64.encode_exn ~pad:false (Marshal.to_string errors []) in
  let dep_edges =
    Base64.encode_exn ~pad:false (Marshal.to_string dep_edges [])
  in
  (errors, dep_edges)
