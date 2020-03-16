(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let update_after_recheck
    genv
    env
    ~(changed_files : Relative_path.Set.t)
    ~(rechecked_files : Relative_path.Set.t)
    (errors : Errors.t) : ServerEnv.env * string Future.t option =
  ignore (genv, rechecked_files, errors);
  let env =
    {
      env with
      ServerEnv.changed_files =
        Relative_path.Set.union env.ServerEnv.changed_files changed_files;
    }
  in
  (env, None)

let set_up_replay_environment
    ~(handle : string)
    ~(root : Path.t)
    ~(temp_dir : Path.t)
    ~(fanout_input_path : Path.t)
    ~(expected_errors_path : Path.t) : (unit, string) result =
  ignore (handle, root, temp_dir, fanout_input_path, expected_errors_path);
  Error "Not implemented"
