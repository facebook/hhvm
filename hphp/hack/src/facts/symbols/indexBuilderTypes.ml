(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type index_builder_context = {
  repo_folder: string;
  sqlite_filename: string option;
  text_filename: string option;
  json_filename: string option;
  json_chunk_size: int;
  json_repo_name: string option;
  custom_service: string option;
  custom_repo_name: string option;
  hhi_root_folder: Path.t option;
  set_paths_for_worker: bool;
  silent: bool;
}

(* Fully parsed data structure *)
type si_scan_result = {
  sisr_capture: SearchUtils.si_capture;
  sisr_namespaces: (string, int) Caml.Hashtbl.t;
  sisr_filepaths: (string, int64) Caml.Hashtbl.t;
}
