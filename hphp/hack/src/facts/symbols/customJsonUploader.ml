(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Export all files matching this prefix to a custom symbol service.
 * Parallelize the execution if workers are provided.
 * Print the status of uploading each file if desired *)
let send_to_custom_writer
    ~(workers : MultiWorker.worker list option)
    ~(print_file_status : bool)
    ~(files : string list)
    ~(service : string)
    ~(repo_name : string)
    ~(repo_folder : string) : unit =
  let _ = workers in
  let _ = print_file_status in
  let _ = files in
  let _ = service in
  let _ = repo_name in
  let _ = repo_folder in
  ()
