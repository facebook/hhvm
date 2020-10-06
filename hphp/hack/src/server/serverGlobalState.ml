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
  deps_mode: Typing_deps.mode;
  allowed_fixme_codes_strict: ISet.t;
  allowed_fixme_codes_partial: ISet.t;
  codes_not_raised_partial: ISet.t;
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
    deps_mode = Typing_deps.get_mode ();
    allowed_fixme_codes_strict = !Errors.allowed_fixme_codes_strict;
    allowed_fixme_codes_partial = !Errors.allowed_fixme_codes_partial;
    codes_not_raised_partial = !Errors.codes_not_raised_partial;
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

let restore
    {
      saved_root;
      saved_hhi;
      saved_tmp;
      saved_gi_tmp;
      trace;
      deps_mode;
      allowed_fixme_codes_strict;
      allowed_fixme_codes_partial;
      codes_not_raised_partial;
      strict_codes;
      paths_to_ignore;
      no_load;
      logging_init;
    }
    ~(worker_id : int) =
  Hh_logger.set_id (worker_id_str ~worker_id);
  Relative_path.(set_path_prefix Root saved_root);
  Relative_path.(set_path_prefix Hhi saved_hhi);
  Relative_path.(set_path_prefix Tmp saved_tmp);
  Typing_global_inference.restore_path saved_gi_tmp;
  Typing_deps.trace := trace;
  Typing_deps.set_mode deps_mode;
  Typing_deps.worker_id := Some worker_id;
  Errors.allowed_fixme_codes_strict := allowed_fixme_codes_strict;
  Errors.allowed_fixme_codes_partial := allowed_fixme_codes_partial;
  Errors.codes_not_raised_partial := codes_not_raised_partial;
  Errors.error_codes_treated_strictly := strict_codes;
  FilesToIgnore.set_paths_to_ignore paths_to_ignore;
  ServerLoadFlag.set_no_load no_load;
  Errors.set_allow_errors_in_default_path false;
  logging_init ()

let to_string
    {
      saved_root;
      saved_hhi;
      saved_tmp;
      saved_gi_tmp;
      trace;
      deps_mode;
      allowed_fixme_codes_strict = _;
      allowed_fixme_codes_partial = _;
      codes_not_raised_partial = _;
      strict_codes;
      paths_to_ignore = _;
      no_load = _;
      logging_init = _;
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
  let deps_mode = Typing_deps.show_mode deps_mode in
  let strict_codes = ISet.to_string strict_codes in
  (* OCaml regexps cannot be re-serialized to strings *)
  let paths_to_ignore = "(...)" in
  [
    ("saved_root", saved_root);
    ("saved_hhi", saved_hhi);
    ("saved_tmp", saved_tmp);
    ("saved_gi_tmp", saved_gi_tmp);
    ("trace", trace);
    ("deps_mode", deps_mode);
    ("strict_codes", strict_codes);
    ("paths_to_ignore", paths_to_ignore);
  ]
  |> List.map (fun (x, y) -> Printf.sprintf "%s : %s" x y)
  |> String.concat ", "
  |> Printf.sprintf "{%s}"
