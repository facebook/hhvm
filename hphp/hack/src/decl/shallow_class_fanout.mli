(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val fanout_of_changes :
  ctx:Provider_context.t -> (string * ClassDiff.t) list -> Fanout.t

val class_names_from_deps :
  ctx:Provider_context.t ->
  get_classes_in_file:(Relative_path.t -> SSet.t) ->
  Typing_deps.DepSet.t ->
  SSet.t
