(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type snapshot = unit

let update_env env changed_files =
  {
    env with
    ServerEnv.changed_files =
      Relative_path.Set.union env.ServerEnv.changed_files changed_files;
  }

let update_before_recheck
    genv
    env
    ~(changed_files : Relative_path.Set.t)
    ~(to_recheck_count : int)
    ~(parse_t : float) : ServerEnv.env * snapshot =
  ignore (genv, to_recheck_count, parse_t);
  (update_env env changed_files, ())

let update_after_recheck
    genv
    env
    snapshot
    ~(cancelled_files : Relative_path.Set.t)
    ~(rechecked_files : Relative_path.Set.t)
    ~(changed_files : Relative_path.Set.t)
    ~(recheck_errors : Errors.t)
    ~(all_errors : Errors.t) : ServerEnv.env * string Future.t option =
  ignore
    ( genv,
      snapshot,
      cancelled_files,
      rechecked_files,
      recheck_errors,
      all_errors );
  (update_env env changed_files, None)

let set_up_replay_environment
    ~(handle : string)
    ~(root : Path.t)
    ~(temp_dir : Path.t)
    ~(fanout_input_path : Path.t)
    ~(expected_errors_path : Path.t) : (unit, string) result =
  ignore (handle, root, temp_dir, fanout_input_path, expected_errors_path);
  Error "Not implemented"
