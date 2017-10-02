(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

let go (genv: ServerEnv.genv) (env: ServerEnv.env) : ServerRageTypes.result =
  let open ServerRageTypes in

  (* Gather up the contents of all files that hh_server believes are in the *)
  (* IDE different from what's on disk *)
  let ide_files_different_from_disk =
    ServerFileSync.get_unsaved_changes env
    |> Relative_path.Map.map ~f:fst
    |> Relative_path.Map.elements
    |> List.map ~f:begin fun (relPath, data) ->
       {
         title = Some ((Relative_path.to_absolute relPath) ^ ":modified_hh");
         data;
       } end in

  (* Gather up the logfile and the monitor logfile *)
  let root = genv.ServerEnv.options |> ServerArgs.root in
  let log1 = ServerFiles.monitor_log_link root in
  let log2 = ServerFiles.log_link root
  in

  {title = Some log1; data = Sys_utils.cat log1}
  ::
  {title = Some log2; data = Sys_utils.cat log2}
  ::
  ide_files_different_from_disk
