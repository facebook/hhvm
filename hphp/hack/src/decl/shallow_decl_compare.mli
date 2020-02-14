(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val compute_class_fanout :
  Provider_context.t ->
  get_classes_in_file:(Relative_path.t -> SSet.t) ->
  Relative_path.t list ->
  AffectedDeps.t
