(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module List = Hh_core.List

let save_all_file_info_sqlite db_name files_info =
  begin
    SharedMem.save_file_info_init db_name;
    Relative_path.Map.iter files_info (
      fun path file_info ->
        let funs = file_info.FileInfo.funs in
        let classes = file_info.FileInfo.classes in
        let typedefs = file_info.FileInfo.typedefs in
        let consts = file_info.FileInfo.consts in
        let path = Relative_path.S.to_string path in
        begin
          List.iter funs ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_heap.heap_string_of_key `FunPos name
            ) ~name:name `FuncK path
          );
          List.iter classes ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_heap.heap_string_of_key `TypeId name
            ) ~name:name `ClassK path
          );
          List.iter typedefs ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_heap.heap_string_of_key `TypeId name
            ) ~name:name `ClassK path
          );
          List.iter consts ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_heap.heap_string_of_key `ConstPos name
            ) ~name:name `ConstantK path
          )
        end
    );
    SharedMem.save_file_info_free ()
  end


let dump_filesinfo fn files_info =
  let chan = Sys_utils.open_out_no_fail fn in
  let saved = FileInfo.info_to_saved files_info in
  Marshal.to_channel chan saved [];
  Sys_utils.close_out_no_fail fn chan

let update_save_state ~file_info_on_disk files_info fn =
  let t = Unix.gettimeofday () in
  let db_name = fn ^ ".sql" in
  if not (Disk.file_exists db_name) then
    failwith "Given existing save state SQL file missing";
  dump_filesinfo fn files_info;
  let () = if file_info_on_disk then
    save_all_file_info_sqlite db_name |> ignore
  else () in
  SharedMem.update_dep_table_sqlite db_name Build_id.build_revision;
  ignore @@ Hh_logger.log_duration "Updating saved state took" t

let save_state ~file_info_on_disk files_info fn =
  let () = Sys_utils.mkdir_p (Filename.dirname fn) in
  let db_name = fn ^ ".sql" in
  let () = if Sys.file_exists fn then
             failwith (Printf.sprintf "Cowardly refusing to overwrite '%s'." fn)
           else () in
  let () = if Sys.file_exists db_name then
             failwith (Printf.sprintf "Cowardly refusing to overwrite '%s'." db_name)
           else () in
  match SharedMem.loaded_dep_table_filename () with
  | None ->
    let t = Unix.gettimeofday () in
    dump_filesinfo fn files_info;
    let () = if file_info_on_disk then
      save_all_file_info_sqlite db_name |> ignore
    else () in
    SharedMem.save_dep_table_sqlite db_name Build_id.build_revision;
    ignore @@ Hh_logger.log_duration "Saving saved state took" t
  | Some old_table_fn ->
    (** If server is running from a loaded saved state, it's in-memory
     * tracked depdnencies are incomplete - most of the actual dependencies
     * are in the SQL table. We need to copy that file and update it with
     * the in-memory edges. *)
    let t = Unix.gettimeofday () in
    let content = Sys_utils.cat old_table_fn in
    let () = Sys_utils.mkdir_p (Filename.dirname fn) in
    let () = Sys_utils.write_file ~file:db_name content in
    let _ : float = Hh_logger.log_duration "Made disk copy of loaded saved state. Took" t in
    update_save_state ~file_info_on_disk files_info fn

let get_in_memory_dep_table_entry_count () =
  Core_result.try_with (fun () ->
    SharedMem.get_in_memory_dep_table_entry_count ())
  |> Core_result.map_error ~f:Printexc.to_string

let go ~file_info_on_disk files_info filename =
  Core_result.try_with (fun () ->
    save_state ~file_info_on_disk files_info filename)
  |> Core_result.map_error ~f:Printexc.to_string
