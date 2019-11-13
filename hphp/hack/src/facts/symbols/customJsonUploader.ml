(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Export all files matching this prefix to a custom symbol service.
 * Parallelize the execution if workers are provided. *)
let send_to_custom_writer
    ?(_workers : MultiWorker.worker list option = None)
    (_files : string list)
    (_service : string)
    (_repo_name : string)
    (_repo_folder : string) : unit =
  ()
