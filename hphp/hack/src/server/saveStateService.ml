(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let save_state ~file_info_on_disk files_info fn =
  let () = Sys_utils.mkdir_p (Filename.dirname fn) in
  let db_name = fn ^ ".sql" in
  let () = if Sys.file_exists fn then
             failwith (Printf.sprintf "Cowardly refusing to overwrite '%s'." fn)
           else () in
  let () = if Sys.file_exists db_name then
             failwith (Printf.sprintf "Cowardly refusing to overwrite '%s'." db_name)
           else () in
  let t = Unix.gettimeofday () in
  let chan = Sys_utils.open_out_no_fail fn in
  let saved = FileInfo.info_to_saved files_info in
  Marshal.to_channel chan saved [];
  Sys_utils.close_out_no_fail fn chan;
  let () = if file_info_on_disk then
    SharedMem.save_file_info_sqlite db_name |> ignore
  else () in
  let sqlite_save_t = SharedMem.save_dep_table_sqlite db_name Build_id.build_revision in
  Hh_logger.log "Saving deptable using sqlite took(seconds): %d" sqlite_save_t;
  ignore @@ Hh_logger.log_duration "Saving" t

let go ~file_info_on_disk files_info filename =
  Core_result.try_with (fun () ->
    save_state ~file_info_on_disk files_info filename)
  |> Core_result.map_error ~f:Printexc.to_string
