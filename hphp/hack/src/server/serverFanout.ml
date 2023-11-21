(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let resolve_files ctx env fanout =
  ServerProgress.with_message "resolving files" @@ fun () ->
  let { Fanout.changed = _; to_recheck; to_recheck_if_errors } = fanout in
  let files_to_recheck = Naming_provider.get_files ctx to_recheck in
  let files_with_errors_to_recheck =
    let files_with_errors = Errors.get_failed_files env.ServerEnv.errorl in
    let files_to_recheck_if_errors =
      Naming_provider.get_files ctx to_recheck_if_errors
    in
    Relative_path.Set.inter files_with_errors files_to_recheck_if_errors
  in
  Relative_path.Set.union files_to_recheck files_with_errors_to_recheck
