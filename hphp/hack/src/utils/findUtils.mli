(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** The file extensions we are interested, each in the form ".ext" *)
val extensions : string list

val is_dot_file : string -> bool

val is_hack : string -> bool

val has_ancestor : string -> string -> bool

(** Filter the relative path. This will reject files not in [extensions] and maybe more. *)
val file_filter : string -> bool

val path_filter : Relative_path.t -> bool

(** It can be hard to be precise about what each filtering function does. This
function there is explained in the role it plays. We must ask watchman to give
files that satisfy FilesToIgnore.watchman_server_expression_terms, and once watchman
has given us its raw updates, then we must put them through this function.
If this function says there are any changes, then hh_server will necessarily take some
action in its recheck loop -- maybe do a recheck, or maybe restart because
.hhconfig has changed or similar. *)
val post_watchman_filter :
  root:Path.t -> raw_updates:SSet.t -> Relative_path.Set.t
