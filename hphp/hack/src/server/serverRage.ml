(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

let go (_genv: ServerEnv.genv) (env: ServerEnv.env) : ServerRageTypes.result =
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
       } end
  in
  ide_files_different_from_disk
