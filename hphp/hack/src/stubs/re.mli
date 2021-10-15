(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val initialize_lease : ?num_re_workers_opt:int option -> bool -> ReEnv.t

val process_files :
  ReEnv.t ->
  Relative_path.t list ->
  Typing_deps.Mode.t ->
  string option ->
  Errors.t
