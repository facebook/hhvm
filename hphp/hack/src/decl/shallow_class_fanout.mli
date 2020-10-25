(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val fanout_of_changes :
  mode:Typing_deps_mode.t ->
  get_classes_in_file:(Relative_path.t -> SSet.t) ->
  (string * ClassDiff.t) list ->
  AffectedDeps.t
