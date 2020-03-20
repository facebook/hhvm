(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** the naming_db_path says where the naming-table sqlite file is found. *)
val get_naming_db_path : Provider_context.t -> Naming_sqlite.db_path option

(** naming_db_path is set at initialization once we know the path. *)
val set_naming_db_path :
  Provider_context.t -> Naming_sqlite.db_path option -> unit
