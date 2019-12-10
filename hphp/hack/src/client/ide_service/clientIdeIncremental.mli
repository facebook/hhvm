(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Update the forward and reverse naming tables by parsing the file at the
given path and installing their declarations. If the file could not be read,
it's assumed to be deleted.

Returns an updated [ServerEnv.env], but also
modifies the global naming table state in [Naming_global]. *)
val process_changed_file : ServerEnv.env -> Path.t -> ServerEnv.env Lwt.t
