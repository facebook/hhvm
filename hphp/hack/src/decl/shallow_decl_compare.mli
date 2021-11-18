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
  fetch_old_decls:(string list -> Shallow_decl_defs.shallow_class option SMap.t) ->
  Relative_path.t list ->
  AffectedDeps.t
