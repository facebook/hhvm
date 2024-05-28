(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val build_revision : string

val build_commit_time : int

val build_commit_time_string : string

val build_mode : string

val is_build_optimized : bool

(**
  TODO(T190591227): make `is_dev_build` logic make sense.
  To the best of our understanding, the intent was:
  - for `is_dev_build` to return true iff the binary was not built by fbpkg
  - and the reason to check `is_dev_build` was because when the binary was *not* built by fbpkg
  then that meant that we were probably developing locally and probably did not have a saved state
  corresponding to the hh version

  > And, iuc, `is_dev_build` *never* checked whether the build mode started with '@//mode/dev'

  However, as of D57699186, `is_dev_build` returns true for `opt` builds even when not built by fbpkg,
  since the build info is injected by the linker.
*)
val is_dev_build : bool
