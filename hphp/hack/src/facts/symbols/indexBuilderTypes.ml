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
  hhi_root_folder: Path.t option;
  silent: bool;
  namespace_map: (string * string) list;
}

(* Fully parsed data structure *)
type si_scan_result = {
  sisr_capture: SearchUtils.si_capture;
  sisr_namespaces: (string, int) Caml.Hashtbl.t;
  sisr_filepaths: (string, int64) Caml.Hashtbl.t;
}
