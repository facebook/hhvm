(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type batch_state = {
  saved_root: Path.t;
  saved_hhi: Path.t;
  saved_tmp: Path.t;
  trace: bool;
  paths_to_ignore: Str.regexp list;
  allowed_fixme_codes_strict: ISet.t;
  code_agnostic_fixme: bool;
}

let worker_id_str ~(worker_id : int) =
  if worker_id = 0 then
    "batch master"
  else
    Printf.sprintf "batch worker-%d" worker_id

let restore
    ({
       saved_root;
       saved_hhi;
       saved_tmp;
       trace;
       paths_to_ignore;
       allowed_fixme_codes_strict;
       code_agnostic_fixme;
     } :
      batch_state)
    ~(worker_id : int) : unit =
  Hh_logger.set_id (worker_id_str ~worker_id);
  Relative_path.(set_path_prefix Root saved_root);
  Relative_path.(set_path_prefix Hhi saved_hhi);
  Relative_path.(set_path_prefix Tmp saved_tmp);
  Typing_deps.trace := trace;
  FilesToIgnore.set_paths_to_ignore paths_to_ignore;
  Errors.allowed_fixme_codes_strict := allowed_fixme_codes_strict;
  Errors.code_agnostic_fixme := code_agnostic_fixme;
  Errors.set_allow_errors_in_default_path false

let save ~(trace : bool) : batch_state =
  {
    saved_root = Path.make Relative_path.(path_of_prefix Root);
    saved_hhi = Path.make Relative_path.(path_of_prefix Hhi);
    saved_tmp = Path.make Relative_path.(path_of_prefix Tmp);
    trace;
    paths_to_ignore = FilesToIgnore.get_paths_to_ignore ();
    allowed_fixme_codes_strict = !Errors.allowed_fixme_codes_strict;
    code_agnostic_fixme = !Errors.code_agnostic_fixme;
  }
