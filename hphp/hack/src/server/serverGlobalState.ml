(*
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  saved_root: Path.t;
  saved_hhi: Path.t;
  saved_tmp: Path.t;
  saved_gi_tmp: string;
  trace: bool;
  fixme_codes: ISet.t;
  strict_codes: ISet.t;
  paths_to_ignore: Str.regexp list;
  no_load: bool;
  logging_init: unit -> unit;
}

let save ~logging_init =
  {
    saved_root = Path.make Relative_path.(path_of_prefix Root);
    saved_hhi = Path.make Relative_path.(path_of_prefix Hhi);
    saved_tmp = Path.make Relative_path.(path_of_prefix Tmp);
    saved_gi_tmp = Typing_global_inference.get_path ();
    trace = !Typing_deps.trace;
    fixme_codes = !Errors.ignored_fixme_codes;
    strict_codes = !Errors.error_codes_treated_strictly;
    paths_to_ignore = FilesToIgnore.get_paths_to_ignore ();
    no_load = ServerLoadFlag.get_no_load ();
    logging_init;
  }

let worker_id_str ~(worker_id : int) =
  if worker_id = 0 then
    "master"
  else
    Printf.sprintf "worker-%d" worker_id

let restore state ~(worker_id : int) =
  Hh_logger.set_id (worker_id_str ~worker_id);
  Relative_path.(set_path_prefix Root state.saved_root);
  Relative_path.(set_path_prefix Hhi state.saved_hhi);
  Relative_path.(set_path_prefix Tmp state.saved_tmp);
  Typing_global_inference.restore_path state.saved_gi_tmp;
  Typing_deps.trace := state.trace;
  Errors.ignored_fixme_codes := state.fixme_codes;
  Errors.error_codes_treated_strictly := state.strict_codes;
  FilesToIgnore.set_paths_to_ignore state.paths_to_ignore;
  ServerLoadFlag.set_no_load state.no_load;
  Errors.set_allow_errors_in_default_path false;
  state.logging_init ()

let to_string state =
  let saved_root = Path.to_string state.saved_root in
  let saved_hhi = Path.to_string state.saved_hhi in
  let saved_tmp = Path.to_string state.saved_tmp in
  let trace =
    if state.trace then
      "true"
    else
      "false"
  in
  let fixme_codes = ISet.to_string state.fixme_codes in
  let strict_codes = ISet.to_string state.strict_codes in
  (* OCaml regexps cannot be re-serialized to strings *)
  let paths_to_ignore = "(...)" in
  [
    ("saved_root", saved_root);
    ("saved_hhi", saved_hhi);
    ("saved_tmp", saved_tmp);
    ("saved_gi_tmp", state.saved_gi_tmp);
    ("trace", trace);
    ("fixme_codes", fixme_codes);
    ("strict_codes", strict_codes);
    ("paths_to_ignore", paths_to_ignore);
  ]
  |> List.map (fun (x, y) -> Printf.sprintf "%s : %s" x y)
  |> String.concat ", "
  |> Printf.sprintf "{%s}"
