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
  trace: bool;
  allowed_fixme_codes_strict: ISet.t;
  code_agnostic_fixme: bool;
  paths_to_ignore: Str.regexp list;
  no_load: bool;
  logging_init: unit -> unit;
  cgroup_initial_reading: CgroupProfiler.initial_reading;
}

let save ~logging_init =
  {
    saved_root = Path.make Relative_path.(path_of_prefix Root);
    saved_hhi = Path.make Relative_path.(path_of_prefix Hhi);
    saved_tmp = Path.make Relative_path.(path_of_prefix Tmp);
    trace = !Typing_deps.trace;
    allowed_fixme_codes_strict = !Errors.allowed_fixme_codes_strict;
    code_agnostic_fixme = !Errors.code_agnostic_fixme;
    paths_to_ignore = FilesToIgnore.get_paths_to_ignore ();
    no_load = ServerLoadFlag.get_no_load ();
    logging_init;
    cgroup_initial_reading = CgroupProfiler.get_initial_reading ();
  }

let worker_id_str ~(worker_id : int) =
  if worker_id = 0 then
    "master"
  else
    Printf.sprintf "worker-%d" worker_id

let restore
    {
      saved_root;
      saved_hhi;
      saved_tmp;
      trace;
      allowed_fixme_codes_strict;
      code_agnostic_fixme;
      paths_to_ignore;
      no_load;
      logging_init;
      cgroup_initial_reading;
    }
    ~(worker_id : int) =
  Hh_logger.set_id (worker_id_str ~worker_id);
  Relative_path.(set_path_prefix Root saved_root);
  Relative_path.(set_path_prefix Hhi saved_hhi);
  Relative_path.(set_path_prefix Tmp saved_tmp);
  Typing_deps.trace := trace;
  Typing_deps.worker_id := Some worker_id;
  Errors.allowed_fixme_codes_strict := allowed_fixme_codes_strict;
  Errors.code_agnostic_fixme := code_agnostic_fixme;
  FilesToIgnore.set_paths_to_ignore paths_to_ignore;
  ServerLoadFlag.set_no_load no_load;
  Errors.set_allow_errors_in_default_path false;
  CgroupProfiler.use_initial_reading cgroup_initial_reading;
  logging_init ()

let to_string
    {
      saved_root;
      saved_hhi;
      saved_tmp;
      trace;
      allowed_fixme_codes_strict = _;
      code_agnostic_fixme = _;
      paths_to_ignore = _;
      no_load = _;
      logging_init = _;
      cgroup_initial_reading = _;
    } =
  let saved_root = Path.to_string saved_root in
  let saved_hhi = Path.to_string saved_hhi in
  let saved_tmp = Path.to_string saved_tmp in
  let trace =
    if trace then
      "true"
    else
      "false"
  in
  (* OCaml regexps cannot be re-serialized to strings *)
  let paths_to_ignore = "(...)" in
  [
    ("saved_root", saved_root);
    ("saved_hhi", saved_hhi);
    ("saved_tmp", saved_tmp);
    ("trace", trace);
    ("paths_to_ignore", paths_to_ignore);
  ]
  |> List.map (fun (x, y) -> Printf.sprintf "%s : %s" x y)
  |> String.concat ", "
  |> Printf.sprintf "{%s}"
