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
  defs:FileInfo.names Relative_path.Map.t ->
  get_remote_old_decl:
    (string -> Shallow_decl_defs.shallow_class option SMap.t option) ->
  Relative_path.t list ->
  AffectedDeps.t
